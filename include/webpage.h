#ifndef WEBPAGE_H
#define WEBPAGE_H

static const char PAGE_HELLO[] = R"(
{
  "uri": "/",
  "title": "Hello",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Hello, World"
    },
  ]
}
)";

#endif