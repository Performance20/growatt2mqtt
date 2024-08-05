#include "growattInterface.h"


growattIF::growattIF(int _PinMAX485_RE_NEG, int _PinMAX485_DE, int _PinMAX485_RX, int _PinMAX485_TX) {
  PinMAX485_RE_NEG = _PinMAX485_RE_NEG;
  PinMAX485_DE = _PinMAX485_DE;
  PinMAX485_RX = _PinMAX485_RX;
  PinMAX485_TX = _PinMAX485_TX;

  // Init outputs, RS485 in receive mode
  pinMode(PinMAX485_RE_NEG, OUTPUT);
  pinMode(PinMAX485_DE, OUTPUT);
  digitalWrite(PinMAX485_RE_NEG, 0);
  digitalWrite(PinMAX485_DE, 0);
}

void growattIF::initGrowatt() {
  serial = new SoftwareSerial (PinMAX485_RX, PinMAX485_TX, false); //RX, TX
  serial->begin(MODBUS_RATE);
  growattInterface.begin(SLAVE_ID , *serial);

  static growattIF* obj = this;                               //pointer to the object
  // Callbacks allow us to configure the RS485 transceiver correctly
  growattInterface.preTransmission ([]() {                   //Set function pointer via anonymous Lambda function
    obj->preTransmission();
  });

  growattInterface.postTransmission([]() {                   //Set function pointer via anonymous Lambda function
    obj->postTransmission();
  });
}

uint8_t growattIF::writeRegister(uint16_t reg, uint16_t message) {
  return growattInterface.writeSingleRegister(reg, message);
}

uint16_t growattIF::readRegister(uint16_t reg) {
  growattInterface.readHoldingRegisters(reg, 1);
  return growattInterface.getResponseBuffer(0);				// returns 16bit
}

void growattIF::preTransmission() {
  digitalWrite(PinMAX485_RE_NEG, 1);
  digitalWrite(PinMAX485_DE, 1);
}

void growattIF::postTransmission() {
  digitalWrite(PinMAX485_RE_NEG, 0);
  digitalWrite(PinMAX485_DE, 0);
}

