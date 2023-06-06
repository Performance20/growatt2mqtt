// Growatt Solar Inverter to MQTT
// Repo: https://github.com/nygma2004/growatt2mqtt
// author: Csongor Varga, csongor.varga@gmail.com, some adaption by Helge
// 1 Phase, 2 string inverter version such as MIN 3000 TL-XE, MIC 1500 TL-X
//
// Libraries:
// - ModbusMaster by Doc Walker
// - ArduinoOTA
// - SoftwareSerial
// - AHT10
// Hardware:
// - Wemos D1 mini
// - RS485 to TTL converter: https://www.aliexpress.com/item/1005001621798947.html
// - To power from mains: Hi-Link 5V power supply (https://www.aliexpress.com/item/1005001484531375.html), fuseholder and 1A fuse, and varistor

#include <Arduino.h>
#include <ESP8266WiFi.h>      // Wifi connection
#include <ESP8266WebServer.h> // Web server for general HTTP response
#include <PubSubClient.h>     // MQTT support
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <EEPROM.h>
#include <AutoConnect.h>

#include "globals.h"
#include "settings.h"
#include "growattInterface.h"
#include "webpage.h"

#ifdef AHTXX_SENSOR
#include <AHT10.h>
#include <Wire.h>
#endif

#define MAX_JSON_TOPIC_LENGTH 1024
#define MAX_ROOT_TOPIC_LENGTH 80
#define MAX_EXPECTED_TOPIC_LENGTH 50

bool updateRegister;
bool updateStatus;
bool checkWifi;
#ifdef AHTXX_SENSOR
bool ath15_connected;
#endif 

#define CLIENT_ID_SIZE (sizeof(clientID) + 7) // 3*2 char = 6 + '-'
#define TOPPIC_ROOT_SIZE (sizeof(topicRootStart) + 7) // 3*2 char = 6 + '-'
char fullClientID[CLIENT_ID_SIZE];
char topicRoot[TOPPIC_ROOT_SIZE]; // MQTT root topic for the device, + client ID

os_timer_t myTimer;


ESP8266WebServer server(80);
AutoConnect      Portal(server);
AutoConnectConfig autocconfig;

WiFiClient espClient;
PubSubClient mqtt(mqtt_server, mqtt_server_port, espClient);

#ifdef AHTXX_SENSOR
AHT10 sensorAHT15(AHT10_ADDRESS_0X38);
#endif

void callback(char *topic, byte *payload, unsigned int length);
growattIF growattInterface(MAX485_RE_NEG, MAX485_DE, MAX485_RX, MAX485_TX);

void ReadInputRegisters()
{
  char json[MAX_JSON_TOPIC_LENGTH];
  char topic[MAX_ROOT_TOPIC_LENGTH];
  uint8_t result;

  digitalWrite(STATUS_LED, 0);
  result = growattInterface.ReadInputRegisters();
  if (result == growattInterface.Success)
  {
    growattInterface.InputRegistersToJson(json);
#ifdef DEBUG_MQTT
    Serial.println(json);
#endif
    snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/data", topicRoot);
    mqtt.publish(topic, json);
#ifdef DEBUG_MQTT
    Serial.println("Data MQTT sent");
#endif    
  }
  else 
  {
    Serial.print(F("Error: "));
    String message = growattInterface.sendModbusError(result);
    Serial.println(message);
    snprintf(topic, MAX_ROOT_TOPIC_LENGTH , "%s/error", topicRoot);
    mqtt.publish(topic, message.c_str());
    delay(5);
  }
  digitalWrite(STATUS_LED, 1);
}

void ReadHoldingRegisters()
{
  char json[MAX_JSON_TOPIC_LENGTH];
  char topic[MAX_ROOT_TOPIC_LENGTH];

  uint8_t result;

  digitalWrite(STATUS_LED, 0);
  result = growattInterface.ReadHoldingRegisters();
  if (result == growattInterface.Success)
  {
    growattInterface.HoldingRegistersToJson(json);
#ifdef DEBUG_MQTT
    Serial.println(json);
#endif
    snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/settings", topicRoot);
    mqtt.publish(topic, json);
#ifdef DEBUG_MQTT
    Serial.println("Setting MQTT sent");
#endif    
    // Set the flag to true not to read the holding registers again
    holdingregisters = false;
  }
  else
  {
    Serial.print(F("Error: "));
    String message;
    message = growattInterface.sendModbusError(result);
    Serial.println(message);
    snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
    mqtt.publish(topic, message.c_str());
    delay(5);
  }
  digitalWrite(STATUS_LED, 1);
}

