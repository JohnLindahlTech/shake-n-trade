# Shake'n'Trade

## Hardware
M5Stack Core 2 ESP32 IoT Dev kit

[https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit](https://shop.m5stack.com/products/m5stack-core2-esp32-iot-development-kit)

## Setup
### Arduino IDE
[https://www.arduino.cc/en/software](https://www.arduino.cc/en/software) 

### Dependencies
* Libraries & Drivers for Core 2: [https://docs.m5stack.com/en/quick_start/arduino](https://docs.m5stack.com/en/quick_start/arduino)
* ArduinoJson: [https://arduinojson.org/v6/doc/installation/](https://arduinojson.org/v6/doc/installation/)
* ArduinoHttpClient: [https://github.com/arduino-libraries/ArduinoHttpClient](https://github.com/arduino-libraries/ArduinoHttpClient)

#### Dependencies, Not yet used
* Arduino ESP32 fs uploader: [https://github.com/me-no-dev/arduino-esp32fs-plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin)
* Arduino Audio: [https://github.com/earlephilhower/ESP8266Audio/tree/master](https://github.com/earlephilhower/ESP8266Audio/tree/master)
    * See [PlayMP3FromSPIFFS example](https://github.com/earlephilhower/ESP8266Audio/tree/master/examples/PlayMP3FromSPIFFS)

## Notes
To program the Core 2 from Mac, remember to pull `G0` (4th pin from bottom, on right side) low, by grounding (top 3 pins on left side), before (and during start of) the uploading phase.

Note/unsure: Should probably not be `G0`, but rather `RST`/`EN`, which is pin 3 from top on right side.

## Troubleshoot

### Problems with void M5Display::drawPngUrl
If compile gives the following error:
```
Arduino\libraries\M5Core2\src\M5Display.cpp: In member function 'void M5Display::drawPngUrl(const char*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, double, uint8_t)':
Arduino\libraries\M5Core2\src\M5Display.cpp:563:3: error: 'HTTPClient' was not declared in this scope
   HTTPClient http;
   ^~~~~~~~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:563:3: note: suggested alternative: 'HttpClient'
   HTTPClient http;
   ^~~~~~~~~~
   HttpClient
Arduino\libraries\M5Core2\src\M5Display.cpp:565:7: error: 'WiFi' was not declared in this scope
   if (WiFi.status() != WL_CONNECTED) {
       ^~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:565:24: error: 'WL_CONNECTED' was not declared in this scope
   if (WiFi.status() != WL_CONNECTED) {
                        ^~~~~~~~~~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:570:3: error: 'http' was not declared in this scope
   http.begin(url);
   ^~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:573:19: error: 'HTTP_CODE_OK' was not declared in this scope
   if (httpCode != HTTP_CODE_OK) {
                   ^~~~~~~~~~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:579:3: error: 'WiFiClient' was not declared in this scope
   WiFiClient *stream = http.getStreamPtr();
   ^~~~~~~~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:579:3: note: suggested alternative: 'HttpClient'
   WiFiClient *stream = http.getStreamPtr();
   ^~~~~~~~~~
   HttpClient
Arduino\libraries\M5Core2\src\M5Display.cpp:579:15: error: 'stream' was not declared in this scope
   WiFiClient *stream = http.getStreamPtr();
               ^~~~~~
Arduino\libraries\M5Core2\src\M5Display.cpp:579:15: note: suggested alternative: 'Stream'
   WiFiClient *stream = http.getStreamPtr();
               ^~~~~~
               Stream
exit status 1

Compilation error: exit status 1
```

Just go to the [~/Documents]/Arduino\libraries\M5Core2\src\M5Display.cpp and remove the content of the function `void M5Display::drawPngUrl(const char*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, double, uint8_t)`