const char * growattIF::getWebStatusPage(unsigned long uptime)
{

#define WEB_LINE_SIZE  100
char webLineBuffer[WEB_LINE_SIZE];

  strcpy(webResponse,  "<html><head><title>Growatt</title><meta http-equiv=\"refresh\" content=\"10\"></head><body><h2>Growatt Solar Inverter to MQTT Gateway </h2>");

  snprintf(webLineBuffer, WEB_LINE_SIZE, "Last Modbus Transmission Status: %i - ",lastModbusTransmissionStatus ); strcat(webResponse, webLineBuffer);
  strcat(webResponse, getModbusErrorString(lastModbusTransmissionStatus));


  snprintf(webLineBuffer, WEB_LINE_SIZE, "<table><tr><td><table><tr><td>Interface Uptime: </td><td> %lu days %lu:%02lu:%02lu h</td></tr>", uptime/(3600*24),uptime%(3600*24)/3600,(uptime/60)%60,uptime%60); strcat(webResponse, webLineBuffer);
  unsigned long t = (unsigned long)modbusdata.totalworktime;
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>Total Inverter Work Time: </td><td> %lu days %lu:%02lu:%02lu h </td></tr>", t/(3600*24),t%(3600*24)/3600,(t/60)%60,t%60 ); strcat(webResponse, webLineBuffer);
  strcat(webResponse, "<tr><td>-------------</td><td>");
  //Modbus Status

  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_STATUS         "</td><td> %s </td></tr>"      , getInverterStatusString(modbusdata.status) ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_OUTPUTPOWER    "</td><td><b> %3.2f W</b></td></tr>"  , modbusdata.outputpower    ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_ENERGYTODAY    "</td><td> %3.3f kWh</td></tr>", modbusdata.energytoday    ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_ENERGYTOTAL    "</td><td> %3.2f kWh</td></tr>", modbusdata.energytotal    ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_SOLARPOWER     "</td><td> %3.2f W</td></tr>"  , modbusdata.solarpower     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV1ENERGYTODAY "</td><td> %3.2f kWh</td></tr>", modbusdata.pv1energytoday ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV1ENERGYTOTAL "</td><td> %3.2f kWh</td></tr>", modbusdata.pv1energytotal ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV1POWER       "</td><td> %3.2f W</td></tr>"  , modbusdata.pv1power       ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV1VOLTAGE     "</td><td> %3.2f V</td></tr>"  , modbusdata.pv1voltage     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV1CURRENT     "</td><td> %3.2f A</td></tr>"  , modbusdata.pv1current     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV2ENERGYTODAY "</td><td> %3.2f kWh</td></tr>", modbusdata.pv2energytoday ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV2ENERGYTOTAL "</td><td> %3.2f kWh</td></tr>", modbusdata.pv2energytotal ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV2POWER       "</td><td> %3.2f W</td></tr>"  , modbusdata.pv2power       ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV2VOLTAGE     "</td><td> %3.2f V</td></tr>"  , modbusdata.pv2voltage     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_PV2CURRENT     "</td><td> %3.2f A</td></tr>"  , modbusdata.pv2current     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDFREQUENCY  "</td><td> %3.2f Hz</td></tr>" , modbusdata.gridfrequency  ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDVOLTAGE    "</td><td> %3.2f V</td></tr>"  , modbusdata.gridvoltage    ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_OPFULLPOWER    "</td><td> %3.2f </td></tr>"   , modbusdata.opfullpower    ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_TEMPINVERTER   "</td><td> %3.2f C</td></tr>"  , modbusdata.tempinverter   ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_TEMPIPM        "</td><td> %3.2f C</td></tr>"  , modbusdata.tempipm        ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_TEMPBOOST      "</td><td> %3.2f C</td></tr>"  , modbusdata.tempboost      ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_TOTALWORKTIME  "</td><td> %3.2f s</td></tr>"  , modbusdata.totalworktime  ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_IPF            "</td><td> %i    </td></tr>"   , modbusdata.ipf            ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_REALOPPERCENT  "</td><td> %i    </td></tr>"   , modbusdata.realoppercent  ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_DERATINGMODE   "</td><td> %i    </td></tr>"   , modbusdata.deratingmode   ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_FAULTCODE      "</td><td> %i    </td></tr>"   , modbusdata.faultcode      ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_FAULTBITCODE   "</td><td> %i    </td></tr>"   , modbusdata.faultbitcode   ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_WARNINGBITCODE "</td><td> %i    </td></tr>"   , modbusdata.warningbitcode ); strcat(webResponse, webLineBuffer);

  snprintf(webLineBuffer, WEB_LINE_SIZE, "</table></td><td><table>"); strcat(webResponse, webLineBuffer);

  // Modbus Settings
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_ENABLE                "</td><td> %i    </td></tr>"   , modbussettings.enable                ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_SAFETYFUNCEN          "</td><td> %i    </td></tr>"   , modbussettings.safetyfuncen          ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_MAXOUTPUTACTIVEPP     "</td><td> %i    </td></tr>"   , modbussettings.maxoutputactivepp     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_MAXOUTPUTREACTIVEPP   "</td><td> %i    </td></tr>"   , modbussettings.maxoutputreactivepp   ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_MODUL                 "</td><td> %i    </td></tr>"   , modbussettings.modul                 ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_MAXPOWER              "</td><td> %3.2f </td></tr>"   , modbussettings.maxpower              ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_VOLTNORMAL            "</td><td> %3.2f </td></tr>"   , modbussettings.voltnormal            ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_STARTVOLTAGE          "</td><td> %3.2f </td></tr>"   , modbussettings.startvoltage          ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDVOLTLOWLIMIT      "</td><td> %3.2f </td></tr>"   , modbussettings.gridvoltlowlimit      ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDVOLTHIGHLIMIT     "</td><td> %3.2f </td></tr>"   , modbussettings.gridvolthighlimit     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDFREQLOWLIMIT      "</td><td> %3.2f </td></tr>"   , modbussettings.gridfreqlowlimit      ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDFREQHIGHLIMIT     "</td><td> %3.2f </td></tr>"   , modbussettings.gridfreqhighlimit     ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDVOLTLOWCONNLIMIT  "</td><td> %3.2f </td></tr>"   , modbussettings.gridvoltlowconnlimit  ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDVOLTHIGHCONNLIMIT "</td><td> %3.2f </td></tr>"   , modbussettings.gridvolthighconnlimit ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDFREQLOWCONNLIMIT  "</td><td> %3.2f </td></tr>"   , modbussettings.gridfreqlowconnlimit  ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_GRIDFREQHIGHCONNLIMIT "</td><td> %3.2f </td></tr>"   , modbussettings.gridfreqhighconnlimit ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_FIRMWARE              "</td><td> %s    </td></tr>"   , modbussettings.firmware              ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_CONTROLFIRMWARE       "</td><td> %s    </td></tr>"   , modbussettings.controlfirmware       ); strcat(webResponse, webLineBuffer);
  snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td>" STR_SERIAL                "</td><td> %s    </td></tr>"   , modbussettings.serial                ); strcat(webResponse, webLineBuffer);

  snprintf(webLineBuffer, WEB_LINE_SIZE, "</table></table></body></html>"); strcat(webResponse, webLineBuffer);
  return webResponse;
}

