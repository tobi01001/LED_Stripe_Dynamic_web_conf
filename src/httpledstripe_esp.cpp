/**************************************************************
   This work is based on many others.
   If published then under whatever licence free to be used,
   distibuted, changed and whatsoever.
   This Work is based on many others 
   and heavily modified for my personal use.
   It is basically based on the following great developments:
   - Adafruit Neopixel https://github.com/adafruit/Adafruit_NeoPixel
   - WS2812FX library https://github.com/kitesurfer1404/WS2812FX
   - fhem esp8266 implementation - Idea from https://github.com/sw-home/FHEM-LEDStripe 
   - FastLED library - see http://www.fastLed.io
   - ESPWebserver - see https://github.com/jasoncoon/esp8266-fastled-webserver
  
  My GIT source code storage
  https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf

  Done by tobi01001

  **************************************************************

  MIT License

  Copyright (c) 2018 tobi01001

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.


 **************************************************************/

#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>


#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

/* use build flags to define these */
#ifndef LED_NAME
  #error "You need to give your LED a Name (build flag e.g. '-DLED_NAME=\"My LED\"')!"
#endif


#define BUILD_VERSION ("0.6.1 ") 
#ifndef BUILD_VERSION
  #error "We need a SW Version and Build Version!"
#endif

#ifdef DEBUG
  String build_version = BUILD_VERSION + String("DEBUG ") + String(__TIMESTAMP__);
#else
  String build_version = BUILD_VERSION + String(__TIMESTAMP__);
#endif


//#define DEBUG
#ifndef LED_COUNT
  #error "You need to define the number of Leds by LED_COUNT (build flag e.g. -DLED_COUNT=50)"
#endif

#define LED_PIN 3  // Needs to be 3 (raw value) for ESP8266 because of DMA

#define STRIP_FPS 60          // 60 FPS seems to be a good value
#define STRIP_VOLTAGE 5       // fixed to 5 volts
#define STRIP_MILLIAMPS 2500  // can be changed during runtime

// The delay being used for several init phases.
#ifdef DEBUG
  #define INITDELAY 500
#else
  #define INITDELAY 2
#endif

// new approach starts here:
#include "led_strip.h"


/* Definitions for network usage */
/* maybe move all wifi stuff to separate files.... */
#define WIFI_TIMEOUT 5000
ESP8266WebServer server(80);
WebSocketsServer *webSocketsServer; // webSocketsServer = WebSocketsServer(81);


String AP_SSID = LED_NAME + String(ESP.getChipId());

/* END Network Definitions */

//flag for saving data
bool shouldSaveConfig = false;
bool shouldSaveRuntime = false;

typedef struct strEEPROMSaveData {
    uint16_t CRC = 0;
    WS2812FX::segment seg;
    uint8_t brightness = DEFAULT_BRIGHTNESS;
    mysunriseParam sParam;
    uint8_t currentEffect = FX_NO_FX;
    uint8_t pal_num;
    CRGBPalette16 pal;

    bool stripIsOn = false;
} EEPROMSaveData;

EEPROMSaveData myEEPROMSaveData;

unsigned long last_wifi_check_time = 0;

#include "FSBrowser.h"

// function Definitions
void  saveConfigCallback    (void),
      saveEEPROMData        (void),
      initOverTheAirUpdate  (void),
      setupWiFi             (void),
      handleSet             (void),
      handleNotFound        (void),
      handleGetModes        (void),
      handleStatus          (void),
      factoryReset          (void),
      handleResetRequest    (void),
      setupWebServer        (void),
      showInitColor         (CRGB Color),
      sendInt               (String name, uint16_t value),
      sendString            (String name, String value),
      sendAnswer            (String jsonAnswer),
      broadcastInt          (String name, uint16_t value),
      broadcastString       (String name, String value),
      webSocketEvent        (uint8_t num, WStype_t type, uint8_t * payload, size_t length),
      clearEEPROM           (void);

      
const String 
      pals_setup            (void);



// used to send an answer as INT to the calling http request
// ToDo: Use one answer function with parameters being overloaded
void sendInt(String name, uint16_t value)
{
  String answer = F("{ ");
  answer += F("\"currentState\" : { \"");  
  answer += name;
  answer += F("\": ");
  answer += value;
  answer += " } }";
  #ifdef DEBUG
  Serial.print("Send HTML respone 200, application/json with value: ");
  Serial.println(answer);
  #endif
  server.send(200, "application/json", answer);
}

// used to send an answer as String to the calling http request
// ToDo: Use one answer function with parameters being overloaded
void sendString(String name, String value)
{
  String answer = F("{ ");
  answer += F("\"currentState\" : { \"");  
  answer += name;
  answer += F("\": \"");
  answer += value;
  answer += "\" } }";
  #ifdef DEBUG
  Serial.print("Send HTML respone 200, application/json with value: ");
  Serial.println(answer);
  #endif
  server.send(200, "application/json", answer);
}

// used to send an answer as JSONString to the calling http request
// send answer can embed a complete json string instead of a single name / value pair.
void sendAnswer(String jsonAnswer)
{
  String answer = "{ \"currentState\": { " + jsonAnswer + "} }";
  server.send(200, "application/json", answer);
}


// broadcasts the name and value to all websocket clients
void broadcastInt(String name, uint16_t value)
{
  String json = "{\"name\":\"" + name + "\",\"value\":" + String(value) + "}";
  #ifdef DEBUG
  Serial.print("Send websocket broadcast with value: ");
  Serial.println(json);
  #endif
  webSocketsServer->broadcastTXT(json);
}

// broadcasts the name and value to all websocket clients
// ToDo: One function with parameters being overloaded.
void broadcastString(String name, String value)
{
  String json = "{\"name\":\"" + name + "\",\"value\":\"" + String(value) + "\"}";
  #ifdef DEBUG
  Serial.print("Send websocket broadcast with value: ");
  Serial.println(json);
  #endif
  webSocketsServer->broadcastTXT(json);
}

