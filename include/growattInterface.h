#ifndef GROWATTINTERFACE_H
#define GROWATTINTERFACE_H

#include "Arduino.h"
#include <PubSubClient.h>     // MQTT support
#include <ModbusMaster.h>         // Modbus master library for ESP8266
#include <SoftwareSerial.h>       // Leave the main serial line (USB) for debugging and flashing


class growattIF {
#define SLAVE_ID        1         // Default slave ID of Growatt
#define MODBUS_RATE     9600      // Modbus speed of Growatt, do not change

#define TMP_BUFFER_SIZE  50 // used for String Operations
  private:
    ModbusMaster growattInterface;
    SoftwareSerial *serial;
    void preTransmission();
    void postTransmission();
    int PinMAX485_RE_NEG;
    int PinMAX485_DE;
    int PinMAX485_RX;
    int PinMAX485_TX;
    int setcounter = 0;
    int overflow;


    
    #define INPUT_REGISTER_COUNT 128
    uint16_t inputRegisterContents[INPUT_REGISTER_COUNT];
    #define HOLDING_REGISTER_COUNT 128
    uint16_t holdingRegisterContents[HOLDING_REGISTER_COUNT];

    
    #define WEB_RESPONSE_SIZE 8192
    char webResponse[WEB_RESPONSE_SIZE];

    #define WEB_MODBUS_RESPONSE_SIZE 8192
    char webModbusResponse[WEB_MODBUS_RESPONSE_SIZE];

    struct modbus_input_registers
    {
      int   status; 
      float solarpower;
      float pv1voltage;
      float pv1current;
      float pv1power;
      float pv2voltage;
      float pv2current;
      float pv2power;
      float outputpower;
      float gridfrequency;
      float gridvoltage;
      float energytoday;
      float energytotal; 
      float totalworktime;
      float pv1energytoday;
      float pv1energytotal;
      float pv2energytoday;
      float pv2energytotal;
      float opfullpower;
      float tempinverter;
      float tempipm;
      float tempboost;
      int   ipf;
      int   realoppercent;
      int   deratingmode;
      int   faultcode;
      int   faultbitcode; 
      int   warningbitcode;
    };
    struct modbus_input_registers modbusdata;


#define STR_STATUS          "status"          
#define STR_SOLARPOWER      "solarpower"      
#define STR_PV1VOLTAGE      "pv1voltage"        
#define STR_PV1CURRENT      "pv1current"        
#define STR_PV1POWER        "pv1power"            
#define STR_PV2VOLTAGE      "pv2voltage"        
#define STR_PV2CURRENT      "pv2current"          
#define STR_PV2POWER        "pv2power"            
#define STR_OUTPUTPOWER     "outputpower"   
#define STR_GRIDFREQUENCY   "gridfrequency" 
#define STR_GRIDVOLTAGE     "gridvoltage"   
#define STR_ENERGYTODAY     "energytoday"   
#define STR_ENERGYTOTAL     "energytotal"     
#define STR_TOTALWORKTIME   "totalworktime"     
#define STR_PV1ENERGYTODAY  "pv1energytoday"    
#define STR_PV1ENERGYTOTAL  "pv1energytotal"    
#define STR_PV2ENERGYTODAY  "pv2energytoday"  
#define STR_PV2ENERGYTOTAL  "pv2energytotal"
#define STR_OPFULLPOWER     "opfullpower"     
#define STR_TEMPINVERTER    "tempinverter"    
#define STR_TEMPIPM         "tempipm"         
#define STR_TEMPBOOST       "tempboost"         
#define STR_IPF             "ipf"             
#define STR_REALOPPERCENT   "realoppercent"   
#define STR_DERATINGMODE    "deratingmode"    
#define STR_FAULTCODE       "faultcode"         
#define STR_FAULTBITCODE    "faultbitcode"        
#define STR_WARNINGBITCODE  "warningbitcode"      

    struct modbus_holding_registers
    {
      int   enable                ; 
      int   safetyfuncen          ; 
      int   maxoutputactivepp     ;
      int   maxoutputreactivepp   ; 
      int   modul                 ;
      float maxpower              ;
      float voltnormal            ;
      float startvoltage          ;
      float gridvoltlowlimit      ;
      float gridvolthighlimit     ; 
      float gridfreqlowlimit      ;
      float gridfreqhighlimit     ;
      float gridvoltlowconnlimit  ;
      float gridvolthighconnlimit ; 
      float gridfreqlowconnlimit  ;
      float gridfreqhighconnlimit ;
      char  firmware[7]           ;
      char  controlfirmware[7]    ;
      char  serial[11]            ;
    };
    struct modbus_holding_registers modbussettings;

#define STR_ENABLE                "enable"
#define STR_SAFETYFUNCEN          "safetyfuncen"
#define STR_MAXOUTPUTACTIVEPP     "maxoutputactivepp"
#define STR_MAXOUTPUTREACTIVEPP   "maxoutputreactivepp"
#define STR_MODUL                 "modul"
#define STR_MAXPOWER              "maxpower"
#define STR_VOLTNORMAL            "voltnormal"
#define STR_STARTVOLTAGE          "startvoltage"
#define STR_GRIDVOLTLOWLIMIT      "gridvoltlowlimit"
#define STR_GRIDVOLTHIGHLIMIT     "gridvolthighlimit"
#define STR_GRIDFREQLOWLIMIT      "gridfreqlowlimit"
#define STR_GRIDFREQHIGHLIMIT     "gridfreqhighlimit"
#define STR_GRIDVOLTLOWCONNLIMIT  "gridvoltlowconnlimit "
#define STR_GRIDVOLTHIGHCONNLIMIT "gridvolthighconnlimit"
#define STR_GRIDFREQLOWCONNLIMIT  "gridfreqlowconnlimit "
#define STR_GRIDFREQHIGHCONNLIMIT "gridfreqhighconnlimit"
#define STR_FIRMWARE              "firmware"
#define STR_CONTROLFIRMWARE       "controlfirmware"
#define STR_SERIAL                "serial"


  public:
    growattIF(int _PinMAX485_RE_NEG, int _PinMAX485_DE, int _PinMAX485_RX, int _PinMAX485_TX);
    void initGrowatt();
    uint8_t writeRegister(uint16_t reg, uint16_t message);
    uint16_t readRegister(uint16_t reg);

    const char * getWebStatusPage(unsigned long uptime);
    const char * getWebModbuStatusPage();

    uint8_t ReadInputRegisters();
    void PublishInputRegisters(PubSubClient * mqtt,char * topicRoot);

    uint8_t ReadHoldingRegisters();
    void PublishHoldingRegisters(PubSubClient * mqtt,char * topicRoot);

    const char * getModbusErrorString(uint8_t errorCode);
    const char * getInverterStatusString(uint8_t statusCode);

    uint8_t lastModbusTransmissionStatus;    
    char unknownModbusExceptionString[TMP_BUFFER_SIZE];    
    char unknownInverterStatusString[TMP_BUFFER_SIZE];    

    // Error codes
    static const uint8_t Success    = 0x00;
    //static const uint8_t Continue   = 0xFF;

    // Growatt Holding registers
    static const uint8_t regOnOff           = 0;
    static const uint8_t regMaxOutputActive = 3;
    static const uint8_t regStartVoltage    = 17;
    static const uint8_t regModulPower      = 121;
};

#endif