// This is the 1 second timer callback function
void timerCallback(void *pArg)
{
  seconds++;
  uptime++;

  if (seconds % config.modbus_update_sec == 0)
    updateRegister = true;

  if (seconds % config.status_update_sec == 0)
    updateStatus = true;

  if (seconds % config.wificheck_sec == 0)
    checkWifi = true;
}

void saveConfig()
{
  EEPROM.begin(sizeof(config));
  EEPROM.put(EE_START_ADDR, config);
  EEPROM.commit(); // Only needed for ESP8266 to get data written
  EEPROM.end();
}

void loadEEpromData()
{
  EEPROM.begin(sizeof(config));
  EEPROM.get(EE_START_ADDR, config);
  EEPROM.end();

  if (memcmp(config.EEpromInit, DefEEpromInit, EE_INIT_STATE_SIZE) != 0) // Init Code found?
  { // No
    memcpy(config.EEpromInit, DefEEpromInit, EE_INIT_STATE_SIZE); // initialize eeprom with default values 
    config.modbus_update_sec = UPDATE_MODBUS;
    config.status_update_sec = UPDATE_STATUS;
    config.wificheck_sec = WIFICHECK;
    saveConfig();
    ESP.eraseConfig();
#ifdef DEBUG_SERIAL
    delay(3000);
    Serial.println(F("Reset eesprom values to default and clean Wifi settings"));
#endif
  }
}

// MQTT reconnect logic
void reconnect()
{
  // String mytopic;
  //  Loop until we're reconnected
  char topic[MAX_ROOT_TOPIC_LENGTH];

  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");
    Serial.print(F("Client ID: "));
    Serial.println(fullClientID);
    // Attempt to connect
    snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/%s", topicRoot, "connection");
    if (mqtt.connect(fullClientID, mqtt_user, mqtt_password, topic, 1, true, "offline"))
    { // last will
      Serial.println(F("connected"));
      // ... and resubscribe
      mqtt.publish(topic, "online", true);
      snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/write/#", topicRoot);
      mqtt.subscribe(topic);
      snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/writeconfig/#", topicRoot);
      mqtt.subscribe(topic);
    }
    else
    {
      Serial.print(F("failed, rc="));
      Serial.print(mqtt.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// MQTT receive messages
void callback(char *topic, byte *payload, unsigned int length)
{
  // Convert the incoming byte array to a string

  unsigned int i = 0;
  uint8_t result;
  uint16_t resparam;
  char json[MAX_JSON_TOPIC_LENGTH];
  char rootTopic[MAX_ROOT_TOPIC_LENGTH];
  char expectedTopic[MAX_EXPECTED_TOPIC_LENGTH];
  char payloadString[30];
  
  for (i = 0; i < length; i++)
  { // each char to upper
    payloadString[i] = toupper(payload[i]);
  }
  payloadString[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char *)payloadString;

#ifdef DEBUG_SERIAL
  Serial.print(F("Message arrived on topic: ["));
  Serial.print(topic);
  Serial.print(F("], "));
  Serial.println(message);
#endif

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH, "%s/write/getSettings", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    if (message == "ON")
    {
      holdingregisters = true;
    }
  }

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH , "%s/write/setEnable", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    if (message == "ON")
    {
      result = growattInterface.writeRegister(growattInterface.regOnOff, 1);
      if (result == growattInterface.Success)
      {
        holdingregisters = true;
      }
      else
      {
        snprintf(json, MAX_JSON_TOPIC_LENGTH, "last trasmition has faild with: %s", growattInterface.sendModbusError(result).c_str());
        snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
        mqtt.publish(rootTopic, json);
      }
    }
    else if (message == "OFF")
    {
        result = growattInterface.writeRegister(growattInterface.regOnOff, 0);
        if (result == growattInterface.Success)
        {
          holdingregisters = true;
        }
        {
          snprintf(json, MAX_JSON_TOPIC_LENGTH, "last trasmition has faild with: %s", growattInterface.sendModbusError(result).c_str());
          snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
          mqtt.publish(rootTopic, json);
        }
     } 
  }

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH , "%s/write/setMaxOutput", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    result = growattInterface.writeRegister(growattInterface.regMaxOutputActive, message.toInt());
    if (result == growattInterface.Success)
    {
      holdingregisters = true;
    }
    else
    {
      snprintf(json, MAX_JSON_TOPIC_LENGTH, "last trasmition has faild with: %s", growattInterface.sendModbusError(result).c_str());
      snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
      mqtt.publish(rootTopic, json);
    }
  }

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH , "%s/write/setStartVoltage", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    result = growattInterface.writeRegister(growattInterface.regStartVoltage, (message.toInt() * 10)); //*10 transmit with one digit after decimal place
    if (result == growattInterface.Success)
    {
      holdingregisters = true;
    }
    else
    {
      snprintf(json, MAX_JSON_TOPIC_LENGTH, "last trasmition has faild with: %s", growattInterface.sendModbusError(result).c_str());
      snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
      mqtt.publish(rootTopic, json);
    }
  }

