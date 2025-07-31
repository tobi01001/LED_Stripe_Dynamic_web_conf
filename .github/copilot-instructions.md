# Project Overview

This project is a providing LED effects on WS2812 LED stripes connected to an ESP2866 controller.
It includes a web application that allows users to control the led strips via web. It implements a GET API and websocket to allow external control from home automation system like fhem.
If enabled (via include flag) it also provides local display support and control via rotray knob with button.


## Folder Structure

- `/src`: Contains the source code for the main application, inluding subfolders for effects and led strip control.
- `/src/LED_strip`: overall led_strip control, but also containing fields managing strip, api and web variables and functions
- `/src/WS2812FX`: contains overall functionality for the strip (service, common functions)
- `/src/WS2812FX/effects`: all the impementable effects in their respective effect classes
- `/data`: contains the content for the web frontend to be uploaded to the filesystem (littlefs)
- `/env_data_folders`: Contains generic files for the frontend as well as environment specific ones. The generic ones will be modifed accoring to the environment and both modifed generic and specific get copied (via script) to the /data folder before building the file system
- `/include`: Header file with defaults (i.e. default configuration options and constants). Header file for Debug output if enabled.
- `/lib`: Local copies of libraries with custom changes (or a specific version) as required.


## Libraries and Frameworks

- ArduinoJson@6.21.5
- thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays
- pasko-zh/Brzo I2C 
- lennarthennigs/Button2 ; was 1.6.5 - updated too 2.0.3 on 2022-05-30
- https://github.com/amkrk/ESPAsyncWebServer.git  ; was 306 me-no-dev/ESP Async WebServer
- me-no-dev/ESPAsyncTCP
- alanswx/ESPAsyncWiFiManager
- xoseperez/EEPROM_Rotate@^0.9.2
- custom libraries in `/lib`

- platform = espressif8266@2.6.3 ;https://github.com/platformio/platform-espressif8266.git ;espressif8266@2.2.3
- framework = arduino

## Coding Standards, Coding and review guidelines
- Use camelCase naming, while private variables start with _
- use comments for automatic documentation creation
- primary objectve is runtime and memory efficient code for embedded use with limited ressouces (or dedicated use on the HW mentioned in platformio.ini) - where applicable:
  - avoid uneccessary overhead
  - avoid multiple type casts
  - prefer Flash over RAM
  - avoid dynamic memory - especially use of String
- further:
  - focus on modularity, maintainability and readability
  - focus on robustness (failsafe code)
- this code is for platform.io
- enforce the https://isocpp.github.io/CppCoreGuidelines/ CppCoreGuidelines