const char * growattIF::getWebModbuStatusPage()
{

#define WEB_LINE_SIZE  100
char webLineBuffer[WEB_LINE_SIZE];

  strcpy(webModbusResponse,  "<html><head></head><body><h2>Growatt Solar Inverter Modbus Status </h2>");
  snprintf(webLineBuffer, WEB_LINE_SIZE, "Last Transmission Status: %i - ",lastModbusTransmissionStatus ); strcat(webModbusResponse, webLineBuffer);
  strcat(webModbusResponse, getModbusErrorString(lastModbusTransmissionStatus));

  //Modbus Input Status
  strcat(webModbusResponse,  "<h3>Input Registers</h3><table>");

  for (int i = 0; i < INPUT_REGISTER_COUNT; i+=8) {
    snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td> %03i  </td><td>" , i ); strcat(webModbusResponse, webLineBuffer); 
    // Hex values
    for (int j = i ; j< i+8 ; j++) {
      snprintf(webLineBuffer, WEB_LINE_SIZE, "%04x ",inputRegisterContents[j] ); strcat(webModbusResponse, webLineBuffer);
    }
    //ASCII values
    strcat(webModbusResponse, " ");
    for (int j = i ; j< i+8 ; j++) {
      uint16_t low,high;
      high = inputRegisterContents[j]&0xff;
      low = inputRegisterContents[j]>>8;
      snprintf(webLineBuffer, WEB_LINE_SIZE, "%c%c", low > 31 && low < 127 ? low:46, high > 31 && high < 127 ? high:46);
      strcat(webModbusResponse, webLineBuffer);
    }
    strcat(webModbusResponse, "</td></tr>"); 
  }
  strcat(webModbusResponse, "</table>"); 

  //Modbus Holding Status
  strcat(webModbusResponse,  "<h3>Holding Registers</h3><table>");

  for (int i = 0; i < HOLDING_REGISTER_COUNT; i+=8) {
    snprintf(webLineBuffer, WEB_LINE_SIZE, "<tr><td> %03i </td><td>" , i ); strcat(webModbusResponse, webLineBuffer); 
    // Hex values
    for (int j = i ; j< i+8 ; j++) {
      snprintf(webLineBuffer, WEB_LINE_SIZE, "%04x ",holdingRegisterContents[j] ); strcat(webModbusResponse, webLineBuffer);
    }
    //ASCII values
    strcat(webModbusResponse, " ");
    for (int j = i ; j< i+8 ; j++) {
      uint16_t low,high;
      high = holdingRegisterContents[j]&0xff;
      low = holdingRegisterContents[j]>>8;
      snprintf(webLineBuffer, WEB_LINE_SIZE, "%c%c", low > 31 && low < 127 ? low:46, high > 31 && high < 127 ? high:46);
      strcat(webModbusResponse, webLineBuffer);
    }
    strcat(webModbusResponse, "</td></tr>"); 
  }
  strcat(webModbusResponse, "</table></body></html>"); 

  return webModbusResponse;
}