#ifdef useModulPower
  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH , "%s/write/setModulPower", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    growattInterface.writeRegister(growattInterface.regOnOff, 0);
    delay(500);

    result = growattInterface.writeRegister(growattInterface.regModulPower, int(strtol(message.c_str(), NULL, 16)));
    delay(500);

    growattInterface.writeRegister(growattInterface.regOnOff, 1);
    delay(1500);
    
    if (result == growattInterface.Success)
    {
      holdingregisters = true;
    }
    else
    {
      snprintf(json, MAX_JSON_TOPIC_LENGTH, "last trasmition has faild with: %s", growattInterface.sendModbusError(result).c_str());
      snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/error", topicRoot);
      mqtt.publish(rootTopic, json);
    }
  }
#endif

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH, "%s/writeconfig/setModbusUpd", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    resparam = message.toInt();
    if (resparam != config.modbus_update_sec)
    {
      if (resparam > 0)
      {
       config.modbus_update_sec = resparam;
       saveConfig();
      } 
    }
    snprintf(json, MAX_JSON_TOPIC_LENGTH, "Reading Modbus values updated to %d sec", config.modbus_update_sec);
    snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/info", topicRoot);
    mqtt.publish(rootTopic, json);
#ifdef DEBUG_SERIAL
    Serial.println(json);
#endif    
  }

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH, "%s/writeconfig/setStatusUpd", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    resparam = message.toInt();
    if (resparam != config.status_update_sec)
    {
      if (resparam > 0)
      {
       config.status_update_sec = resparam;
       saveConfig();
      }
    }
    snprintf(json, MAX_JSON_TOPIC_LENGTH, "Send Status updated to %d sec", config.status_update_sec);
    snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/info", topicRoot);
    mqtt.publish(rootTopic, json);
#ifdef DEBUG_SERIAL
    Serial.println(json);
#endif    
  }

  snprintf(expectedTopic, MAX_EXPECTED_TOPIC_LENGTH, "%s/writeconfig/setWifiCheck", topicRoot);
  if (strcmp(expectedTopic, topic) == 0)
  {
    resparam = message.toInt();
    if (resparam != config.wificheck_sec)
    {
      if (resparam > 0)
      {
       config.wificheck_sec = resparam;
       saveConfig();
      }
    }
    snprintf(json, MAX_JSON_TOPIC_LENGTH, "Check Wifi Status updated to %d sec", config.wificheck_sec);
    snprintf(rootTopic, MAX_ROOT_TOPIC_LENGTH, "%s/info", topicRoot);
    mqtt.publish(rootTopic, json);
#ifdef DEBUG_SERIAL
    Serial.println(json);
#endif    
  }
}
/*
void rootPage() {
  char content[] = "Growatt Solar Inverter to MQTT Gateway";
  server.send(200, "text/plain", content);
}
*/
void setup()
{
  Serial.begin(SERIAL_RATE);
  Serial.println(F("\nGrowatt Solar Inverter to MQTT Gateway"));
  // Init outputs, RS485 in receive mode
  pinMode(STATUS_LED, OUTPUT);

  // Initialize some variables
  uptime = 0;
  updateRegister = true;
  updateStatus = true;
  checkWifi = true;

  loadEEpromData();

#ifdef DEBUG_SERIAL
  Serial.println(F("Load Update values"));
  Serial.printf("Values via Modbus: %d sec\n", config.modbus_update_sec);
  Serial.printf("Status Update: %d sec\n", config.status_update_sec);
  Serial.printf("Wifi Check: %d sec\n", config.wificheck_sec);
#endif

  // Connect to Wifi
#ifdef FIXEDIP
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
#endif
  autocconfig.ota = AC_OTA_BUILTIN;
  Portal.config(autocconfig);
  Portal.load(PAGE_HELLO);

  //  AutoConnect AP - Configure SSID and password for Captive Portal
  
  //server.on("/", rootPage);
  if (Portal.begin()) 
  {
    Serial.println("");
    Serial.println("Connected to WiFi");
    Serial.println("IPAddress: " + WiFi.localIP().toString());
    Serial.print("Signal [RSSI]: ");
    Serial.println(WiFi.RSSI());
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
    ESP.restart();
  }

  // Set up the fully client ID
  byte mac[6]; // the MAC address of your Wifi shield
  WiFi.macAddress(mac);
  snprintf(fullClientID, CLIENT_ID_SIZE, "%s-%02x%02x%02x", clientID, mac[3], mac[4], mac[5]);
  snprintf(topicRoot, TOPPIC_ROOT_SIZE, "%s-%02x%02x%02x", clientID, mac[3], mac[4], mac[5]);

  Serial.print(F("Client ID: "));
  Serial.println(fullClientID);

  // Set up the Modbus line
  growattInterface.initGrowatt();
  Serial.println("Modbus connection is set up");

  #ifdef AHTXX_SENSOR
    // AHT15 connection check
    ath15_connected = sensorAHT15.begin(SDA_PIN, SCL_PIN);
    if (ath15_connected != true)
    {
      Serial.println(F("AHT15 sensor not connected or fail to load calibration coefficient"));
    }
  #endif

    // Create the 1 second timer interrupt
    os_timer_setfn(&myTimer, timerCallback, NULL);
    os_timer_arm(&myTimer, 1000, true);

    // Set up the MQTT server connection
    if (strlen(mqtt_server) > 0)
    {
      mqtt.setServer(mqtt_server, mqtt_server_port);
      mqtt.setBufferSize(1024);
      mqtt.setCallback(callback);
    }
/*
    // OTA Firmware Update
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(fullClientID);

    // No authentication by default
    // ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]()
                       {
      os_timer_disarm(&myTimer);
      Serial.println("Start"); });

    ArduinoOTA.onEnd([]()
                     {
      Serial.println("\nEnd");
      os_timer_arm(&myTimer, 1000, true); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

    ArduinoOTA.begin();
    */
}

