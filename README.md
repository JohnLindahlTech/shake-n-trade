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