uint8_t growattIF::ReadInputRegisters() {

  ESP.wdtDisable();
  lastModbusTransmissionStatus = growattInterface.readInputRegisters(0 * 64, 64);
  ESP.wdtEnable(1);

  if (lastModbusTransmissionStatus == growattInterface.ku8MBSuccess)   
  {
    //local Buffer for Webinterface
    for (int i = 0*64; i< 64; i++) inputRegisterContents[i] = growattInterface.getResponseBuffer(i);
    // register 0-63
    //  Status and PV data
    modbusdata.status = growattInterface.getResponseBuffer(0);
    modbusdata.solarpower = ((growattInterface.getResponseBuffer(1) << 16) | growattInterface.getResponseBuffer(2)) * 0.1;

    modbusdata.pv1voltage = growattInterface.getResponseBuffer(3) * 0.1;
    modbusdata.pv1current = growattInterface.getResponseBuffer(4) * 0.1;
    modbusdata.pv1power = ((growattInterface.getResponseBuffer(5) << 16) | growattInterface.getResponseBuffer(6)) * 0.1;

    modbusdata.pv2voltage = growattInterface.getResponseBuffer(7) * 0.1;
    modbusdata.pv2current = growattInterface.getResponseBuffer(8) * 0.1;
    modbusdata.pv2power = ((growattInterface.getResponseBuffer(9) << 16) | growattInterface.getResponseBuffer(10)) * 0.1;

    // Output
    modbusdata.outputpower = ((growattInterface.getResponseBuffer(35) << 16) | growattInterface.getResponseBuffer(36)) * 0.1;
    modbusdata.gridfrequency = growattInterface.getResponseBuffer(37) * 0.01;
    modbusdata.gridvoltage = growattInterface.getResponseBuffer(38) * 0.1;

    // Energy
    modbusdata.energytoday = ((growattInterface.getResponseBuffer(53) << 16) | growattInterface.getResponseBuffer(54)) * 0.1;
    modbusdata.energytotal = ((growattInterface.getResponseBuffer(55) << 16) | growattInterface.getResponseBuffer(56)) * 0.1;
    modbusdata.totalworktime = ((growattInterface.getResponseBuffer(57) << 16) | growattInterface.getResponseBuffer(58)) * 0.5;

    modbusdata.pv1energytoday = ((growattInterface.getResponseBuffer(59) << 16) | growattInterface.getResponseBuffer(60)) * 0.1;
    modbusdata.pv1energytotal = ((growattInterface.getResponseBuffer(61) << 16) | growattInterface.getResponseBuffer(62)) * 0.1;
    overflow = growattInterface.getResponseBuffer(63);
  }
  else
  {
      return lastModbusTransmissionStatus;
  }
  delay(10); // if not bus error occours
  // next register block
  ESP.wdtDisable();
  lastModbusTransmissionStatus = growattInterface.readInputRegisters(1 * 64, 64);
  ESP.wdtEnable(1);

  if (lastModbusTransmissionStatus == growattInterface.ku8MBSuccess) 
  { // register 64 -127
      for (int i = 0; i< 64; i++) inputRegisterContents[i+64] = growattInterface.getResponseBuffer(i);

      modbusdata.pv2energytoday = ((overflow << 16) | growattInterface.getResponseBuffer(64 - 64)) * 0.1;
      modbusdata.pv2energytotal = ((growattInterface.getResponseBuffer(65 - 64) << 16) | growattInterface.getResponseBuffer(66 - 64)) * 0.1;

      // Temperatures
      modbusdata.tempinverter = growattInterface.getResponseBuffer(93 - 64) * 0.1;
      modbusdata.tempipm = growattInterface.getResponseBuffer(94 - 64) * 0.1;
      modbusdata.tempboost = growattInterface.getResponseBuffer(95 - 64) * 0.1;

      // Diag data
      modbusdata.ipf = growattInterface.getResponseBuffer(100 - 64);
      modbusdata.realoppercent = growattInterface.getResponseBuffer(101 - 64);
      modbusdata.opfullpower = ((growattInterface.getResponseBuffer(102 - 64) << 16) | growattInterface.getResponseBuffer(103 - 64)) * 0.1;
      modbusdata.deratingmode = growattInterface.getResponseBuffer(103 - 64);
      //  0:no derate;
      //  1:PV;
      //  2:*;
      //  3:Vac;
      //  4:Fac;
      //  5:Tboost;
      //  6:Tinv;
      //  7:Control;
      //  8:*;
      //  9:*OverBack
      //  ByTime;

      modbusdata.faultcode = growattInterface.getResponseBuffer(105 - 64);
      //  1~23 " Error: 99+x
      //  24 "Auto Test
      //  25 "No AC
      //  26 "PV Isolation Low"
      //  27 " Residual I
      //  28 " Output High
      //  29 " PV Voltage
      //  30 " AC V Outrange
      //  31 " AC F Outrange
      //  32 " Module Hot


      modbusdata.faultbitcode = ((growattInterface.getResponseBuffer(106 - 64) << 16) | growattInterface.getResponseBuffer(107 - 64));
      //  0x00000001 
      //  0x00000002 Communication error
      //  0x00000004 
      //  0x00000008 StrReverse or StrShort fault
      //  0x00000010 Model Init fault
      //  0x00000020 Grid Volt Sample diffirent
      //  0x00000040 ISO Sample diffirent
      //  0x00000080 GFCI Sample diffirent
      //  0x00000100 
      //  0x00000200 
      //  0x00000400 
      //  0x00000800 
      //  0x00001000 AFCI Fault
      //  0x00002000 
      //  0x00004000 AFCI Module fault
      //  0x00008000 
      //  0x00010000 
      //  0x00020000 Relay check fault
      //  0x00040000 
      //  0x00080000 
      //  0x00100000 
      //  0x00200000 Communication error
      //  0x00400000 Bus Voltage error
      //  0x00800000 AutoTest fail
      //  0x01000000 No Utility
      //  0x02000000 PV Isolation Low
      //  0x04000000 Residual I High
      //  0x08000000 Output High DCI
      //  0x10000000 PV Voltage high
      //  0x20000000 AC V Outrange
      //  0x40000000 AC F Outrange
      //  0x80000000 TempratureHigh

      modbusdata.warningbitcode = ((growattInterface.getResponseBuffer(110 - 64) << 16) | growattInterface.getResponseBuffer(111 - 64));
      //  0x0001 Fan warning
      //  0x0002 String communication abnormal
      //  0x0004 StrPIDconfig Warning
      //  0x0008 
      //  0x0010 DSP and COM firmware unmatch
      //  0x0020 
      //  0x0040 SPD abnormal
      //  0x0080 GND and N connect abnormal
      //  0x0100 PV1 or PV2 circuit short
      //  0x0200 PV1 or PV2 boost driver broken
      //  0x0400 
      //  0x0800 
      //  0x1000 
      //  0x2000 
      //  0x4000 
      //  0x8000
  }
  else
  {
      return lastModbusTransmissionStatus;
  }
  
  return Success;
}

