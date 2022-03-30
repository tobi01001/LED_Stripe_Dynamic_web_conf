This work is based on inredibly good work from many others... I just modified it to my personal use.

So thanks to:
-------------
   - FastLED library - see http://www.fastLed.io
   - ESPWebserver - see https://github.com/jasoncoon/esp8266-fastled-webserver
   - Adafruit Neopixel https://github.com/adafruit/Adafruit_NeoPixel
   - WS2812FX library https://github.com/kitesurfer1404/WS2812FX
   - fhem esp8266 implementation (see https://github.com/tobi01001/FHEM-LED_CONTROL-) - Idea from https://github.com/sw-home/FHEM-LEDStripe 


# Kindly introducing KNOB Control!!
Now this is awesome as it provides a direct haptic feedback and direct control to the LEDs with a nice and shiny display. :-)
Using platformio, this should be ready to be used rightaway.

HW used (in addition to the LED stripe and the WEMOS D1 mini (or other ESP8266) of course): 
- Rotary Encoder KY-040
- 0.96 128x64 I2C OLED
- 2x Capacitors 6n8
- 1x Capacitor 1µF
- 2x Resistor 1700 Ohm
- 1x Resistor 980 Ohm
Schematics:
![Knob Control Schematics](https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf/blob/main/circuit.png)
The debouncing circuits (RC part) may not be required but makes the rotary input quite stable.

The rest of the Project can be modified to your needs in platformio. See the Build flags in [platformio.ini]( https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf/blob/main/platformio.ini) to see how to define the number of LEDs, the Controller / LED lamp name and if it has Knob control.

Have fun and let me know if theres anything not working as expected....

Pictures:
![Status Screen](https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf/blob/main/20200227_210857.jpg)
Switches Off after some time to be not "disturbing" and switches on if one uses the rotary encoder and on some other conditions.
![Menu Screen](https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf/blob/main/20200227_210707.jpg)
Sub Menu screen for a boolean parameter. Numbers are represented as progress bar and value. Text selection as scrollable list.
The menu Screen follows the same structure as the online version and is derived from the code...

# LED_Stripe_Dynamic_web_conf
LED Stripe including simple Web Server, basics from the ws2812fx library - but different effects etc... 
FHEM control is possible with the related perl fhem module...

- need to describe installation etc. in more detail - first approach below

Major differences to WS2812FX / ESPWebserver:

- somehow combines the "best of both"
- almost everything is adjustable via web interface / get-interface
- includes (basic) sunrise / sunset effects with adjustable time (minutes) 
- a void effect which can be used to remotely change sinlge LEDs or ranges
- everything is smoothed, i.e. switching and changeing is smoothened to the max, brighness change is smoothed...
- can be controlled from FHEM (home automation - separate module not yet published)
- heavily use of color palettes...
- setting of parameters / modes is no longer a complete "REST" interface but done with HTTP GET query strings. This allows changing more parameters at once (either via URL with query string directly or via home automation...). e.g. to activate effect number 2 with color palette number 3 and the speed set to 1000 it just needs: http://esp.address:/set?mo=2&pa=3&sp=1000 - changes are communicated via HTTP response and websocket broadcast. The current state can be checked with http://esp.address/status
- every setting is stored to the EEPROM (with CRC protection, defaults being loaded on mismatch, sudden ESP reset (by watchdog or exception)
- ...

# Installation
If you intend to use, compile and run this code you need to:
- have a WS2812FX strip on **Pin 3 (RX)** on a ESP8266 (nodemcu / wemos D1 mini)
- set necessary settings: e.g.: https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf/blob/26ceb77ee160dcee6343824c60c1161b5db4dd64/platformio.ini#L25
defining the LED_NAME (the web page and hostname is set to this) and the number of LEDs (LED_COUNT) on the stripe (theoretically limited to 65535 but for performance reasons rather limited to 300 (max fps you can get with 300 LEDs will be around 111) - if you use the KNOB_CONTROL or not...
- copy the personal icons of your led device in the env_data_folders/[environment_name] folder. They will be copied to the data folder during file system build using perl scripts. There is a default folder in case you don't have your own.
- **Attention:** This SW now uses **LittleFS** and **AsyncWebServer**. 
- upload the **LittelFS** data file system image
- upload the compiled code
- get in touch if you need / want some help

I would advice to use **platformio (with VSCode)** as this should work more or less out of the box (i.e. library dependencies etc).

*Remark: The use of AsyncWebServer required some modifications in how parameters are sent to the WebPage. If - after uploading for the first time or a new version - the webpage does not react on commands, please use the "resetDefaults" button.

I personally use platform.io with Visual Studio Code. So I don't know exactly how to do this with the Arduino IDE.

When you first startup and the WiFiManager Library does not find credentials being stored it will open a WiFi access point. Connect to it and navigate to 192.168.4.1. There you should be able to connect to your WiFi. After restrart the ESP connects to your Wifi and if its ready to go all the LEDs will fade to Green and back to black...

If you define the DEBUG build flag (-DDEBUG) it will enable debug code and the WifiManager will also publish the IP-Adress received...

The Web-Page is a mix of German and English (sorry) - I needed to provide something for my kids. But I will change back to complete English (or a language flag) in the future...

in case of questions, comments, issues ... feel free to contact me.
