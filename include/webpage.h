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

#endif