void growattIF::PublishInputRegisters(PubSubClient * mqtt,char * topicData)
{
  char thisTopic[TMP_BUFFER_SIZE];
  char thisValue[TMP_BUFFER_SIZE];

  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_STATUS        , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.status         ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_SOLARPOWER    , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.solarpower     ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV1VOLTAGE    , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv1voltage     ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV1CURRENT    , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv1current     ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV1POWER      , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv1power       ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV2VOLTAGE    , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv2voltage     ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV2CURRENT    , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv2current     ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV2POWER      , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv2power       ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_OUTPUTPOWER   , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.outputpower    ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDFREQUENCY , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.gridfrequency  ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDVOLTAGE   , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.gridvoltage    ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_ENERGYTODAY   , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.energytoday    ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_ENERGYTOTAL   , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.energytotal    ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_TOTALWORKTIME , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.totalworktime  ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV1ENERGYTODAY, topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv1energytoday ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV1ENERGYTOTAL, topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv1energytotal ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV2ENERGYTODAY, topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv2energytoday ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_PV2ENERGYTOTAL, topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.pv2energytotal ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_OPFULLPOWER   , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.opfullpower    ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_TEMPINVERTER  , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.tempinverter   ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_TEMPIPM       , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.tempipm        ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_TEMPBOOST     , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbusdata.tempboost      ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_IPF           , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.ipf            ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_REALOPPERCENT , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.realoppercent  ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_DERATINGMODE  , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.deratingmode   ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_FAULTCODE     , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.faultcode      ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_FAULTBITCODE  , topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.faultbitcode   ); mqtt->publish(thisTopic, thisValue);
  snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_WARNINGBITCODE, topicData  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%d", modbusdata.warningbitcode ); mqtt->publish(thisTopic, thisValue);
  return;
}



  uint8_t growattIF::ReadHoldingRegisters()
  {
    ESP.wdtDisable();
    lastModbusTransmissionStatus = growattInterface.readHoldingRegisters(0 * 64, 64);
    ESP.wdtEnable(1);

    if (lastModbusTransmissionStatus == growattInterface.ku8MBSuccess)
    {
       for (int i = 0; i< 64; i++) holdingRegisterContents[i] = growattInterface.getResponseBuffer(i);

      // register 0-63
        modbussettings.enable = growattInterface.getResponseBuffer(0);
        modbussettings.safetyfuncen = growattInterface.getResponseBuffer(1); // Safety Function Enabled
        //  Bit0: SPI enable
        //  Bit1: AutoTestStart
        //  Bit2: LVFRT enable
        //  Bit3: FreqDerating Enable
        //  Bit4: Softstart enable
        //  Bit5: DRMS enable
        //  Bit6: Power Volt Func Enable
        //  Bit7: HVFRT enable
        //  Bit8: ROCOF enable
        //  Bit9: Recover FreqDerating Mode Enable
        //  Bit10~15: Reserved
        modbussettings.maxoutputactivepp = growattInterface.getResponseBuffer(3);   // Inverter Max output active power percent  0-100: %, 255: not limited
        modbussettings.maxoutputreactivepp = growattInterface.getResponseBuffer(4); // Inverter Max output reactive power percent  0-100: %, 255: not limited
        modbussettings.maxpower = ((growattInterface.getResponseBuffer(6) << 16) | growattInterface.getResponseBuffer(7)) * 0.1;
        modbussettings.voltnormal = growattInterface.getResponseBuffer(8) * 0.1;

        modbussettings.firmware[0] = growattInterface.getResponseBuffer(9) >> 8;
        modbussettings.firmware[1] = growattInterface.getResponseBuffer(9) & 0xff;
        modbussettings.firmware[2] = growattInterface.getResponseBuffer(10) >> 8;
        modbussettings.firmware[3] = growattInterface.getResponseBuffer(10) & 0xff;
        modbussettings.firmware[4] = growattInterface.getResponseBuffer(11) >> 8;
        modbussettings.firmware[5] = growattInterface.getResponseBuffer(11) & 0xff;
        modbussettings.firmware[6] = '\0';

        modbussettings.controlfirmware[0] = growattInterface.getResponseBuffer(12) >> 8;
        modbussettings.controlfirmware[1] = growattInterface.getResponseBuffer(12) & 0xff;
        modbussettings.controlfirmware[2] = growattInterface.getResponseBuffer(13) >> 8;
        modbussettings.controlfirmware[3] = growattInterface.getResponseBuffer(13) & 0xff;
        modbussettings.controlfirmware[4] = growattInterface.getResponseBuffer(14) >> 8;
        modbussettings.controlfirmware[5] = growattInterface.getResponseBuffer(14) & 0xff;
        modbussettings.controlfirmware[6] = '\0';

        modbussettings.startvoltage = growattInterface.getResponseBuffer(17) * 0.1;

        modbussettings.serial[0] = growattInterface.getResponseBuffer(23) >> 8;
        modbussettings.serial[1] = growattInterface.getResponseBuffer(23) & 0xff;
        modbussettings.serial[2] = growattInterface.getResponseBuffer(24) >> 8;
        modbussettings.serial[3] = growattInterface.getResponseBuffer(24) & 0xff;
        modbussettings.serial[4] = growattInterface.getResponseBuffer(25) >> 8;
        modbussettings.serial[5] = growattInterface.getResponseBuffer(25) & 0xff;
        modbussettings.serial[6] = growattInterface.getResponseBuffer(26) >> 8;
        modbussettings.serial[7] = growattInterface.getResponseBuffer(26) & 0xff;
        modbussettings.serial[8] = growattInterface.getResponseBuffer(27) >> 8;
        modbussettings.serial[9] = growattInterface.getResponseBuffer(27) & 0xff;
        modbussettings.serial[10] = '\0';

        modbussettings.gridvoltlowlimit = growattInterface.getResponseBuffer(52) * 0.1;
        modbussettings.gridvolthighlimit = growattInterface.getResponseBuffer(53) * 0.1;
        modbussettings.gridfreqlowlimit = growattInterface.getResponseBuffer(54) * 0.01;
        modbussettings.gridfreqhighlimit = growattInterface.getResponseBuffer(55) * 0.01;
    }
    else
    {
     return lastModbusTransmissionStatus;
    }
    delay(10);
    ESP.wdtDisable();
    lastModbusTransmissionStatus = growattInterface.readHoldingRegisters(1 * 64, 64);
    ESP.wdtEnable(1);

    if (lastModbusTransmissionStatus == growattInterface.ku8MBSuccess)
    {
     for (int i = 0; i< 64; i++) holdingRegisterContents[i+64] = growattInterface.getResponseBuffer(i);

     // register 64 -127
      modbussettings.gridvoltlowconnlimit = growattInterface.getResponseBuffer(64 - 64) * 0.1;
      modbussettings.gridvolthighconnlimit = growattInterface.getResponseBuffer(65 - 64) * 0.1;
      modbussettings.gridfreqlowconnlimit = growattInterface.getResponseBuffer(66 - 64) * 0.01;
      modbussettings.gridfreqhighconnlimit = growattInterface.getResponseBuffer(67 - 64) * 0.01;

      modbussettings.modul = growattInterface.getResponseBuffer(121 - 64);
    }
    else
    {
      return lastModbusTransmissionStatus;
    }
    /*{ // register 128-191
        //          //          modbussettings.modul = growattInterface.getResponseBuffer(130 - 128);
    
    //}
    else
    {
      return result;
    }*/
    return Success;
}