// calculates a simple CRC over the given buffer and length
unsigned int calc_CRC16(unsigned int crc, unsigned char *buf, int len)
{
	for (int pos = 0; pos < len; pos++)
	{
		crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	return crc;
}

// write runtime data to EEPROM (when required by "shouldSave Runtime")
void saveEEPROMData(void) {
  if(!shouldSaveRuntime) return;
  shouldSaveRuntime = false;
  #ifdef DEBUG
    Serial.println("\nGoing to store runtime on EEPROM...");
  
    Serial.println("\tget segments");
  #endif
  // we will store the complete segment data 
  myEEPROMSaveData.seg = *strip->getSegment();
  #ifdef DEBUG
    Serial.print("\t\tautoPal ");
    Serial.println(myEEPROMSaveData.seg.autoPal);
    Serial.print("\t\tautoPalDuration ");
    Serial.println(myEEPROMSaveData.seg.autoPalDuration);
    Serial.print("\t\tautoplay ");
    Serial.println(myEEPROMSaveData.seg.autoplay);
    Serial.print("\t\t autoplayDuration ");
    Serial.println(myEEPROMSaveData.seg.autoplayDuration);
    Serial.print("\t\t beat88 ");
    Serial.println(myEEPROMSaveData.seg.beat88);
    Serial.print("\t\t blendType ");
    Serial.println(myEEPROMSaveData.seg.blendType);
    Serial.print("\t\t cooling ");
    Serial.println(myEEPROMSaveData.seg.cooling);
    Serial.print("\t\t deltaHue ");
    Serial.println(myEEPROMSaveData.seg.deltaHue);
    Serial.print("\t\t hueTime ");
    Serial.println(myEEPROMSaveData.seg.hueTime);
    Serial.print("\t\t milliamps ");
    Serial.println(myEEPROMSaveData.seg.milliamps);
    Serial.print("\t\t mode ");
    Serial.println(myEEPROMSaveData.seg.mode);
    Serial.print("\t\t reverse ");
    Serial.println(myEEPROMSaveData.seg.reverse);
    Serial.print("\t\t sparking ");
    Serial.println(myEEPROMSaveData.seg.sparking);
  #endif
  myEEPROMSaveData.brightness = strip->getBrightness();
  #ifdef DEBUG
    Serial.print("\tget brightness ");
    Serial.println(myEEPROMSaveData.brightness);
  #endif
  myEEPROMSaveData.sParam = sunriseParam;
  #ifdef DEBUG
    Serial.println("\tget sunriseparam");
  #endif
  myEEPROMSaveData.currentEffect = currentEffect;
  #ifdef DEBUG
    Serial.print("\tget current effect ");
    Serial.println(myEEPROMSaveData.currentEffect);
  #endif
  myEEPROMSaveData.pal_num = strip->getTargetPaletteNumber();
  #ifdef DEBUG
    Serial.print("\tget pal number ");
    Serial.println(myEEPROMSaveData.pal_num);
  #endif
  myEEPROMSaveData.pal = strip->getTargetPalette();
  #ifdef DEBUG
    Serial.println("\tget palette");
  #endif
  myEEPROMSaveData.stripIsOn = stripIsOn;
  #ifdef DEBUG
    Serial.print("\tget stripIsOn ");
    Serial.println(myEEPROMSaveData.stripIsOn);

    Serial.println("\nGoing to calculate the CRC over the data...");
    Serial.print("\tsize of myEEPROMSaveData\t");
    Serial.println(sizeof(myEEPROMSaveData));
    Serial.print("\tsize of myEEPROMSaveData - 2\t");
    Serial.println(sizeof(myEEPROMSaveData)-2);
    Serial.print("\tsize of myEEPROMSaveData.CRC\t");
    Serial.println(sizeof(myEEPROMSaveData.CRC));
  #endif
  
  // once the data is all stroed and/or refreshed in the structure
  // we calculate a CRC tin order to validate the correctness during read.
  // the CRC is the first value in the struct
  myEEPROMSaveData.CRC = (uint16_t)calc_CRC16(0x5a5a, (unsigned char*)&myEEPROMSaveData+2, sizeof(myEEPROMSaveData)-2);

  #ifdef DEBUG
  Serial.printf("\tCRC\t0x%04x\n", myEEPROMSaveData.CRC);
  #endif

  // write the data to the EEPROM
  EEPROM.put(0, myEEPROMSaveData);
  // as we work on ESP, we also need to commit the written data.
  EEPROM.commit();
  #ifdef DEBUG
    Serial.println("EEPROM write finished...");
  #endif
}

// callback notifying us of the need to save config
// we do not save anything in here but only set the flag
// to store on the next loop through
void saveConfigCallback(void) {
  #ifdef DEBUG
    Serial.println("\n\tWe are now invited to save the configuration...");
  #endif // DEBUG
  shouldSaveConfig = true;
}


// reads the stored runtime data from EEPROM
// must be called after everything else is already setup to be working
// otherwise this may terribly fail and could override what was read already
void readRuntimeDataEEPROM(void) {
  #ifdef DEBUG
    Serial.println("\n\tReading Config From EEPROM...");
  #endif
  //read the configuration from EEPROM into RAM
  EEPROM.begin(sizeof(myEEPROMSaveData));

  EEPROM.get(0, myEEPROMSaveData);

  // calculate the CRC over the data being stored in the EEPROM (except the CRC itself)
  uint16_t mCRC =  (uint16_t)calc_CRC16( 0x5a5a, // polynom -> could be changed to a define
                            (unsigned char*)&myEEPROMSaveData+2, // start address = strcut address + 2 (just behind the CRC itslef)
                            sizeof(myEEPROMSaveData)-2);  // length of the buffer (minus CRC itaself)

  // we have a matching CRC, so we update the runtime data.
  if( myEEPROMSaveData.CRC == mCRC) 
  {
    #ifdef DEBUG
    Serial.println("\n\tWe got a CRC match!...");
    #endif
    strip->setBrightness(myEEPROMSaveData.brightness);

    // sanity checks ( defensive, I know... but just in case)
    if(strip->getSegment()->stop != myEEPROMSaveData.seg.stop)
    {
      myEEPROMSaveData.seg.stop = strip->getSegment()->stop;
    }
    if(myEEPROMSaveData.currentEffect > 3) myEEPROMSaveData.currentEffect = 0;
    if(myEEPROMSaveData.seg.fps < 10) myEEPROMSaveData.seg.fps = 60;

    *(strip->getSegment()) = myEEPROMSaveData.seg;


    sunriseParam = myEEPROMSaveData.sParam;
    currentEffect = myEEPROMSaveData.currentEffect;
    if(myEEPROMSaveData.pal_num < strip->getPalCount())
    {
      strip->setTargetPalette(myEEPROMSaveData.pal_num);
    }
    else
    {
      strip->setTargetPalette(myEEPROMSaveData.pal, "Custom");
    }
    stripIsOn = myEEPROMSaveData.stripIsOn;
    stripWasOff = stripIsOn;
    previousEffect = currentEffect;    
  }
  else // load defaults
  {
    #ifdef DEBUG
    Serial.println("\n\tWe got NO NO NO CRC match!...");
    #endif
  }

  #ifdef DEBUG
  Serial.printf("\tCRC stored\t0x%04x\n", myEEPROMSaveData.CRC);
  Serial.printf("\tCRC calculated\t0x%04x\n", mCRC);
    Serial.println("\tRead Segment Data:");
    Serial.print("\t\tautoPal\t\t ");
    Serial.println(myEEPROMSaveData.seg.autoPal);
    Serial.print("\t\tautoPalDuration\t\t ");
    Serial.println(myEEPROMSaveData.seg.autoPalDuration);
    Serial.print("\t\tautoplay\t\t ");
    Serial.println(myEEPROMSaveData.seg.autoplay);
    Serial.print("\t\t autoplayDuration\t\t ");
    Serial.println(myEEPROMSaveData.seg.autoplayDuration);
    Serial.print("\t\t beat88\t\t ");
    Serial.println(myEEPROMSaveData.seg.beat88);
    Serial.print("\t\t blendType\t\t ");
    Serial.println(myEEPROMSaveData.seg.blendType);
    Serial.print("\t\t cooling\t\t ");
    Serial.println(myEEPROMSaveData.seg.cooling);
    Serial.print("\t\t deltaHue\t\t ");
    Serial.println(myEEPROMSaveData.seg.deltaHue);
    Serial.print("\t\t hueTime\t\t ");
    Serial.println(myEEPROMSaveData.seg.hueTime);
    Serial.print("\t\t milliamps\t\t ");
    Serial.println(myEEPROMSaveData.seg.milliamps);
    Serial.print("\t\t mode\t\t ");
    Serial.println(myEEPROMSaveData.seg.mode);
    Serial.print("\t\t reverse\t\t ");
    Serial.println(myEEPROMSaveData.seg.reverse);
    Serial.print("\t\t sparking\t\t ");
    Serial.println(myEEPROMSaveData.seg.sparking);

    Serial.print("\tget brightness\t\t ");
    Serial.println(myEEPROMSaveData.brightness);
    Serial.print("\tget current effect\t\t ");
    Serial.println(myEEPROMSaveData.currentEffect);
    Serial.print("\tget pal number\t\t ");
    Serial.println(myEEPROMSaveData.pal_num);
    Serial.print("\tget stripIsOn\t\t ");
    Serial.println(myEEPROMSaveData.stripIsOn);
  #endif

  // no need to save right now. next save should be after /set?....
  shouldSaveRuntime = false;
}

// we do not want anything to distrub the OTA
// therefore there is a flag which could be used to prevent from that...
// However, seems that a connection being active via websocket interrupts
// even if the web socket server is stopped??
bool OTAisRunning = false;

void initOverTheAirUpdate(void) {
  #ifdef DEBUG
  Serial.println("\nInitializing OTA capabilities....");
  #endif
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // ToDo: Implement Hostname in config and WIFI Settings?

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("esp8266Toby01");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    #ifdef DEBUG
    Serial.println("OTA start");
    #endif
    
    setEffect(FX_NO_FX);
    reset();
    // the following is just to visually indicate the OTA start
    // which is done by blinking the complete stripe in different colors from yellow to green
    uint8_t factor = 85;
    for(uint8_t c = 0; c < 4; c++) {

      for(uint16_t i=0; i<strip->getLength(); i++) {
        uint8_t r = 256 - (c*factor);
        uint8_t g = c > 0 ? (c*factor-1) : (c*factor);
        //strip.setPixelColor(i, r, g, 0);
        strip->leds[i] = CRGB(strip_color32(r,g,0));
      }
      strip->show();
      delay(250);
      for(uint16_t i=0; i<strip->getLength(); i++) {
        strip->leds[i] = CRGB::Black;
      }
      strip->show();
      delay(500);
    }
    // we need to delete the websocket server in order to have OTA correctly running.
    delete webSocketsServer;
    // we stop the webserver to not get interrupted....
    server.stop();
    // we indicate our sktch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });

  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    #ifdef DEBUG
    Serial.println("\nOTA end");
    #endif
    // OTA finished.
    // We fade out the green Leds being activated during OTA.
    for(uint8_t i = strip->leds[0].green; i>0; i--)
    {
      for(uint16_t p=0; p<strip->getLength(); p++)
      {
        strip->leds[p].subtractFromRGB(2);
        //strip.setPixelColor(p, 0, i-1 ,0);
      }
      strip->show();
      delay(2);
    }
    // indicate that OTA is no longer running.
    OTAisRunning = false;
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress on the strips as well to be informed if anything gets stuck...
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    #ifdef DEBUG
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    #endif

    // OTA Update will show increasing green LEDs during progress:
    uint16_t progress_value = progress*100 / (total / strip->getLength());
    uint16_t pixel = (uint16_t) (progress_value / 100);
    uint16_t temp_color = progress_value - (pixel*100);
    if(temp_color > 255) temp_color = 255;

    strip->leds[pixel] = strip_color32(0, (uint8_t)temp_color, 0);
    strip->show();
  });

  // something went wrong, we gonna show an error "message" via LEDs.
  ArduinoOTA.onError([](ota_error_t error) {
    #ifdef DEBUG
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    #endif
    // something went wrong during OTA.
    // We will fade in to red...
    for(uint16_t c = 0; c<256; c++)
    {
      for(uint16_t i = 0; i<strip->getLength(); i++)
      {
        //strip.setPixelColor(i,(uint8_t)c,0,0);
        strip->leds[i] = strip_color32((uint8_t)c,0,0);
      }
      strip->show();
      delay(2);
    }
    // We wait 10 seconds and then reset the ESP...
    delay(10000);
    ESP.reset();
  });
  // start the service
  ArduinoOTA.begin();
  #ifdef DEBUG
  Serial.println("OTA capabilities initialized....");
  #endif
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}