void loop()
{
  char value[MAX_JSON_TOPIC_LENGTH];
  char topic[MAX_ROOT_TOPIC_LENGTH];
#ifdef AHTXX_SENSOR
  float valueTemp;
  float valueHum;
#endif

  //ArduinoOTA.handle();

  // Handle HTTP server requests
  //server.handleClient();

  // Handle MQTT connection/reconnection
  if (strlen(mqtt_server) > 0)
  {
    if (!mqtt.connected())
    {
      reconnect();
    }
    mqtt.loop();
  }

  // Query the modbus device
  if (updateRegister == true)
  {
    ReadInputRegisters();
    if (holdingregisters == true)
    {
      // Read the holding registers
      ReadHoldingRegisters();  //Settings
    }
    updateRegister = false;
  }

  // Send RSSI and uptime status
  if (updateStatus == true)
  {
    // Send MQTT update
    if (strlen(mqtt_server) > 0)
    {
#ifdef AHTXX_SENSOR                                                   // recomended polling frequency 8sec..30sec
      if (ath15_connected != true)
      {
        ath15_connected = sensorAHT15.begin(SDA_PIN, SCL_PIN);
      }
      if (ath15_connected == true)
      {
        valueTemp = sensorAHT15.readTemperature(AHT10_FORCE_READ_DATA); // read 6-bytes over I2C
        valueHum = sensorAHT15.readHumidity(AHT10_USE_READ_DATA);
      }
      else
      {
        valueTemp = 0;
        valueHum = 0;
      }
#ifdef DEBUG_SERIAL
      Serial.printf("Temperature: %.2f °C      Humidity: %.2f %%\n", valueTemp, valueHum);
#endif
      snprintf(value, MAX_JSON_TOPIC_LENGTH, "{\"rssi\":%d,\"uptime\":%lu,\"ssid\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"clientid\":\"%s\",\"version\":\"%s\",\"modbusUpdate\":%d,\"statusUpdate\":%d,\"Wifi check\":%d,\"temperature\":%.2f,\"humidity\":%.2f}", WiFi.RSSI(), uptime, WiFi.SSID().c_str(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], fullClientID, buildversion, config.modbus_update_sec, config.status_update_sec, config.wificheck_sec, valueTemp, valueHum);
#else
      snprintf(value, MAX_JSON_TOPIC_LENGTH, "{\"rssi\":%d,\"uptime\":%lu,\"ssid\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"clientid\":\"%s\",\"version\":\"%s\",\"modbusUpdate\":%d,\"statusUpdate\":%d,\"Wifi check\":%d}", WiFi.RSSI(), uptime, WiFi.SSID().c_str(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], fullClientID, buildversion, config.modbus_update_sec, config.status_update_sec, config.wificheck_sec);
#endif
      snprintf(topic, MAX_ROOT_TOPIC_LENGTH, "%s/%s", topicRoot, "status");
      mqtt.publish(topic, value);
#ifdef DEBUG_MQTT
      Serial.println(value);
      Serial.println(F("MQTT status sent"));
#endif      
    }
    updateStatus = false;
  }
/*
  if (checkWifi == true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Reconnecting to wifi...");
      WiFi.reconnect();
      uptime = 0;
    }
    checkWifi = false;
  }
*/
  
}