void growattIF::PublishHoldingRegisters(PubSubClient * mqtt,char * topicSettings)
{
  char thisTopic[TMP_BUFFER_SIZE];
  char thisValue[TMP_BUFFER_SIZE];

snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_ENABLE               , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%i", modbussettings.enable                ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_SAFETYFUNCEN         , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%i", modbussettings.safetyfuncen          ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_MAXOUTPUTACTIVEPP    , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%i", modbussettings.maxoutputactivepp     ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_MAXOUTPUTREACTIVEPP  , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%i", modbussettings.maxoutputreactivepp   ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_MODUL                , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%i", modbussettings.modul                 ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_MAXPOWER             , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.maxpower              ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_VOLTNORMAL           , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.voltnormal            ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_STARTVOLTAGE         , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.startvoltage          ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDVOLTLOWLIMIT     , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridvoltlowlimit      ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDVOLTHIGHLIMIT    , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridvolthighlimit     ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDFREQLOWLIMIT     , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridfreqlowlimit      ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDFREQHIGHLIMIT    , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridfreqhighlimit     ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDVOLTLOWCONNLIMIT , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridvoltlowconnlimit  ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDVOLTHIGHCONNLIMIT, topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridvolthighconnlimit ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDFREQLOWCONNLIMIT , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridfreqlowconnlimit  ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_GRIDFREQHIGHCONNLIMIT, topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%f", modbussettings.gridfreqhighconnlimit ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_FIRMWARE             , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%s", modbussettings.firmware              ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_CONTROLFIRMWARE      , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%s", modbussettings.controlfirmware       ); mqtt->publish(thisTopic, thisValue);
snprintf(thisTopic, TMP_BUFFER_SIZE, "%s/" STR_SERIAL               , topicSettings  ) ; snprintf(thisValue, TMP_BUFFER_SIZE, "%s", modbussettings.serial                ); mqtt->publish(thisTopic, thisValue);
return;
}