// for DEBUG purpose without Serial connection...
void showInitColor(CRGB Color)
{
  #ifdef DEBUG
  FastLED.showColor(Color);
  #endif
}

// setup the Wifi connection with Wifi Manager...
void setupWiFi(void){

  showInitColor(CRGB::Blue);
  delay(INITDELAY);

  WiFi.hostname(LED_NAME + String(ESP.getChipId()));

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(240);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  #ifdef DEBUG
  Serial.println("Going to autoconnect and/or Start AP");
  #endif
  if (!wifiManager.autoConnect(AP_SSID.c_str())) {
    #ifdef DEBUG
    Serial.println("failed to connect, we should reset as see if it connects");
    #endif
    showInitColor(CRGB::Yellow);
    delay(3000);
    showInitColor(CRGB::Red);
    ESP.reset();
    delay(5000);
  }
  //if we get here we have connected to the WiFi
  #ifdef DEBUG
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());
  #endif

  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
}

// helper function to change an 8bit value by the given percentage
// ToDo: we could use 8bit fractions for performance
uint8_t changebypercentage (uint8_t value, uint8_t percentage) {
  uint16_t ret = max((value*percentage)/100, 10);
  if (ret > 255) ret = 255;
  return (uint8_t) ret;
}

// if /set was called
void handleSet(void) {

  // Debug only
  #ifdef DEBUG
  Serial.println("<Begin>Server Args:");
  for(uint8_t i = 0; i<server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print("\t");
    Serial.println(server.arg(i));
    Serial.print(server.argName(i));
    Serial.print("\t char[0]: ");
    Serial.println(server.arg(i)[0]);
  }
  Serial.println("<End> Server Args");
  #endif
  // to be completed in general
  // ToDo: question: is there enough memory to store color and "timing" per pixel?
  // i.e. uint32_t onColor, OffColor, uint16_t ontime, offtime
  // = 12 * 300 = 3600 byte...???
  // this will be a new branch possibly....

  // The following list is unfortunately not complete....
  // mo = mode set (eihter +, - or value)
  // br = brightness (eihter +, - or value)
  // co = color (32 bit unsigned color)
  // re = red value of color (eihter +, - or value)
  // gr = green value of color (eihter +, - or value)
  // bl = blue value of color (eihter +, - or value)
  // sp = speed (eihter +, - or value)
  // sec = sunrise / sunset time in seconds....
  // min = sunrise/ sunset time in minutes
  // pi = pixel to be set (clears others?)
  // rnS = Range start Pixel;
  // rnE = Range end Pixel;

  // here we set a new mode if we have the argument mode
  if(server.hasArg("mo")) {
    // flag to decide if this is an library effect
    bool isWS2812FX = false;
    // current library effect number
    uint8_t effect = strip->getMode();
    
    #ifdef DEBUG
    Serial.println("got Argument mo....");
    #endif
    
    // just switch to the next if we get an "u" for up
    if (server.arg("mo")[0] == 'u') {
    
      #ifdef DEBUG
      Serial.println("got Argument mode up....");
      #endif
   
      effect = effect + 1;
      isWS2812FX = true;
    }
    // switch to the previous one if we get a "d" for down
    else if (server.arg("mo")[0] == 'd') {
      #ifdef DEBUG
      Serial.println("got Argument mode down....");
      #endif
      effect = effect - 1;
      isWS2812FX = true;
    }
    // if we get an "o" for off, we switch off
    else if (server.arg("mo")[0] == 'o') {
      #ifdef DEBUG
      Serial.println("got Argument mode Off....");
      #endif
      sendString("state", "off");
      //reset();
      strip_On_Off(false);
      //strip->stop();
      broadcastInt("power", stripIsOn);
    }
    // for backward compatibility and FHEM:
    // --> activate fire flicker
    else if (server.arg("mo")[0] == 'f') {
      #ifdef DEBUG
      Serial.println("got Argument fire....");
      #endif
      effect = FX_MODE_FIRE_FLICKER;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate rainbow effect
    else if (server.arg("mo")[0] == 'r') {
      #ifdef DEBUG
      Serial.println("got Argument mode rainbow cycle....");
      #endif
      effect = FX_MODE_RAINBOW_CYCLE;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate the K.I.T.T. (larson scanner)
    else if (server.arg("mo")[0] == 'k') {
      #ifdef DEBUG
      Serial.println("got Argument mode KITT....");
      #endif
      effect = FX_MODE_LARSON_SCANNER;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox
    else if (server.arg("mo")[0] == 's') {
      #ifdef DEBUG
      Serial.println("got Argument mode Twinkle Fox....");
      #endif
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox in white...
    else if (server.arg("mo")[0] == 'w') {
      #ifdef DEBUG
      Serial.println("got Argument mode White Twinkle....");
      #endif
      strip->setColor(CRGBPalette16(CRGB::White));
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // sunrise effect
    else if (server.arg("mo") == "Sunrise") {
      #ifdef DEBUG
      Serial.println("got Argument mode sunrise....");
      #endif
      // milliseconds time to full sunrise
      uint32_t mytime = myEEPROMSaveData.sParam.deltaTime * myEEPROMSaveData.sParam.steps;
      const uint16_t mysteps = 512; // defaults to 512 as color values are 255...
      // sunrise time in seconds
      if(server.hasArg("sec")) {
        #ifdef DEBUG
        Serial.println("got Argument sec....");
        #endif
        mytime = 1000 * (uint32_t)strtoul(&server.arg("sec")[0], NULL, 10);
      }
      // sunrise time in minutes
      else if(server.hasArg("min")) {
        #ifdef DEBUG
        Serial.println("got Argument min....");
        #endif
        mytime = (1000 * 60) * (uint8_t)strtoul(&server.arg("min")[0], NULL, 10);
      }
      // use default if time less than 1000 ms;
      // because smaller values should not be possible.
      if(mytime < 1000) {
        // default will be 10 minutes
        // = (1000 ms * 60) = 1 minute *10 = 10 minutes
        mytime = 1000 * 60 * 10; // for readability
      }
      mySunriseStart(mytime, mysteps, true);
      // answer for the "calling" party
      handleStatus();
    }
    // the same for sunset....
    else if (server.arg("mo") == "Sunset") {
      #ifdef DEBUG
      Serial.println("got Argument mode sunset....");
      #endif
      // milliseconds time to full sunrise
      uint32_t mytime = myEEPROMSaveData.sParam.deltaTime * myEEPROMSaveData.sParam.steps;
      const uint16_t mysteps = 512; // defaults to 1000;
      // sunrise time in seconds
      if(server.hasArg("sec")) {
        #ifdef DEBUG
        Serial.println("got Argument sec....");
        #endif
        mytime = 1000 * (uint32_t)strtoul(&server.arg("sec")[0], NULL, 10);
      }
      // sunrise time in minutes
      else if(server.hasArg("min")) {
        #ifdef DEBUG
        Serial.println("got Argument min....");
        #endif
        mytime = (1000 * 60) * (uint8_t)strtoul(&server.arg("min")[0], NULL, 10);
      }
      // use default if time < 1000 ms;
      if(mytime < 1000) {
        // default will be 10 minutes
        // = (1000 ms * 60) = 1 minute *10 = 10 minutes
        mytime = 1000 * 60 * 10; // for readability
      }
      mySunriseStart(mytime, mysteps, false);
      // answer for the "calling" party
      handleStatus();
    }
    // finally - if nothing matched before - we switch to the effect  being provided.
    // we don't care if its actually an int or not
    // because it will be zero anyway if not.
    else {
      #ifdef DEBUG
      Serial.println("got Argument mode and seems to be an Effect....");
      #endif
      effect = (uint8_t)strtoul(&server.arg("mo")[0], NULL, 10);
      isWS2812FX = true;
    }
    // make sure we roll over at the max number
    if(effect >= strip->getModeCount()) {
      #ifdef DEBUG
      Serial.println("Effect to high....");
      #endif
      effect = 0;
    }
    // activate the effect...
    if(isWS2812FX) {
      setEffect(FX_WS2812);
      strip->setMode(effect);
      // in case it was stopped before
      // seems to be obsolete but does not hurt anyway...
      strip->start();
      #ifdef DEBUG
      Serial.println("gonna send mo response....");
      #endif
      // let the caller know what we just did
      sendAnswer(  "\"mode\": 3, \"modename\": \"" + 
                  (String)strip->getModeName(effect) + 
                  "\", \"wsfxmode\": " + String(effect));
      // let anyone connected know what we just did
      broadcastInt("mo", effect);
      broadcastInt("power", stripIsOn);
    }
  }
  // global on/off
  if(server.hasArg("power"))
  {
    #ifdef DEBUG
      Serial.println("got Argument power....");
      #endif
    if(server.arg("power")[0] == '0')
    {
      strip_On_Off(false);
    }
    else
    {
      strip_On_Off(true);
    }
    if(currentEffect == FX_WS2812)
    {
      strip->start();
      strip->setMode(strip->getMode());
    }

    sendString("state", stripIsOn?"on":"off");
    broadcastInt("power", stripIsOn);
  }
    
  // if we got a palette change
  if(server.hasArg("pa")) {
    // ToDo: Possibility to setColors and new Palettes...
    uint8_t pal = (uint8_t)strtoul(&server.arg("pa")[0], NULL, 10);
    #ifdef DEBUG
    Serial.print("New palette with value: ");
    Serial.println(pal);
    #endif
    strip->setTargetPalette(pal);
    sendAnswer(   "\"palette\": " + String(pal) + ", \"palette name\": \"" + 
                  (String)strip->getPalName(pal) + "\"");
    broadcastInt("pa", pal);
  }

  // if we got a new brightness value
  if(server.hasArg("br")) {
    #ifdef DEBUG
      Serial.println("got Argument brightness....");
      #endif
    uint8_t brightness = strip->getBrightness();
    if (server.arg("br")[0] == 'u') {
    brightness = changebypercentage(brightness, 110);
    } else if (server.arg("br")[0] == 'd') {
      brightness = changebypercentage(brightness, 90);
    } else {
      brightness = constrain((uint8_t)strtoul(&server.arg("br")[0], NULL, 10), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    }
    strip->setBrightness(brightness);
    sendInt("brightness", brightness);
    broadcastInt("br", strip->getBrightness());
  }

  // if we got a speed value
  // for backward compatibility.
  // is beat88 value anyway 
  if(server.hasArg("sp")) {
    #ifdef DEBUG
      Serial.println("got Argument speed....");
      #endif
    uint16_t speed = strip->getBeat88();
    if (server.arg("sp")[0] == 'u') {
      uint16_t ret = max((speed*115)/100, 10);
      if (ret > BEAT88_MAX) ret = BEAT88_MAX;
      speed = ret;
    } else if (server.arg("sp")[0] == 'd') {
      uint16_t ret = max((speed*80)/100, 10);
      if (ret > BEAT88_MAX) ret = BEAT88_MAX;
      speed = ret;
    } else {
      speed = constrain((uint16_t)strtoul(&server.arg("sp")[0], NULL, 10), BEAT88_MIN, BEAT88_MAX);
    }
    strip->setSpeed(speed);
    strip->show();
    sendAnswer( "\"speed\": " + String(speed) + ", \"beat88\": \"" + String(speed));
    broadcastInt("sp", strip->getBeat88());
    strip->setTransition();
  }

  // if we got a speed value (as beat88)
  if(server.hasArg("be")) {
    #ifdef DEBUG
      Serial.println("got Argument speed (beat)....");
      #endif
    uint16_t speed = strip->getBeat88();
    if (server.arg("be")[0] == 'u') {
      uint16_t ret = max((speed*115)/100, 10);
      if (ret > BEAT88_MAX) ret = BEAT88_MAX;
      speed = ret;
    } else if (server.arg("be")[0] == 'd') {
      uint16_t ret = max((speed*80)/100, 10);
      if (ret > BEAT88_MAX) ret = BEAT88_MAX;
      speed = ret;
    } else {
      speed = constrain((uint16_t)strtoul(&server.arg("be")[0], NULL, 10), BEAT88_MIN, BEAT88_MAX);
    }
    strip->setSpeed(speed);
    strip->show();
    sendAnswer( "\"speed\": " + String(speed) + ", \"beat88\": \"" + String(speed));
    broadcastInt("sp", strip->getBeat88());
    strip->setTransition();
  }

  // color handling
  // this is a bit tricky, as it handles either RGB as one or different values.

  // current color (first value from palette)
  uint32_t color = strip->getColor(0);
  bool setColor = false;
  // we got red
  if(server.hasArg("re")) {
    setColor = true;
    #ifdef DEBUG
      Serial.println("got Argument red....");
      #endif
    uint8_t re = Red(color);
    if(server.arg("re")[0] == 'u') {
        re = changebypercentage(re, 110);
    } else if (server.arg("re")[0] == 'd') {
      re = changebypercentage(re, 90);
    } else {
      re = constrain((uint8_t)strtoul(&server.arg("re")[0], NULL, 10), 0, 255);
    }
    color = (color & 0x00ffff) | (re << 16);
  }
  // we got green
  if(server.hasArg("gr")) {
    setColor = true;
    #ifdef DEBUG
      Serial.println("got Argument green....");
      #endif
    uint8_t gr = Green(color);
    if(server.arg("gr")[0] == 'u') {
        gr = changebypercentage(gr, 110);
    } else if (server.arg("gr")[0] == 'd') {
      gr = changebypercentage(gr, 90);
    } else {
      gr = constrain((uint8_t)strtoul(&server.arg("gr")[0], NULL, 10), 0, 255);
    }
    color = (color & 0xff00ff) | (gr << 8);
  }
  // we got blue
  if(server.hasArg("bl")) {
    setColor = true;
    #ifdef DEBUG
      Serial.println("got Argument blue....");
      #endif
    uint8_t bl = Blue(color);
    if(server.arg("bl")[0] == 'u') {
        bl = changebypercentage(bl, 110);
    } else if (server.arg("bl")[0] == 'd') {
      bl = changebypercentage(bl, 90);
    } else {
      bl = constrain((uint8_t)strtoul(&server.arg("bl")[0], NULL, 10), 0, 255);
    }
    color = (color & 0xffff00) | (bl << 0);
  }
  // we got a 32bit color value (24 actually)
  if(server.hasArg("co")) {
    setColor = true;
    #ifdef DEBUG
      Serial.println("got Argument color....");
      #endif
    color = constrain((uint32_t)strtoul(&server.arg("co")[0], NULL, 16), 0, 0xffffff);
  }
  // we got one solid color value as r, g, b
  if(server.hasArg("solidColor"))
  {
    setColor = true;
    #ifdef DEBUG
      Serial.println("got Argument solidColor....");
      #endif
    uint8_t r,g,b;
    r = constrain((uint8_t)strtoul(&server.arg("r")[0], NULL, 10), 0, 255);  
    g = constrain((uint8_t)strtoul(&server.arg("g")[0], NULL, 10), 0, 255);  
    b = constrain((uint8_t)strtoul(&server.arg("b")[0], NULL, 10), 0, 255); 
    color = (r << 16) | (g << 8) | (b << 0);
    // CRGB solidColor(color); // obsolete?
    
    broadcastInt("pa", strip->getPalCount()); // this reflects a "custom palette"
  }
  // a signle pixel...
  if(server.hasArg("pi")) {
    #ifdef DEBUG
      Serial.println("got Argument pixel....");
      #endif
    //setEffect(FX_NO_FX);
    uint16_t pixel = constrain((uint16_t)strtoul(&server.arg("pi")[0], NULL, 10), 0, strip->getLength()-1);
    strip_setpixelcolor(pixel, color);
    handleStatus();  
  // a range of pixels from start rnS to end rnE
  } else if (server.hasArg("rnS") && server.hasArg("rnE")) {
    #ifdef DEBUG
      Serial.println("got Argument range start / range end....");
      #endif
    uint16_t start = constrain((uint16_t)strtoul(&server.arg("rnS")[0], NULL, 10), 0, strip->getLength());
    uint16_t end = constrain((uint16_t)strtoul(&server.arg("rnE")[0], NULL, 10), start, strip->getLength());
    set_Range(start, end, color);
    handleStatus();
  // one color for the complete strip
  } else if (server.hasArg("rgb")) {
    #ifdef DEBUG
      Serial.println("got Argument rgb....");
      #endif
    strip->setColor(color);
    setEffect(FX_WS2812);
    strip->setMode(FX_MODE_STATIC);
    handleStatus();
  // finally set a new color
  } else {
    if(setColor) {
      strip->setColor(color);
      handleStatus();
    }
  }

  // autoplay flag changes
  if(server.hasArg("autoplay"))
  {
    uint16_t value = String(server.arg("autoplay")).toInt();
    strip->getSegment()->autoplay = value;
    sendInt("Autoplay Mode", value);
    broadcastInt("autoplay", value);
  }

  // autoplay duration changes
  if(server.hasArg("autoplayDuration"))
  {
    uint16_t value = String(server.arg("autoplayDuration")).toInt();
    strip->setAutoplayDuration(value);
    sendInt("Autoplay Mode Interval", value);
    broadcastInt("autoplayDuration", value);
  }

  // auto plaette change 
  if(server.hasArg("autopal"))
  {
    uint16_t value = String(server.arg("autopal")).toInt();
    strip->getSegment()->autoPal = value;
    sendInt("Autoplay Palette", value);
    broadcastInt("autopal", value);
  }

  // auto palette change duration changes
  if(server.hasArg("autopalDuration"))
  {
    uint16_t value = String(server.arg("autopalDuration")).toInt();
    strip->setAutoPalDuration(value);
    sendInt("Autoplay Palette Interval", value);
    broadcastInt("autopalDuration", value);
  }

  // time for cycling through the basehue value changes
  if(server.hasArg("huetime"))
  {
    uint16_t value = String(server.arg("huetime")).toInt();
    sendInt("Hue change time", value);
    broadcastInt("huetime", value);
    strip->sethueTime(value);
  }

   #pragma message "We could implement a value to change how a palette is distributed accross the strip"
  
  // the hue offset for a given effect (if - e.g. not spread across the whole strip)
  if(server.hasArg("deltahue"))
  {
    uint16_t value = constrain(String(server.arg("deltahue")).toInt(), 0, 255);
    sendInt("Delta hue per change", value);
    broadcastInt("deltahue", value);
    strip->getSegment()->deltaHue = value;
    strip->setTransition();
  }

  // parameter for teh "fire" - flame cooldown
  if(server.hasArg("cooling"))
  {
    uint16_t value = String(server.arg("cooling")).toInt();
    sendInt("Fire Cooling", value);
    broadcastInt("cooling", value);
    strip->getSegment()->cooling = value;
    strip->setTransition();
  }

  // parameter for the sparking - new flames
  if(server.hasArg("sparking"))
  {
    uint16_t value = String(server.arg("sparking")).toInt();
    sendInt("Fire sparking", value);
    broadcastInt("sparking", value);
    strip->getSegment()->sparking = value;
    strip->setTransition();
  }

  // parameter for twinkle fox (speed)
  if(server.hasArg("twinkleSpeed"))
  {
    uint16_t value = String(server.arg("twinkleSpeed")).toInt();
    sendInt("Twinkle Speed", value);
    broadcastInt("twinkleSpeed", value);
    strip->getSegment()->twinkleSpeed = value;
    strip->setTransition();
  }

   // parameter for twinkle fox (density)
  if(server.hasArg("twinkleDensity"))
  {
    uint16_t value = String(server.arg("twinkleDensity")).toInt();
    sendInt("Twinkle Density", value);
    broadcastInt("twinkleDensity", value);
    strip->getSegment()->cooling = value;
    strip->setTransition();
  }

  // parameter for number of bars (beat sine glows etc...)
  if(server.hasArg("numBars"))
  {
    uint16_t value = String(server.arg("numBars")).toInt();
    sendInt("Number of Bars", value);
    broadcastInt("numBars", value);
    strip->setNumBars(value);
    strip->setTransition();
  }

  // parameter to change the palette blend type for cetain effects
  if(server.hasArg("blendType"))
  {
    uint16_t value = String(server.arg("blendType")).toInt();
    
    broadcastInt("blendType", value);
    if(value) {
      strip->getSegment()->blendType = LINEARBLEND;
      sendString("BlendType", "LINEARBLEND");
    }
    else {
      strip->getSegment()->blendType = NOBLEND;
      sendString("BlendType", "NOBLEND");
    }
    strip->setTransition();
  }

  // parameter to change the Color Temperature of the Strip
  if(server.hasArg("ColorTemperature"))
  {
    uint8_t value = String(server.arg("ColorTemperature")).toInt();
    
    broadcastInt("ColorTemperature", value);
    sendString("ColorTemperature", strip->getColorTempName(value));
    strip->setColorTemperature(value);
    strip->setTransition();
  }

  // parameter to change direction of certain effects..
  if(server.hasArg("reverse"))
  {
    uint16_t value = String(server.arg("reverse")).toInt();
    sendInt("reverse", value);
    broadcastInt("reverse", value);
    strip->getSegment()->reverse = value;
    strip->setTransition();
  }

  // parameter to invert colors of all effects..
  if(server.hasArg("inverse"))
  {
    uint16_t value = String(server.arg("inverse")).toInt();
    sendInt("inverse", value);
    broadcastInt("inverse", value);
    strip->setInverse(value);
    strip->setTransition();
  }

  // parameter to divide LEDS into two equal halfs...
  if(server.hasArg("mirror"))
  {
    uint16_t value = String(server.arg("mirror")).toInt();
    sendInt("mirror", value);
    broadcastInt("mirror", value);
    strip->setMirror(value);
    strip->setTransition();
  }

  // parameter so set the max current the leds will draw
  if(server.hasArg("current"))
  {
    uint16_t value = String(server.arg("current")).toInt();
    sendInt("Lamp Max Current", value);
    broadcastInt("current", value);
    strip->setMilliamps(value);
  }

  // parameter for the blur against the previous LED values
  if(server.hasArg("LEDblur"))
  {
    uint8_t value = String(server.arg("LEDblur")).toInt();
    sendInt("LEDblur", value);
    broadcastInt("LEDblur", value);
    strip->setBlurValue(value);
    strip->setTransition();
  }

  // parameter for the frames per second (FPS)
  if(server.hasArg("fps"))
  {
    uint8_t value = String(server.arg("fps")).toInt();
    sendInt("fps", value);
    broadcastInt("fps", value);
    strip->setMaxFPS(value);
    strip->setTransition();
  }

  // new parameters, it's time to save
  shouldSaveRuntime = true;
  /// strip->setTransition();  <-- this is not wise as it removes the smooth fading for colors. So we need to set it case by case
}

// if something unknown was called...
void handleNotFound(void){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleGetModes(void){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(56) + 1070;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();

  JsonObject& modeinfo = root.createNestedObject("modeinfo");
  modeinfo["count"] = strip->getModeCount();

  JsonObject& modeinfo_modes = modeinfo.createNestedObject("modes");
  for(uint8_t i=0; i<strip->getModeCount(); i++) {
      modeinfo_modes[strip->getModeName(i)] = i;
  }

  #ifdef DEBUG
  root.printTo(Serial);
  #endif

  String message = "";
  root.prettyPrintTo(message);
  server.send(200, "application/json", message);
}

void handleGetPals(void){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(56) + 1070;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();

  JsonObject& modeinfo = root.createNestedObject("palinfo");
  modeinfo["count"] = strip->getPalCount();

  JsonObject& modeinfo_modes = modeinfo.createNestedObject("pals");
  for(uint8_t i=0; i<strip->getPalCount(); i++) {
      modeinfo_modes[strip->getPalName(i)] = i;
  }

  #ifdef DEBUG
  root.printTo(Serial);
  #endif

  String message = "";
  root.prettyPrintTo(message);
  server.send(200, "application/json", message);
}

void handleStatus(void){
  uint32_t answer_time = micros();

  String message;
  message.reserve(1500);
  uint16_t num_leds_on = 0;
  // if brightness = 0, no LED can be lid.
  if(strip->getBrightness()) {
    // count the number of active LEDs
    // in rare occassions, this can still be 0, depending on the effect.
    for(uint16_t i=0; i<strip->getLength(); i++) {
      if(strip->leds[i]) num_leds_on++;
    }
  }

  message += F("{\n  \"currentState\": {\n    \"state\": ");
  if(stripIsOn) {
    message += F("\"on\"");
  } else {
    message += F("\"off\"");
  }
  message += F(",\n    \"Buildversion\": \"");
  message += build_version; //String(BUILD_VERSION);
  message += F("\",\n    \"Lampenname\": \"");
  message += String(LED_NAME);
  message += F("\",\n    \"Anzahl Leds\": ");
  message += String(strip->getLength());
  message += F(",\n    \"Lamp Voltage\": ");
  message += String(strip->getVoltage());
  message += F(",\n    \"Lamp Max Current\": ");
  message += String(strip->getMilliamps());
  message += F(",\n    \"Lamp Max Power (mW)\": ");
  message += String(strip->getVoltage() * strip->getMilliamps());
  message += F(",\n    \"Lamp current Power\": ");
  message += String(strip->getCurrentPower());
  message += F(",\n    \"Leds an\": ");
  message += String(num_leds_on) ;
  message += F(",\n    \"mode\": "); 
  message += String(currentEffect); 
  message += F(",\n    \"modename\": ");
  switch (currentEffect) {
    case FX_NO_FX :
      message += F("\"No FX");
      break;
    case FX_SUNRISE :
      message += F("\"Sunrise Effect");
      break;
    case FX_SUNSET :
      message += F("\"Sunset Effect");
      break;
    case FX_WS2812 :
      message += F("\"WS2812fx "); 
      message += String(strip->getModeName(strip->getMode()));
      break;
    default :
      message += F("\"UNKNOWN");
      break;
  }
  message += F("\", \n    \"wsfxmode\": "); 
  message += String(strip->getMode());
  message += F(", \n    \"beat88\": ");
  message += String(strip->getBeat88());
  message += F(", \n    \"speed\": ");
  message += String(strip->getBeat88());
  message += F(", \n    \"brightness\": ");
  message += String(strip->getBrightness());
  
  // Palettes and Colors
  message += F(", \n    \"palette count\": ");
  message += String(strip->getPalCount()); 
  message += F(", \n    \"palette\": ");
  message += String(strip->getTargetPaletteNumber()); 
  message += F(", \n    \"palette name\": \"");
  message += String(strip->getTargetPaletteName());
  message += F("\"");

  CRGB col = CRGB::Black;
  // We return either black (strip effectively off)
  // or the color of the first pixel....
  for(uint16_t i = 0; i<strip->getLength(); i++)
  {
    if(strip->leds[i])
    {
      col = strip->leds[i];
      break;
    }
  }
  message += F(", \n    \"rgb\": ");
  message += String( ((col.r << 16) | 
                      (col.g <<  8) | 
                      (col.b <<  0)) & 0xffffff );
  message += F(", \n    \"color red\": ");
  message += String(col.red);
  message += F(", \n    \"color green\": ");
  message += String(col.green);
  message += F(", \n    \"color blue\": ");
  message += String(col.blue);

  message += F(", \n    \"BlendType\": ");
  if(strip->getSegment()->blendType == NOBLEND)
  {
    message += "\"No Blend\"";
  }
  else if (strip->getSegment()->blendType == LINEARBLEND)
  {
    message += "\"Linear Blend\"";
  }
  else
  {
    message += "\"Unknown Blend\"";
  }
  
  message += F(", \n    \"Reverse\": ");
  message += getReverse();

  message += F(", \n    \"Hue change time\": ");
  message += getHueTime();
  message += F(", \n    \"Delta hue per change\": ");
  message += getDeltaHue();

  message += F(", \n    \"Autoplay Mode\": ");
  message += getAutoplay();
  message += F(", \n    \"Autoplay Mode Interval\": ");
  message += getAutoplayDuration();

  message += F(", \n    \"Autoplay Palette\": ");
  message += getAutopal();

  message += F(", \n    \"Autoplay Palette Interval\": ");
  message += getAutopalDuration();

  message += F(", \n    \"Fire Cooling\": ");
  message += getCooling();

  message += F(", \n    \"Fire sparking\": ");
  message += getSparking();

  message += F(", \n    \"Twinkle Speed\": ");
  message += getTwinkleSpeed();

  message += F(", \n    \"Twinkle Density\": ");
  message += getTwinkleDensity();

  message += F("\n  },\n  \"sunRiseState\": {\n    \"sunRiseMode\": ");


  if(sunriseParam.isSunrise) {
    message += F("\"Sunrise\"");
  } else {
    message += F("\"Sunset\"");
  }
  message += F(",\n    \"sunRiseActive\": ");
  if(sunriseParam.isRunning) {
    message += F("\"on\"");
    message += F(", \n    \"sunRiseCurrStep\": ");
    message += String(sunriseParam.step);
    message += F(", \n    \"sunRiseTotalSteps\": ");
    message += String(sunriseParam.steps);
    if(sunriseParam.isSunrise) {
      message += F(", \n    \"sunRiseTimeToFinish\": ");
      message += String(((sunriseParam.steps - sunriseParam.step) * sunriseParam.deltaTime)/1000);
    } else {
      message += F(", \n    \"sunRiseTimeToFinish\": ");
      message += String(((sunriseParam.step) * sunriseParam.deltaTime)/1000);
    }
  } else {
    message += F("\"off\", \n    \"sunRiseCurrStep\": 0, \n    \"sunRiseTotalSteps\": 0, \n    \"sunRiseTimeToFinish\": 0"); 
  }

  message += F("\n  }");

  #ifdef DEBUG
  message += F(",\n  \"ESP_Data\": {\n    \"DBG_Debug code\": \"On\",\n    \"DBG_CPU_Freq\": ");
  message += String(ESP.getCpuFreqMHz());
  message += F(",\n    \"DBG_Flash Real Size\": ");
  message += String(ESP.getFlashChipRealSize());
  message += F(",\n    \"DBG_Free RAM\": ");
  message += String(ESP.getFreeHeap());
  message += F(",\n    \"DBG_Free Sketch Space\": ");
  message += String(ESP.getFreeSketchSpace());
  message += F(",\n    \"DBG_Sketch Size\": ");
  message += String(ESP.getSketchSize());
  message += F("\n  }");
  
  message += F(",\n  \"Server_Args\": {");
  for(uint8_t i = 0; i<server.args(); i++) {
    message += F("\n    \"");
    message += server.argName(i);
    message += F("\": \"");
    message += server.arg(i);
    if( i < server.args()-1)
      message += F("\",");
    else
      message += F("\"");
  }
  message += F("\n  }");
  #endif
  message += F(",\n  \"Stats\": {\n    \"Answer_Time\": ");
  answer_time = micros() - answer_time;
  message += answer_time;
  message += F(",\n    \"FPS\": ");
  message += FastLED.getFPS();
  message += F("\n  }");
  message += F("\n}");

  #ifdef DEBUG
  Serial.println(message);
  #endif
  
  server.send(200, "application/json", message);
}


void factoryReset(void){
  #ifdef DEBUG
  Serial.println("Someone requested Factory Reset");
  #endif
  // on factory reset, each led will be red
  // increasing from led 0 to max.
  for(uint16_t i = 0; i<strip->getLength(); i++) {
    strip->leds[i] = 0xa00000;
    strip->show();
    delay(2);
  }
  strip->show();
  delay(INITDELAY);
  /*#ifdef DEBUG
  Serial.println("Reset WiFi Settings");
  #endif
  wifiManager.resetSettings();
  delay(INITDELAY);
  */
  clearEEPROM();
  //reset and try again
  #ifdef DEBUG
  Serial.println("Reset ESP and start all over...");
  #endif
  delay(3000);
  ESP.reset();
}

void clearEEPROM(void) {
  //Clearing EEPROM
  #ifdef DEBUG
  Serial.println("Clearing EEPROM");
  #endif
  EEPROM.begin(sizeof(myEEPROMSaveData)+10);
  for(uint i = 0; i< EEPROM.length(); i++)
  {
    EEPROM.write(i,0);
  }
  EEPROM.commit();
  EEPROM.end();
}

// Received Factoryreset request.
// To be sure we check the related parameter....
void handleResetRequest(void){
  if(server.arg("rst") == "FactoryReset") {
    server.send(200, "text/plain", "Will now Reset to factory settings. You need to connect to the WLAN AP afterwards....");
    factoryReset();
  } else if(server.arg("rst") == "Defaults") {
    //strip->setSegment(0, 0, strip->getLength()-1, FX_MODE_STATIC, colors, DEFAULT_BEAT88, false);
    strip->setTargetPalette(0);
    strip->setMode(0);
    setEffect(FX_NO_FX);
    strip->stop();
    strip_On_Off(false);
    server.send(200, "text/plain", "Strip was reset to the default values...");
    shouldSaveRuntime = true;
  }
}

void setupWebServer(void){
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  SPIFFS.begin();
  {
    #ifdef DEBUG
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
     
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    
    Serial.printf("\n");
    #endif
  }

  server.on("/all", HTTP_GET, []() {
    #ifdef DEBUG
    Serial.println("Called /all!");
    #endif
    String json = getFieldsJson(fields, fieldCount);
    server.send(200, "text/json", json);
  });

  server.on("/fieldValue", HTTP_GET, []() {
    String name = server.arg("name");
    #ifdef DEBUG
    Serial.println("Called /fieldValue with arg name =");
    Serial.println(name);
    #endif
   
    String value = getFieldValue(name, fields, fieldCount);
    server.send(200, "text/json", value);
  });

  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });

  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  
  server.on("/set", handleSet);
  server.on("/getmodes", handleGetModes);
  server.on("/getpals", handleGetPals);
  server.on("/status", handleStatus);
  server.on("/reset", handleResetRequest);
  server.onNotFound(handleNotFound);
  
  server.serveStatic("/", SPIFFS, "/", "max-age=86400");
  delay(INITDELAY);
  server.begin();

  showInitColor(CRGB::Yellow);
  delay(INITDELAY);
  #ifdef DEBUG
  Serial.println("HTTP server started.\n");
  #endif
  webSocketsServer = new WebSocketsServer(81);
  webSocketsServer->begin();
  webSocketsServer->onEvent(webSocketEvent);

  showInitColor(CRGB::Green);
  delay(INITDELAY);
  #ifdef DEBUG
  Serial.println("webSocketServer started.\n");
  #endif
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  #ifdef DEBUG
  switch (type) {
    case WStype_DISCONNECTED:
      
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocketsServer->remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        // webSocketsServer.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      // send message to client
      // webSocketsServer.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocketsServer.broadcastTXT("message here");
      break;

    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);

      break;
    
    default:
    break;
  }
  #endif
}

// setup network and output pins
void setup() {
  // Sanity delay to get everything settled....
  delay(500);

  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("\n\n\n");
  Serial.println(F("Booting"));
  #endif

  stripe_setup( LED_COUNT, 
                STRIP_FPS, 
                STRIP_VOLTAGE, 
                STRIP_MILLIAMPS, 
                RainbowColors_p, 
                F("Rainbow Colors"), 
                UncorrectedColor);//TypicalLEDStrip);

  setupWiFi();

  setupWebServer();

  initOverTheAirUpdate();

  // if we got that far, we show by a nice little animation
  // as setup finished signal....
  #ifdef DEBUG
  Serial.println("Init done - fading green in..");
  Serial.print("\tcurrent Effect = ");
  Serial.println(currentEffect);
  #endif
  for(uint8_t a = 0; a < 1; a++) {
    for(uint16_t c = 0; c<256; c+=3) {
      for(uint16_t i = 0; i<strip->getLength(); i++) {
        strip->leds[i].green = c;
      }
      strip->show();
      delay(1);
    }
    #ifdef DEBUG
  Serial.println("Init done - fading green out..");
  Serial.print("\tcurrent Effect = ");
  Serial.println(currentEffect);
  #endif
    delay(2);
    for(uint8_t c = 255; c>0; c-=3) {
      for(uint16_t i = 0; i<strip->getLength(); i++) {
        strip->leds[i].subtractFromRGB(4);
      }
      strip->show();
      delay(1);
    }
  }
  //strip->stop();
  delay(INITDELAY);
  #ifdef DEBUG
  Serial.println("Init finished.. Read runtime data");
  #endif
  readRuntimeDataEEPROM();
  #ifdef DEBUG
  Serial.println("Runtime Data loaded");
  FastLED.countFPS(60);
  #endif
  //setEffect(FX_NO_FX);
}

// request receive loop
void loop() {
  unsigned long now = millis();
  #ifdef DEBUG
  static unsigned long last_status_msg = 0;
  #endif
  if (OTAisRunning) return;
  // if someone requests a call to the factory reset...
  //static bool ResetRequested = false;

  #ifdef DEBUG
    // Debug Watchdog. to be removed for "production".
  if(now - last_status_msg > 10000) {
    last_status_msg = now;
    Serial.print("\n\t");
     switch (currentEffect) {
      case FX_NO_FX :
        Serial.println("No FX");
        break;
      case FX_SUNRISE :
        Serial.println("Sunrise");
        break;
      case FX_SUNSET :
        Serial.println("Sunset");
        break;
      case FX_WS2812 :
        Serial.print("WS2812FX");
        Serial.print("\t");
        Serial.print(strip->getMode());
        Serial.print("\t");
        Serial.println(strip->getModeName(strip->getMode()));
        break;
      default:
        Serial.println("This is a problem!!!");
    }
    Serial.print("\tC:\t");
    Serial.print(strip->getCurrentPaletteName());
    Serial.print("\tT:\t");
    Serial.println(strip->getTargetPaletteName());
    Serial.print("\tFPS:\t");
    Serial.println(FastLED.getFPS());
  }
  #endif
  // Checking WiFi state every WIFI_TIMEOUT
  // Reset on disconnection
  if(now - last_wifi_check_time > WIFI_TIMEOUT) {
    //Serial.print("\nChecking WiFi... ");
    if(WiFi.status() != WL_CONNECTED) {
      #ifdef DEBUG
      Serial.println("WiFi connection lost. Reconnecting...");
      Serial.println("Lost Wifi Connection....");
      #endif
      // Show the WiFi loss with yellow LEDs.
      // Whole strip lid finally.
      for(uint16_t i = 0; i<strip->getLength(); i++)
      {
        strip->leds[i] = 0xa0a000;
        strip->show();
      }
      // Reset after 6 seconds....
      delay(3000);
      #ifdef DEBUG
      Serial.println("Resetting ESP....");
      #endif
      delay(3000);
      ESP.reset();
    }
    last_wifi_check_time = now;
  }

  ArduinoOTA.handle(); // check and handle OTA updates of the code....

  
  webSocketsServer->loop();
  
  server.handleClient();
  
  effectHandler();

  if(shouldSaveRuntime) {
    saveEEPROMData();
    shouldSaveRuntime = false;
  }

}
