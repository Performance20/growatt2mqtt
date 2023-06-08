#ifndef WEBPAGE_H
#define WEBPAGE_H

static const char PAGE_MQTT[] PROGMEM = R"(
{
  "uri": "/mqttsettings",
  "title": "MQTT Settings",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Hello, World"
    }
  ]
}
)";


static const char AUX_MQTT[] PROGMEM = R"*(
[
  {
    "title": "MQTT Setting",
    "uri": "/mqtt_setting",
    "menu": true,
    "element": [
      {
        "name": "style",
        "type": "ACStyle",
        "value": "label+input,label+select{position:sticky;left:140px;width:204px!important;box-sizing:border-box;}"
      },
      {
        "name": "header",
        "type": "ACElement",
        "value": "<h2 style='text-align:center;color:#2f4f4f;margin-top:10px;margin-bottom:10px'>MQTT Broker settings</h2>"
      },
      {
        "name": "caption",
        "type": "ACText",
        "value": "Publish WiFi signal strength via MQTT, publishing the RSSI value of the ESP module to the ThingSpeak public channel.",
        "style": "font-family:serif;color:#053d76",
        "posterior": "par"
      },
      {
        "name": "mqttserver",
        "type": "ACInput",
        "label": "Server",
        "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$",
        "placeholder": "MQTT broker server",
        "global": true
      },
      {
        "name": "apikey",
        "type": "ACInput",
        "label": "User API Key",
        "global": true
      },
      {
        "name": "channelid",
        "type": "ACInput",
        "label": "Channel ID",
        "pattern": "^[0-9]{6}$",
        "global": true
      },
      {
        "name": "writekey",
        "type": "ACInput",
        "label": "Write API Key",
        "global": true
      },
      {
        "name": "nl1",
        "type": "ACElement",
        "value": "<hr>"
      },
      {
        "name": "credential",
        "type": "ACText",
        "value": "MQTT Device Credentials",
        "style": "font-weight:bold;color:#1e81b0",
        "posterior": "div"
      },
      {
        "name": "clientid",
        "type": "ACInput",
        "label": "Client ID",
        "global": true
      },
      {
        "name": "username",
        "type": "ACInput",
        "label": "Username",
        "global": true
      },
      {
        "name": "password",
        "type": "ACInput",
        "label": "Password",
        "apply": "password",
        "global": true
      },
      {
        "name": "nl2",
        "type": "ACElement",
        "value": "<hr>"
      },
      {
        "name": "period",
        "type": "ACRadio",
        "value": [
          "30 sec.",
          "60 sec.",
          "180 sec."
        ],
        "label": "Update period",
        "arrange": "vertical",
        "global": true
      },
      {
        "name": "hostname",
        "type": "ACInput",
        "value": "",
        "label": "ESP host name",
        "pattern": "^([a-zA-Z0-9]([a-zA-Z0-9-])*[a-zA-Z0-9]){1,24}$",
        "global": true
      },
      {
        "name": "save",
        "type": "ACSubmit",
        "value": "Save&amp;Start",
        "uri": "/mqtt_start"
      },
      {
        "name": "discard",
        "type": "ACSubmit",
        "value": "Discard",
        "uri": "/"
      },
      {
        "name": "stop",
        "type": "ACSubmit",
        "value": "Stop publishing",
        "uri": "/mqtt_stop"
      }
    ]
  },
  {
    "title": "MQTT Setting",
    "uri": "/mqtt_start",
    "menu": false,
    "element": [
      {
        "name": "c1",
        "type": "ACText",
        "value": "<h4>MQTT publishing has started.</h4>",
        "style": "text-align:center;color:#2f4f4f;padding:5px;"
      },
      {
        "name": "c2",
        "type": "ACText",
        "value": "<h4>Parameters saved as:</h4>",
        "style": "text-align:center;color:#2f4f4f;padding:5px;"
      },
      {
        "name": "mqttserver",
        "type": "ACText",
        "format": "Server: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "apikey",
        "type": "ACText",
        "format": "User API Key: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "channelid",
        "type": "ACText",
        "format": "Channel ID: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "writekey",
        "type": "ACText",
        "format": "Write API Key: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "clientid",
        "type": "ACText",
        "format": "Client ID: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "username",
        "type": "ACText",
        "format": "Username: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "password",
        "type": "ACText",
        "format": "Password: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "hostname",
        "type": "ACText",
        "format": "ESP host: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "period",
        "type": "ACText",
        "format": "Update period: %s",
        "posterior": "br",
        "global": true
      },
      {
        "name": "clear",
        "type": "ACSubmit",
        "value": "Clear channel",
        "uri": "/mqtt_clear"
      }
    ]
  },
  {
    "title": "MQTT Setting",
    "uri": "/mqtt_clear",
    "menu": false,
    "response": false
  },
  {
    "title": "MQTT Setting",
    "uri": "/mqtt_stop",
    "menu": false,
    "response": false
  }
]
)*";

