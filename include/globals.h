#ifndef GLOBALS_H
#define GLOBALS_H

unsigned long uptime, seconds;
bool holdingregisters = true;
const char buildversion[]="v1.3.0Helge";

#define EE_START_ADDR 0x00 // start address of values stored in the eeprom
#define EE_INIT_STATE_SIZE 4
#define EE_INIT_PATTERN "12f4"
#define MQTT_PARAM_ID "mqttRSSI"
#define CONFIG_SIGN  "1234"

typedef struct
{
    char  EEpromInit[EE_INIT_STATE_SIZE]; // pattern if EEprom is used
    uint16 modbus_update_sec;         // 1: modbus device is read every second and data are anounced via mqtt
    uint16 status_update_sec;         // 10: status mqtt message is sent every 10 seconds
    uint16 wificheck_sec;             // 1: every second
    char  id[sizeof(MQTT_PARAM_ID)];
    char  mqttServer[33];
    char  apikey[17];
    char  channelId[9];
    char  writekey[17];
    char  clientId[25];
    char  username[25];
    char  password[25];
    char  hostname[25];
    unsigned long publishInterval;
} configData_t;

static configData_t  config;

#endif