const char * growattIF::getModbusErrorString(uint8_t errorCode)
{
  switch (errorCode){
    case growattInterface.ku8MBSuccess:             return "Success";
    case growattInterface.ku8MBIllegalFunction:     return "Illegal function";
    case growattInterface.ku8MBIllegalDataAddress:  return "Illegal data address" ;
    case growattInterface.ku8MBIllegalDataValue:    return "Illegal data value" ;
    case growattInterface.ku8MBSlaveDeviceFailure:  return "Slave device failure" ;
    case growattInterface.ku8MBInvalidSlaveID:      return "Invalid slave ID";
    case growattInterface.ku8MBInvalidFunction:     return "Invalid function";;
    case growattInterface.ku8MBResponseTimedOut:    return "Response timed out";
    case growattInterface.ku8MBInvalidCRC:          return "Invalid CRC" ;
    default: snprintf(unknownModbusExceptionString, TMP_BUFFER_SIZE, "Modbus Exception Code: %u", errorCode ) ;return unknownModbusExceptionString;
  }
}
const char * growattIF::getInverterStatusString(uint8_t statusCode)
{
  switch (statusCode){
    case 0:             return "0 - Waiting";
    case 1:             return "1 - Normal";
    case 3:             return "3 -Fault";
    default: snprintf(unknownInverterStatusString, TMP_BUFFER_SIZE, "%u", statusCode  ) ;return unknownInverterStatusString;
  }
}