/*
{
  "title": "MQTT Setting",
  "uri": "/mqtt_setting",
  "menu": true,
  "element": [
    {
      "name": "style",
      "type": "ACStyle",
      "value": "label+input,label+select{position:sticky;left:140px;width:204px!important;box-sizing:border-box;}"
    },
    {
      "name": "header",
      "type": "ACElement",
      "value": "<h2 style='text-align:center;color:#2f4f4f;margin-top:10px;margin-bottom:10px'>MQTT Broker settings</h2>"
    },
    {
      "name": "caption",
      "type": "ACText",
      "value": "Publish WiFi signal strength via MQTT, publishing the RSSI value of the ESP module to the ThingSpeak public channel.",
      "style": "font-family:serif;color:#053d76",
      "posterior": "par"
    },
    {
      "name": "mqttserver",
      "type": "ACInput",
      "label": "Server",
      "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$",
      "placeholder": "MQTT broker server",
      "global": true
    },
    {
      "name": "apikey",
      "type": "ACInput",
      "label": "User API Key",
      "global": true
    },
    {
      "name": "channelid",
      "type": "ACInput",
      "label": "Channel ID",
      "pattern": "^[0-9]{6}$",
      "global": true
    },
    {
      "name": "writekey",
      "type": "ACInput",
      "label": "Write API Key",
      "global": true
    },
    {
      "name": "nl1",
      "type": "ACElement",
      "value": "<hr>"
    },
    {
      "name": "credential",
      "type": "ACText",
      "value": "MQTT Device Credentials",
      "style": "font-weight:bold;color:#1e81b0",
      "posterior": "div"
    },
    {
      "name": "clientid",
      "type": "ACInput",
      "label": "Client ID",
      "global": true
    },
    {
      "name": "username",
      "type": "ACInput",
      "label": "Username",
      "global": true
    },
    {
      "name": "password",
      "type": "ACInput",
      "label": "Password",
      "apply": "password",
      "global": true
    },
    {
      "name": "nl2",
      "type": "ACElement",
      "value": "<hr>"
    },
    {
      "name": "period",
      "type": "ACRadio",
      "value": [
        "30 sec.",
        "60 sec.",
        "180 sec."
      ],
      "label": "Update period",
      "arrange": "vertical",
      "global": true
    },
    {
      "name": "hostname",
      "type": "ACInput",
      "value": "",
      "label": "ESP host name",
      "pattern": "^([a-zA-Z0-9]([a-zA-Z0-9-])*[a-zA-Z0-9]){1,24}$",
      "global": true
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save&amp;Start",
      "uri": "/mqtt_start"
    },
    {
      "name": "discard",
      "type": "ACSubmit",
      "value": "Discard",
      "uri": "/"
    },
    {
      "name": "stop",
      "type": "ACSubmit",
      "value": "Stop publishing",
      "uri": "/mqtt_stop"
    }
  ]
}

*/

#endif