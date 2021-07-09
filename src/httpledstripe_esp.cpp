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
#ifndef DEBUG
# define FASTLED_INTERNAL
#endif
#include <LittleFS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

#ifdef HAS_KNOB_CONTROL
  
  #include "Encoder.h"
  #include "SSD1306Brzo.h"
  bool WiFiConnected = false;
#endif

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

#include "defaults.h"

extern "C"
{
#include "user_interface.h"
}

// new approach starts here:
#include "led_strip.h"

#ifdef HAS_KNOB_CONTROL
  Encoder myEnc(KNOB_C_PNA, KNOB_C_PNB);
  SSD1306Brzo display(KNOB_C_I2C, KNOB_C_SDA, KNOB_C_SCL);
#endif

#ifdef DEBUG
  #ifdef HAS_KNOB_CONTROL
    const char * build_version PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH "_KNOB"; 
  #else
    const char * build_version PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH;
  #endif
#else
  #ifdef HAS_KNOB_CONTROL
    const char * build_version PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH "_KNOB"; 
  #else
    const char * build_version PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH;
  #endif
#endif

const char * git_revision PROGMEM = BUILD_GITREV;

/* Definitions for network usage */
/* maybe move all wifi stuff to separate files.... */

AsyncWebServer server(80);

const char * AP_SSID PROGMEM = "Test"; // String(String (LED_NAME) + String("-") + String(ESP.getChipId())).c_str();

//WebSocketsServer *webSocketsServer = NULL; // webSocketsServer = WebSocketsServer(81);
AsyncWebSocket *webSocketsServer;


/* END Network Definitions */

#include "FSBrowser.h"

// helpers
uint8_t wifi_err_counter = 0;
uint16_t wifi_disconnect_counter = 0;

//flag for saving data
bool shouldSaveRuntime = false;

WS2812FX::segment seg;

uint32_t last_control_operation = 0;
bool display_was_off = false;
uint8_t TimeoutBar = 0;

// function Definitions
void 
    saveEEPROMData        (void),
    initOverTheAirUpdate  (void),
    setupWiFi             (uint16_t timeout),
    handleSet             (AsyncWebServerRequest *request),
    handleNotFound        (AsyncWebServerRequest *request),
    handleGetModes        (AsyncWebServerRequest *request),
    handleStatus          (AsyncWebServerRequest *request),
    clearCRC              (void),
    clearEEPROM           (void),
    factoryReset          (void),
    handleResetRequest    (AsyncWebServerRequest *request),
    setupWebServer        (void),
    showInitColor         (CRGB Color),
    //sendAnswer            (const char * name, uint16_t value, AsyncWebServerRequest *request),
    //sendAnswer            (const char * name, const char * value, AsyncWebServerRequest *request),
    broadcastInt          (const __FlashStringHelper* name, uint16_t value),
    webSocketEvent        (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len),
    checkSegmentChanges   (void),
    updateConfigFile      (void),
    clearEEPROM           (void);

uint8_t
    getResetReason        (void);

/*
// used to send an answer as INT to the calling http request
// TODO: Use one answer function with parameters being overloaded
void sendAnswer(const char* name, uint16_t value, AsyncWebServerRequest *request)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled() || !WiFiConnected) return;
  #endif
  
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jBuf; 
  JsonObject &root = jBuf.createObject();
  JsonObject &sInt = root.createNestedObject(F("returnState"));
  sInt[name] = value;

  response->addHeader(F("Access-Control-Allow-Methods"), "*");
  response->addHeader(F("Access-Control-Allow-Headers"), "*");
  response->addHeader(F("Access-Control-Allow-Origin"), "*");
  root.printTo(*response);
  request->send(response);
}

// used to send an answer as String to the calling http request
// TODO: Use one answer function with parameters being overloaded
void sendAnswer(const char* name, const char* value, AsyncWebServerRequest *request)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled() || !WiFiConnected) return;
  #endif
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jBuf; 
  JsonObject &root = jBuf.createObject();
  JsonObject &sInt = root.createNestedObject(F("returnState"));
  sInt[name] = value;

  response->addHeader(F("Access-Control-Allow-Methods"), "*");
  response->addHeader(F("Access-Control-Allow-Headers"), "*");
  response->addHeader(F("Access-Control-Allow-Origin"), "*");
  root.printTo(*response);
  request->send(response);
}
*/

// broadcasts the name and value to all websocket clients
void broadcastInt(const __FlashStringHelper* name, uint16_t value)
{
  
  #ifdef HAS_KNOB_CONTROL
  if(webSocketsServer == NULL  || strip->getWiFiDisabled() || !WiFiConnected)
  #else
  if(webSocketsServer == NULL)
  #endif
  {
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& answerObj = jsonBuffer.createObject();
  //JsonObject& answer = answerObj.createNestedObject(F("currentState"));
  answerObj[F("name")] = name;
  answerObj[F("value")] = value;

  size_t len = answerObj.measureLength();
  AsyncWebSocketMessageBuffer * buffer = webSocketsServer->makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
      answerObj.printTo((char *)buffer->get(), len + 1);
      webSocketsServer->textAll(buffer);
  }
}

void checkSegmentChanges(void) {

  #ifdef DEBUG
  bool save = shouldSaveRuntime;
  #endif

  if(seg.power != strip->getPower()) {
    seg.power = strip->getPower();
    broadcastInt(F("power"), seg.power);
    shouldSaveRuntime = true;
  }
  if(seg.isRunning != strip->isRunning()) {
    seg.isRunning = strip->isRunning();
    broadcastInt(F("isRunning"), seg.isRunning);
    shouldSaveRuntime = true;
  }
  if(seg.targetBrightness != strip->getTargetBrightness()) {
    seg.targetBrightness = strip->getTargetBrightness();
    broadcastInt(F("br"), seg.targetBrightness);
    shouldSaveRuntime = true;
  }
  if(seg.mode != strip->getMode()){
    seg.mode = strip->getMode();
    broadcastInt(F("mo"), seg.mode);
    shouldSaveRuntime = true;
  }
  if(seg.targetPaletteNum != strip->getTargetPaletteNumber()) {
    seg.targetPaletteNum = strip->getTargetPaletteNumber();
    broadcastInt(F("pa"), seg.targetPaletteNum);
    shouldSaveRuntime = true;
  }
  if(seg.beat88 != strip->getBeat88()) {
    seg.beat88 = strip->getBeat88();
    broadcastInt(F("sp"), seg.beat88);
    shouldSaveRuntime = true;
  }
  if(seg.blendType != strip->getBlendType()) {
    seg.blendType = strip->getBlendType();
    broadcastInt(F("blendType"), seg.blendType);
    shouldSaveRuntime = true;
  }
  if(seg.colorTemp != strip->getColorTemperature())
  {
    seg.colorTemp = strip->getColorTemperature();
    broadcastInt(F("ColorTemperature"), strip->getColorTemp());
    shouldSaveRuntime = true;
  }
  if(seg.blur != strip->getBlurValue())
  {
    seg.blur = strip->getBlurValue();
    broadcastInt(F("LEDblur"), seg.blur);
    shouldSaveRuntime = true;
  }
  if(seg.reverse != strip->getReverse())
  {
    seg.reverse = strip->getReverse();
    broadcastInt(F("reverse"), seg.reverse);
    shouldSaveRuntime = true;
  }
  if(seg.segments != strip->getSegments())
  {
    seg.segments = strip->getSegments();
    broadcastInt(F("segments"), seg.segments);
    shouldSaveRuntime = true;
  }
  if(seg.mirror != strip->getMirror())
  {
    seg.mirror = strip->getMirror();
    broadcastInt(F("mirror"), seg.mirror);
    shouldSaveRuntime = true;
  }
  if(seg.inverse != strip->getInverse())
  {
    seg.inverse = strip->getInverse();
    broadcastInt(F("inverse"),seg.inverse);
    shouldSaveRuntime = true;
  }
  if(seg.hueTime != strip->getHueTime())
  {
    seg.hueTime = strip->getHueTime();
    broadcastInt(F("huetime"), seg.hueTime);
    shouldSaveRuntime = true;
  }
  if(seg.deltaHue != strip->getDeltaHue())
  { 
    seg.deltaHue = strip->getDeltaHue();
    broadcastInt(F("deltahue"), seg.deltaHue);
    shouldSaveRuntime = true;
  }
  if(seg.autoplay != strip->getAutoplay())
  {
    seg.autoplay = strip->getAutoplay();
    broadcastInt(F("autoplay"), seg.autoplay);
    shouldSaveRuntime = true;
  }
  if(seg.autoplayDuration != strip->getAutoplayDuration())
  {
    seg.autoplayDuration = strip->getAutoplayDuration();
    broadcastInt(F("autoplayDuration"), seg.autoplayDuration);
    shouldSaveRuntime = true;
  }
  if(seg.autoPal != strip->getAutopal())
  {
    seg.autoPal = strip->getAutopal();
    broadcastInt(F("autopal"), seg.autoPal);
    shouldSaveRuntime = true;
  }
  if(seg.autoPalDuration != strip->getAutopalDuration())
  {
    seg.autoPalDuration = strip->getAutopalDuration();
    broadcastInt(F("autopalDuration"), seg.autoPalDuration);
    shouldSaveRuntime = true;
  }
  /*
  "solidColor" -> currently not possible to save?
  --> for this to work we need to add 32bit color to the structure 
      and to "set" this including palette creation...
      ...and the palette creation depends on the current palette number?
  */
  if(seg.cooling != strip->getCooling())
  {
    seg.cooling = strip->getCooling();
    broadcastInt(F("cooling"), seg.cooling);
    shouldSaveRuntime = true;
  }
  if(seg.sparking != strip->getSparking())
  {
    seg.sparking = strip->getSparking();
    broadcastInt(F("sparking"), seg.sparking);
    shouldSaveRuntime = true;
  }
  if(seg.twinkleSpeed != strip->getTwinkleSpeed())
  {
    seg.twinkleSpeed = strip->getTwinkleSpeed();
    broadcastInt(F("twinkleSpeed"), seg.twinkleSpeed);
    shouldSaveRuntime = true;
  }
  if(seg.twinkleDensity != strip->getTwinkleDensity())
  {
    seg.twinkleDensity = strip->getTwinkleDensity();
    broadcastInt(F("twinkleDensity"), seg.twinkleDensity);
    shouldSaveRuntime = true;
  }
  if(seg.numBars != strip->getNumBars())
  {
    seg.numBars = strip->getNumBars();
    broadcastInt(F("numBars"), seg.numBars);
    shouldSaveRuntime = true;
  }
  if(seg.damping != strip->getDamping())
  {
    seg.damping = strip->getDamping();
    broadcastInt(F("damping"), seg.damping);
    shouldSaveRuntime = true;
  }
  if(seg.sunrisetime != strip->getSunriseTime())
  {
    seg.sunrisetime = strip->getSunriseTime();
    broadcastInt(F("sunriseset"), seg.sunrisetime);
    shouldSaveRuntime = true;
  }
  if(seg.milliamps != strip->getMilliamps())
  {
    seg.milliamps = strip->getMilliamps();
    broadcastInt(F("current"), seg.milliamps);
    shouldSaveRuntime = true;
  }
  if(seg.fps != strip->getMaxFPS())
  {
    seg.fps = strip->getMaxFPS();
    broadcastInt(F("fps"), seg.fps);
    shouldSaveRuntime = true;
  }
  if(seg.dithering != strip->getDithering())
  {
    seg.dithering = strip->getDithering();
    broadcastInt(F("dithering"), seg.dithering);
    shouldSaveRuntime = true;
  }
  if(seg.addGlitter != strip->getAddGlitter())
  {
    seg.addGlitter= strip->getAddGlitter();
    broadcastInt(F("addGlitter"), seg.addGlitter);
    shouldSaveRuntime = true;
  }
  if(seg.whiteGlitter != strip->getWhiteGlitter())
  {
    seg.whiteGlitter= strip->getWhiteGlitter();
    broadcastInt(F("WhiteOnly"), seg.whiteGlitter);
    shouldSaveRuntime = true;
  }
  if(seg.onBlackOnly != strip->getOnBlackOnly())
  {
    seg.onBlackOnly = strip->getOnBlackOnly();
    broadcastInt(F("onBlackOnly"), seg.onBlackOnly);
    shouldSaveRuntime = true;
  }
  if(seg.synchronous != strip->getSynchronous())
  {
    seg.synchronous = strip->getSynchronous();
    broadcastInt(F("syncGlitter"), seg.synchronous);
    shouldSaveRuntime = true;
  }
  if(seg.backgroundHue != strip->getBckndHue())
  {
    seg.backgroundHue = strip->getBckndHue();
    broadcastInt(F("BckndHue"), seg.backgroundHue);
    shouldSaveRuntime = true;
  }
  if(seg.backgroundSat != strip->getBckndSat())
  {
    seg.backgroundSat = strip->getBckndSat();
    broadcastInt(F("BckndSat"), seg.backgroundSat);
    shouldSaveRuntime = true;
  }
  if(seg.backgroundBri != strip->getBckndBri())
  {
    seg.backgroundBri = strip->getBckndBri();
    broadcastInt(F("BckndBri"), seg.backgroundBri);
    shouldSaveRuntime = true;
  }

  #ifdef HAS_KNOB_CONTROL
  if(seg.wifiDisabled != strip->getWiFiDisabled())
  {
    seg.wifiDisabled = strip->getWiFiDisabled();
    broadcastInt(F("wifiDisabled"), seg.wifiDisabled);
    shouldSaveRuntime = true;
  }
  #endif
}

// write runtime data to EEPROM (when required by "shouldSave Runtime")
void saveEEPROMData(void)
{
  if (!shouldSaveRuntime)
    return;
  shouldSaveRuntime = false;

  seg.CRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a,(unsigned char *)&seg + 2, sizeof(seg) - 2);

  strip->setCRC(seg.CRC);

  // write the data to the EEPROM
  EEPROM.put(0, seg);
  // as we work on ESP, we also need to commit the written data.
  EEPROM.commit();
}


// reads the stored runtime data from EEPROM
// must be called after everything else is already setup to be working
// otherwise this may terribly fail and could override what was read already
void readRuntimeDataEEPROM(void)
{
  // copy segment...
  
  // now separate global....
  //WS2812FX::segment seg;
  
  //read the configuration from EEPROM into RAM
  EEPROM.get(0, seg);

  // calculate the CRC over the data being stored in the EEPROM (except the CRC itself)
  uint16_t mCRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a, (unsigned char *)&seg + 2, sizeof(seg) - 2);

  
  // we have a matching CRC, so we update the runtime data.
  if (seg.CRC == mCRC)
  {
    (*strip->getSegment()) = seg;
    strip->init();
  }
  else // load defaults
  {
    strip->resetDefaults();
  }

  memset(&seg, 0, sizeof(seg));

  checkSegmentChanges();

  // no need to save right now. next save should be after /set?....
  shouldSaveRuntime = false;
  strip->setTransition();
}

// we do not want anything to distrub the OTA
// therefore there is a flag which could be used to prevent from that...
// However, seems that a connection being active via websocket interrupts
// even if the web socket server is stopped??
bool OTAisRunning = false;

void initOverTheAirUpdate(void)
{
  #ifdef HAS_KNOB_CONTROL
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // TODO: Implement Hostname in config and WIFI Settings?

  ArduinoOTA.setHostname(LED_NAME);

  ArduinoOTA.onStart([]() {
    FastLED.clear(true);
    display.clear();
    display.drawString(0, 0, F("Starting OTA..."));
    display.displayOn();
    display.display();
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();
    // we indicate our skEtch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });

  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {

    

    // OTA finished.
    display.drawString(0, 53, F("OTA finished!"));
    display.displayOn();
    display.display();

    // we delete the first uint16 in the EEPROM (which is the CRC)
    // as this will (hopefully) reset to defaults on SW updates
    if(RESET_DEFAULTS)
    {
      clearCRC();
    }

    // indicate that OTA is no longer running.
    OTAisRunning = false;
    delay(100);
    display.displayOff();
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress 
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int prog = (progress / (total / 100));
    display.clear();
    display.drawString(0, 0, F("Starte OTA..."));
    display.drawStringMaxWidth(0, 12, 128, "Prog: " + String(prog) + " % done");
    //display.drawStringMaxWidth(0, 12, 128, "Prog: " + String(progress) + " / " + String(total));
    display.drawProgressBar(1,33, 126, 7, prog);
    display.displayOn();
    display.display();
  });

  // something went wrong, we gonna show an error "message".
  ArduinoOTA.onError([](ota_error_t error) {
    String err = F("OTA Fehler: ");

    if (error == OTA_AUTH_ERROR) {
      err = err + F("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      err = err + F("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      err = err + F("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      err = err + F("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      err = err + F("End Failed");
    }

    display.clear();
    display.drawStringMaxWidth(0, 0,  128, F("Update failed!"));
    display.drawStringMaxWidth(0, 22, 128, err);
    display.drawStringMaxWidth(0, 43, 128, F("Reset in 5 Secs"));
    delay(5000);
    ESP.restart();
  });
  
  #else // HAS_KNOB_CONTROL
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  ArduinoOTA.setHostname(LED_NAME);

  ArduinoOTA.onStart([]() {

    strip->setIsRunning(false);
    strip->setPower(false);
    // the following is just to visually indicate the OTA start
    // which is done by blinking the complete stripe in different colors from yellow to green
    uint8_t factor = 85;
    for (uint8_t c = 0; c < 4; c++)
    {
      uint8_t r = 256 - (c * factor);
      uint8_t g = c > 0 ? (c * factor - 1) : (c * factor);
      fill_solid(strip->leds, strip->getStripLength(), CRGB(r, g, 0));
      strip->show();
      delay(250);
      fill_solid(strip->leds, strip->getStripLength(), CRGB::Black);
      strip->show();
      delay(500);
    }
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();
    // we indicate our sketch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });
  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    // OTA finished.
    // We fade out the green Leds being activated during OTA.
    bool ledsActive = true;
    while(ledsActive)
    {
      ledsActive = false;
      for(uint16_t i=0; i<strip->getStripLength(); i++)
      {
        if(strip->leds[i]) 
        {
          ledsActive = true;
          break;
        }
      }
      fadeToBlackBy(strip->leds, strip->getStripLength(), 8);
      strip->show();
      delay(2);
    }

    // we delete the first uint16 in the EEPROM (which is the CRC)
    // as this will (hopefully) reset to defaults on SW updates
    if(RESET_DEFAULTS)
    {
      clearCRC();
    }
    

    // indicate that OTA is no longer running. (rather useless)
    OTAisRunning = false;
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress on the strips as well to be informed if anything gets stuck...
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // OTA Update will show increasing green LEDs during progress:
    uint16_t progress_value = progress * 100 / (total / strip->getStripLength());
    uint16_t pixel = (uint16_t)(progress_value / 100);
    uint16_t temp_color = progress_value - (pixel * 100);
    if (temp_color > 255)
      temp_color = 255;

    strip->leds[pixel] = CRGB(0, (uint8_t)temp_color, 0);
    strip->show();
  });

  // something went wrong, we gonna show an error "message" via LEDs.
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
    }
    else if (error == OTA_BEGIN_ERROR) {
    }
    else if (error == OTA_CONNECT_ERROR) {
    }
    else if (error == OTA_RECEIVE_ERROR) {
    }
    else if (error == OTA_END_ERROR) {
    }
    // something went wrong during OTA.
    // We will fade in to red...
    for (uint16_t c = 0; c < 256; c++)
    {
      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
        strip->leds[i] = CRGB((uint8_t)c, 0, 0); 
      }
      strip->show();
      delay(2);
    }
    // We wait 5 seconds and then restart the ESP...
    delay(5000);
    ESP.restart();
  });
  #endif // !HAS_KNOB_CONTROL

  // start the service
  ArduinoOTA.begin();
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}

// for DEBUG purpose without Serial connection...
void showInitColor(CRGB Color)
{
#ifdef DEBUG
  Color.r = Color.r&0x20;
  Color.g = Color.g&0x20;
  Color.b = Color.b&0x20;
  fill_solid(strip->leds, NUM_INFORMATION_LEDS, Color);
  strip->show();
  delay(500);
#else
  delay(50);
#endif

}

// setup the Wifi connection with Wifi Manager...
void setupWiFi(uint16_t timeout = 240)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled()) return;
  #endif
  showInitColor(CRGB::Blue);
  delay(INITDELAY);

  WiFi.hostname(LED_NAME);

  WiFi.mode(WIFI_STA);

  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  WiFi.persistent(true);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&server, &dns);

#ifndef HAS_KNOB_CONTROL
  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(timeout);
 
  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

//tries to connect to last known settings
//if it does not connect it starts an access point with the specified name
//and goes into a blocking loop awaiting configuration

  if (!wifiManager.autoConnect(AP_SSID))
  {
    showInitColor(CRGB::Yellow);
    delay(3000);
    showInitColor(CRGB::Red);
    ESP.restart();
  }

  if(WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
  }

  WiFi.setAutoReconnect(true);
  //if we get here we have connected to the WiFi


#else // We have a control knob / button

  // If we are in button control mode
  // we only need WiFi for "convenience"
  wifiManager.setConfigPortalTimeout(timeout);
 
  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

//tries to connect to last known settings
//if it does not connect it starts an access point with the specified name
//and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(AP_SSID))
  {
    WiFiConnected = false;
  }
  else
  {
    WiFiConnected = true;
    wifi_disconnect_counter++;// number of times we (re-)connected // = 0; // reset only in case we actually reconnected via setup routine
    if(WiFi.getMode() != WIFI_STA)
    {
      WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(true);
  //if we get here we have connected to the WiFi
  }
  wifi_err_counter = 0; // need to reset this regardless the connected state. Otherwise we try to reconntect every loop....

#endif

  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
}

// helper function to change an 8bit value by the given percentage
// TODO: we could use 8bit fractions for performance
uint8_t changebypercentage(uint8_t value, uint8_t percentage)
{
  uint16_t ret = max((value * percentage) / 100, 10);
  if (ret > 255)
    ret = 255;
  return (uint8_t)ret;
}

// if /set was called
void handleSet(AsyncWebServerRequest *request)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled() || !WiFiConnected) return;
  #endif

  // to be completed in general
  // TODO: question: is there enough memory to store color and "timing" per pixel?
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

  AsyncJsonResponse * response = new AsyncJsonResponse();
  
  JsonObject& answerObj = response->getRoot();
  JsonObject& answer = answerObj.createNestedObject(F("currentState"));
  
  
  if (request->hasParam(F("mo")))
  {
    // flag to decide if this is an library effect
    bool isWS2812FX = false;
    // current library effect number
    uint8_t effect = strip->getMode();

    
    // just switch to the next if we get an "u" for up
    if (request->getParam(F("mo"))->value().c_str()[0] == 'u')
    {
      isWS2812FX = true;
      effect = strip->nextMode(AUTO_MODE_UP);
    }
    // switch to the previous one if we get a "d" for down
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'd')
    {
      isWS2812FX = true;
      effect = strip->nextMode(AUTO_MODE_DOWN);
    }
    // if we get an "o" for off, we switch off
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'o')
    {
      strip->setPower(false);
    }
    // for backward compatibility and FHEM:
    // --> activate fire flicker
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'f')
    {
      effect = FX_MODE_FIRE_FLICKER_INTENSE;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate rainbow effect
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'r')
    {
      effect = FX_MODE_RAINBOW_CYCLE;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate the K.I.T.T. (larson scanner)
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'k')
    {
      effect = FX_MODE_LARSON_SCANNER;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox
    else if (request->getParam(F("mo"))->value().c_str()[0] == 's')
    {
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox in white...
    else if (request->getParam(F("mo"))->value().c_str()[0] == 'w')
    {
      strip->setColor(CRGBPalette16(CRGB::White));
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // sunrise effect
    else if (request->getParam(F("mo"))->value() == F("Sunrise"))
    {
      // sunrise time in seconds
      if (request->hasParam(F("sec")))
      {
        strip->setSunriseTime(((uint16_t)strtoul(request->getParam(F("sec"))->value().c_str(), NULL, 10)) / 60);
      }
      // sunrise time in minutes
      else if (request->hasParam(F("min")))
      {
        strip->setSunriseTime(((uint16_t)strtoul(request->getParam(F("min"))->value().c_str(), NULL, 10)));
      }
      isWS2812FX = true;
      effect = FX_MODE_SUNRISE;
      strip->setTransition();
      answer.set(F("sunRiseTime"), strip->getSunriseTime());
      answer.set(F("sunRiseTimeToFinish"), strip->getSunriseTimeToFinish());
      answer.set(F("sunRiseMode"), F("sunrise"));
      answer.set(F("sunRiseActive"), F("on"));
    }
    // the same for sunset....
    else if (request->getParam(F("mo"))->value() == F("Sunset"))
    {
      // sunrise time in seconds
      if (request->hasParam(F("sec")))
      {
        strip->setSunriseTime(((uint16_t)strtoul(request->getParam(F("sec"))->value().c_str(), NULL, 10)) / 60);
      }
      // sunrise time in minutes
      else if (request->hasParam(F("min")))
      {
        strip->setSunriseTime( ((uint16_t)strtoul(request->getParam(F("min"))->value().c_str(), NULL, 10)));
      }

      // answer for the "calling" party
      isWS2812FX = true;
      effect = FX_MODE_SUNSET;
      strip->setTransition();
      //broadcastInt("sunriseset", strip->getSunriseTime());
      //sendStatus = true;
      answer.set(F("sunRiseTime"), strip->getSunriseTime());
      answer.set(F("sunRiseTimeToFinish"), strip->getSunriseTimeToFinish());
      answer.set(F("sunRiseMode"), F("sunset"));
      answer.set(F("sunRiseActive"), F("on"));
    }
    // finally - if nothing matched before - we switch to the effect  being provided.
    // we don't care if its actually an int or not
    // because it will be zero anyway if not.
    else
    {
      effect = (uint8_t)strtoul(request->getParam(F("mo"))->value().c_str(), NULL, 10);
      isWS2812FX = true;
    }
    // sanity only, actually handled in the library...
    if (effect >= strip->getModeCount())
    {
      effect = 0;
    }
    // activate the effect...
    if (isWS2812FX)
    {
      strip->setMode(effect);
      // in case it was stopped before
      // seems to be obsolete but does not hurt anyway...
      strip->start();

      answer.set(F("wsfxmode_Num"), effect);
      answer.set(F("wsfxmode"), strip->getModeName(effect));
      answer.set(F("state"), strip->getPower() ? F("on") : F("off"));
      answer.set(F("power"), strip->getPower());
    }
    else
    {
      answer.set(F("state"), strip->getPower() ? F("on") : F("off"));
      answer.set(F("power"), strip->getPower());
    }
    
  }
  // global on/off
  if (request->hasParam(F("power")))
  {
    if (request->getParam(F("power"))->value().c_str()[0] == '0')
    {
      strip->setPower(false);
    }
    else
    {
      strip->setPower(true);
      strip->setMode(strip->getMode());
    }
    answer.set(F("state"), strip->getPower() ? F("on") : F("off"));
    answer.set(F("power"), strip->getPower());
  }

  if(request->hasParam(F("isRunning")))
  {
    if (request->getParam(F("isRunning"))->value().c_str()[0]  == '0')
    {
      strip->setIsRunning(false);
    }
    else
    {
      strip->setIsRunning(true);
    }
    answer.set(F("isRunning"), strip->isRunning() ? F("running") : F("paused"));
  }

  // if we got a palette change
  if (request->hasParam(F("pa")))
  {
    // TODO: Possibility to setColors and new Palettes...
    uint8_t pal = (uint8_t)strtoul(request->getParam(F("pa"))->value().c_str(), NULL, 10);
    strip->setTargetPalette(pal);
    answer.set(F("palette_num"), strip->getTargetPaletteNumber());
    answer.set(F("palette_name"), strip->getPalName(strip->getTargetPaletteNumber()));
    answer.set(F("palette_count"), strip->getPalCount());
  }

  // if we got a new brightness value
  if (request->hasParam(F("br")))
  {
    uint8_t brightness = strip->getBrightness();
    if (request->getParam(F("br"))->value().c_str()[0] == 'u')
    {
      brightness = changebypercentage(brightness, 110);
    }
    else if (request->getParam(F("br"))->value().c_str()[0] == 'd')
    {
      brightness = changebypercentage(brightness, 90);
    }
    else
    {
      brightness = constrain((uint8_t)strtoul(request->getParam(F("br"))->value().c_str(), NULL, 10), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    }
    strip->setBrightness(brightness);
    answer.set(F("brightness"), strip->getBrightness());
  }

  // if we got a speed value
  // for backward compatibility.
  // is beat88 value anyway
  if (request->hasParam(F("sp")))
  {
    strip->setSpeed(constrain((uint16_t)strtoul(request->getParam(F("sp"))->value().c_str(), NULL, 10), BEAT88_MIN, BEAT88_MAX));
    answer.set(F("speed"), strip->getSpeed());
    answer.set(F("beat88"), strip->getSpeed());
    strip->setTransition();
  }

  // if we got a speed value (as beat88)
  if (request->hasParam(F("be")))
  {
    uint16_t speed = strip->getBeat88();
    if (request->getParam(F("be"))->value().c_str()[0] == 'u')
    {
      uint16_t ret = max((speed * 115) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else if (request->getParam(F("be"))->value().c_str()[0] == 'd')
    {
      uint16_t ret = max((speed * 80) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else
    {
      speed = constrain((uint16_t)strtoul(request->getParam(F("be"))->value().c_str(), NULL, 10), BEAT88_MIN, BEAT88_MAX);
    }
    strip->setSpeed(speed);
    answer.set(F("speed"), strip->getSpeed());
    answer.set(F("beat88"), strip->getSpeed());
    strip->setTransition();
  }

  // color handling
  // this is a bit tricky, as it handles either RGB as one or different values.

  // current color (first value from palette)
  uint32_t color = strip->getColor(0);
  bool setColor = false;
  // we got red
  if (request->hasParam(F("re")))
  {
    setColor = true;
    uint8_t re = (color >> 16) & 0xFF; 
    if (request->getParam(F("re"))->value().c_str()[0] == 'u')
    {
      re = changebypercentage(re, 110);
    }
    else if (request->getParam(F("re"))->value().c_str()[0] == 'd')
    {
      re = changebypercentage(re, 90);
    }
    else
    {
      re = constrain((uint8_t)strtoul(request->getParam(F("re"))->value().c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0x00ffff) | (re << 16);
  }
  // we got green
  if (request->hasParam(F("gr")))
  {
    setColor = true;
    uint8_t gr = (color >> 8) & 0xFF;
    if (request->getParam(F("gr"))->value().c_str()[0] == 'u')
    {
      gr = changebypercentage(gr, 110);
    }
    else if (request->getParam(F("gr"))->value().c_str()[0] == 'd')
    {
      gr = changebypercentage(gr, 90);
    }
    else
    {
      gr = constrain((uint8_t)strtoul(request->getParam(F("gr"))->value().c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0xff00ff) | (gr << 8);
  }
  // we got blue
  if (request->hasParam(F("bl")))
  {
    setColor = true;
    uint8_t bl = color & 0xFF;
    if (request->getParam(F("bl"))->value().c_str()[0] == 'u')
    {
      bl = changebypercentage(bl, 110);
    }
    else if (request->getParam(F("bl"))->value().c_str()[0] == 'd')
    {
      bl = changebypercentage(bl, 90);
    }
    else
    {
      bl = constrain((uint8_t)strtoul(request->getParam(F("bl"))->value().c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0xffff00) | (bl << 0);
  }
  // we got a 32bit color value (24 actually)
  if (request->hasParam(F("co")))
  {
    setColor = true;
    color = constrain((uint32_t)strtoul(request->getParam(F("co"))->value().c_str(), NULL, 16), 0, 0xffffff);
  }
  // we got one solid color value as r, g, b
  if (request->hasParam(F("solidColor")))
  {
    setColor = true;
    uint8_t r, g, b;
    r = constrain((uint8_t)strtoul(request->getParam(F("r"))->value().c_str(), NULL, 10), 0, 255);
    g = constrain((uint8_t)strtoul(request->getParam(F("g"))->value().c_str(), NULL, 10), 0, 255);
    b = constrain((uint8_t)strtoul(request->getParam(F("b"))->value().c_str(), NULL, 10), 0, 255);
    color = (r << 16) | (g << 8) | (b << 0);
  }
  // a single pixel...
  if (request->hasParam(F("pi")))
  {
    uint16_t pixel = constrain((uint16_t)strtoul(request->getParam(F("pi"))->value().c_str(), NULL, 10), 0, strip->getStripLength() - 1);

    strip->setMode(FX_MODE_VOID);
    strip->leds[pixel] = CRGB(color);
    // a range of pixels from start rnS to end rnE
    answer.set(F("wsfxmode_Num"), FX_MODE_VOID);
    answer.set(F("wsfxmode"), strip->getModeName(FX_MODE_VOID));
    answer.set(F("state"), strip->getPower() ? "on" : "off");
    answer.set(F("power"), strip->getPower());
  }
  //FIXME: Does not yet work. Lets simplyfy all of this!
  else if (request->hasParam(F("rnS")) && request->hasParam(F("rnE")))
  {
    uint16_t start = constrain((uint16_t)strtoul(request->getParam(F("rnS"))->value().c_str(), NULL, 10), 0, strip->getStripLength());
    uint16_t end = constrain((uint16_t)strtoul(request->getParam(F("rnE"))->value().c_str(), NULL, 10), start, strip->getStripLength());

    strip->setMode(FX_MODE_VOID);
    for (uint16_t i = start; i <= end; i++)
    {
      strip->leds[i] = CRGB(color);
    }
    answer.set(F("wsfxmode_Num"), FX_MODE_VOID);
    answer.set(F("wsfxmode"), strip->getModeName(FX_MODE_VOID));
    answer.set(F("state"), strip->getPower() ? F("on") : F("off"));
    answer.set(F("power"), strip->getPower());
    // one color for the complete strip
  }
  else if (request->hasParam(F("rgb")))
  {
    strip->setColor(color);
    strip->setMode(FX_MODE_STATIC);
    // finally set a new color
    answer.set(F("wsfxmode_Num"), FX_MODE_STATIC);
    answer.set(F("wsfxmode"), strip->getModeName(FX_MODE_STATIC));
    answer.set(F("state"), strip->getPower() ? F("on") : F("off"));
    answer.set(F("power"), strip->getPower());
  }
  else
  {
    if (setColor)
    {
      strip->setColor(color);
      answer.set(F("rgb"), color);
      answer.set(F("rgb_blue"), color & 0xFF);
      answer.set(F("rgb_green"), (color >> 8) & 0xFF);
      answer.set(F("rgb_red"), (color >> 16) & 0xFF);
    }
  }

  // autoplay flag changes
  if (request->hasParam(F("autoplay")))
  {
    uint16_t value = request->getParam(F("autoplay"))->value().toInt();
    strip->setAutoplay((AUTOPLAYMODES)value);
    switch(strip->getAutoplay())
    {
      case AUTO_MODE_OFF:
        answer[F("AutoPlayMode")] = F("Off");
      break;
      case AUTO_MODE_UP:
        answer[F("AutoPlayMode")] = F("Up");
      break;
      case AUTO_MODE_DOWN:
        answer[F("AutoPlayMode")] = F("Down");
      break;
      case AUTO_MODE_RANDOM:
        answer[F("AutoPlayMode")] = F("Random");
      break;
      default:
        answer[F("AutoPlayMode")] = F("unknown error");
      break;
    }
  
  }

  // autoplay duration changes
  if (request->hasParam(F("autoplayDuration")))
  {
    uint16_t value = request->getParam(F("autoplayDuration"))->value().toInt();
    strip->setAutoplayDuration(value);
    answer[F("AutoPlayModeIntervall")] = strip->getAutoplayDuration();
 
  }

  // auto plaette change
  if (request->hasParam(F("autopal")))
  {
    uint16_t value = request->getParam(F("autopal"))->value().toInt();
    strip->setAutopal((AUTOPLAYMODES)value);
    switch(strip->getAutopal())
    {
      case AUTO_MODE_OFF:
        answer[F("AutoPalette")] = F("Off");
      break;
      case AUTO_MODE_UP:
        answer[F("AutoPalette")] = F("Up");
      break;
      case AUTO_MODE_DOWN:
        answer[F("AutoPalette")] = F("Down");
      break;
      case AUTO_MODE_RANDOM:
        answer[F("AutoPalette")] = F("Random");
      break;
      default:
        answer[F("AutoPalette")] = F("unknown error");
      break;
    }
  }

  // auto palette change duration changes
  if (request->hasParam(F("autopalDuration")))
  {
    uint16_t value = request->getParam(F("autopalDuration"))->value().toInt();
    strip->setAutopalDuration(value);
    answer[F("AutoPaletteInterval")] = strip->getAutoplayDuration();
  }

  // time for cycling through the basehue value changes
  if (request->hasParam(F("huetime")))
  {
    uint16_t value = request->getParam(F("huetime"))->value().toInt();
    strip->setHuetime(value);
    answer[F("HueChangeInt")] = strip->getHueTime();
  }

#pragma message "We could implement a value to change how a palette is distributed accross the strip"

  // the hue offset for a given effect (if - e.g. not spread across the whole strip)
  if (request->hasParam(F("deltahue")))
  {
    uint16_t value = constrain(request->getParam(F("deltahue"))->value().toInt(), 0, 255);
    strip->setDeltaHue(value);
    strip->setTransition();
    answer[F("HueDeltaHue")] = strip->getDeltaHue();
  }

  // parameter for teh "fire" - flame cooldown
  if (request->hasParam(F("cooling")))
  {
    uint16_t value = request->getParam(F("cooling"))->value().toInt();
    strip->setCooling(value);
    strip->setTransition();
    answer[F("Cooling")] = strip->getCooling();
  }

  // parameter for the sparking - new flames
  if (request->hasParam(F("sparking")))
  {
    uint16_t value = request->getParam(F("sparking"))->value().toInt();
    strip->setSparking(value);
    strip->setTransition();
    answer[F("Sparking")] = strip->getSparking();
  }

  // parameter for twinkle fox (speed)
  if (request->hasParam(F("twinkleSpeed")))
  {
    uint16_t value = request->getParam(F("twinkleSpeed"))->value().toInt();
    strip->setTwinkleSpeed(value);
    strip->setTransition();
    answer[F("TwinkleSpeed")] = strip->getTwinkleSpeed();
  }

  // parameter for twinkle fox (density)
  if (request->hasParam(F("twinkleDensity")))
  {
    uint16_t value = request->getParam(F("twinkleDensity"))->value().toInt();
    strip->setTwinkleDensity(value);
    strip->setTransition();
    answer[F("TwinkleDensity")] = strip->getTwinkleDensity();
  }

  // parameter for number of bars (beat sine glows etc...)
  if (request->hasParam(F("numBars")))
  {
    uint16_t value = request->getParam(F("numBars"))->value().toInt();
    if (value > MAX_NUM_BARS)
    {
      value = max(MAX_NUM_BARS, 1);
    }
    strip->setNumBars(value);
    strip->setTransition();
    answer[F("NumBars")] = strip->getNumBars();
  }

  // parameter to change the palette blend type for cetain effects
  if (request->hasParam(F("blendType")))
  {
    uint16_t value = request->getParam(F("blendType"))->value().toInt();
    if (value)
    {
      strip->setBlendType(LINEARBLEND);
      answer[F("BlendType")] = F("Linear Blend");
    }
    else
    {
      strip->setBlendType(NOBLEND);
      answer[F("BlendType")] = F("No Blend");
    }
    strip->setTransition();
  }

  // parameter to change the Color Temperature of the Strip
  if (request->hasParam(F("ColorTemperature")))
  {
    uint8_t value = request->getParam(F("ColorTemperature"))->value().toInt();
    strip->setColorTemperature(value);
    strip->setTransition();
    answer[F("ColorTemperature")] = strip->getColorTempName(strip->getColorTemp());
  }

  // parameter to change direction of certain effects..
  if (request->hasParam(F("reverse")))
  {
    uint16_t value = request->getParam(F("reverse"))->value().toInt();
    strip->getSegment()->reverse = value;
    strip->setTransition();
    answer[F("Reverse")] = strip->getReverse();
  }

  // parameter to invert colors of all effects..
  if (request->hasParam(F("inverse")))
  {
    uint16_t value = request->getParam(F("inverse"))->value().toInt();
    strip->setInverse(value);
    strip->setTransition();
    answer[F("Inverse")] = strip->getInverse();
  }

  // parameter to divide LEDS into two equal halfs...
  if (request->hasParam(F("mirror")))
  {
    uint16_t value = request->getParam(F("mirror"))->value().toInt();
    strip->setMirror(value);
    strip->setTransition();
    answer[F("Mirrored")] = strip->getMirror();
  }

  // parameter so set the max current the leds will draw
  if (request->hasParam(F("current")))
  {
    uint16_t value = request->getParam(F("current"))->value().toInt();
    strip->setMilliamps(value);
    answer[F("Lamp_max_current")] = strip->getMilliamps();
  }

  // parameter for the blur against the previous LED values
  if (request->hasParam(F("LEDblur")))
  {
    uint8_t value = request->getParam(F("LEDblur"))->value().toInt();
    strip->setBlur(value);
    strip->setTransition();
    answer[F("Led_blur")] = strip->getBlurValue();
  }

  // parameter for the frames per second (FPS)
  if (request->hasParam(F("fps")))
  {
    uint8_t value = request->getParam(F("fps"))->value().toInt();
    strip->setMaxFPS(value);
    strip->setTransition();
    answer[F("max_FPS")] = strip->getMaxFPS();
  }

  if (request->hasParam(F("dithering")))
  {
    uint8_t value = request->getParam(F("dithering"))->value().toInt();
    strip->setDithering(value);
    answer[F("Dithering")] = strip->getDithering();
  }

  if (request->hasParam(F("sunriseset")))
  {
    uint8_t value = request->getParam(F("sunriseset"))->value().toInt();
    strip->getSegment()->sunrisetime = value;
    answer[F("sunRiseTime")] = strip->getSunriseTime();
  }

  // reset to default values
  if (request->hasParam(F("resetdefaults")))
  {
    uint8_t value = request->getParam(F("resetdefaults"))->value().toInt();
    if (value)
      strip->resetDefaults();
    strip->setTransition();
  }

  if (request->hasParam(F("damping")))
  {
    uint8_t value = constrain(request->getParam(F("damping"))->value().toInt(), 0, 100);
    strip->getSegment()->damping = value;
    answer[F("Damping")] = strip->getDamping();
  }

  if (request->hasParam(F("addGlitter")))
  {
    uint8_t value = constrain(request->getParam(F("addGlitter"))->value().toInt(), 0, 100);
    strip->setAddGlitter(value);
    answer[F("Glitter_Add")] = strip->getAddGlitter();
  }
  if (request->hasParam(F("WhiteOnly")))
  {
    uint8_t value = constrain(request->getParam(F("WhiteOnly"))->value().toInt(), 0, 100);
    strip->setWhiteGlitter(value);
    answer[F("Glitter_White")] = strip->getWhiteGlitter();
  }
  if (request->hasParam(F("onBlackOnly")))
  {
    uint8_t value = constrain(request->getParam(F("onBlackOnly"))->value().toInt(), 0, 100);
    strip->setOnBlackOnly(value);
    answer[F("Glitter_OnBlackOnly")] = strip->getOnBlackOnly();
  }
  if (request->hasParam(F("syncGlitter")))
  {
    uint8_t value = constrain(request->getParam(F("syncGlitter"))->value().toInt(), 0, 100);
    strip->setSynchronous(value);
    answer[F("Glitter_syncGlitter")] = strip->getSynchronous();
  }
#ifdef DEBUG
  // Testing different Resets
  // can then be triggered via web interface (at the very bottom)
  if (request->hasParam(F("resets")))
  {
    uint8_t value = request->getParam(F("resets"))->value().toInt();
    volatile uint8_t d = 1;
    switch (value)
    {
    case 0:
      break;
    case 1: //
      ESP.reset();
      break;
    case 2:
      ESP.restart();
      break;
    case 3:
      ESP.wdtDisable();
      break;
    case 4:
      while (d)
      {
        d=1;
      }
      break;
    case 5:
      volatile uint8_t a = 0;
      volatile uint8_t b = 5;
      volatile uint8_t c = 0;
      c = b / a;
      break;
    }
  }
#endif

  // parameter for number of segments
  if (request->hasParam(F("segments")))
  {
    uint16_t value = request->getParam(F("segments"))->value().toInt();
    strip->getSegment()->segments = constrain(value, 1, MAX_NUM_SEGMENTS);
    strip->setTransition();
    answer[F("Segments")] = strip->getSegments();
  }

  if (request->hasParam(F("BckndHue")))
  {
    uint8_t value = request->getParam(F("BckndHue"))->value().toInt();
    strip->setBckndHue(value);
    answer[F("BckndHue")] = strip->getBckndHue();
  }
  if (request->hasParam(F("BckndSat")))
  {
    uint8_t value = request->getParam(F("BckndSat"))->value().toInt();
    strip->setBckndSat(value);
    answer[F("BckndSat")] = strip->getBckndSat();
  }
  if (request->hasParam(F("BckndBri")))
  {
    uint8_t value = request->getParam(F("BckndBri"))->value().toInt();
    strip->setBckndBri(value);
    answer[F("BckndBri")] = strip->getBckndBri();
  }


  #ifdef HAS_KNOB_CONTROL
  if (request->hasParam(F("wifiDisabled")))
  {
    uint16_t value = request->getParam(F("wifiDisabled"))->value().toInt();
    if(value)
      strip->setWiFiDisabled(true);
    else
      strip->setWiFiDisabled(false);    

    answer[F("wifiDisabled")] = strip->getWiFiDisabled();
  }
  #endif

  response->setLength();
  request->send(response);
}

// if something unknown was called...
void handleNotFound(AsyncWebServerRequest * request)
{
  /*
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
  */
  AsyncWebServerResponse *response = request->beginResponse(404); //Sends 404 File Not Found
  response->addHeader("Server",LED_NAME);
  request->send(response);
}

void handleGetModes(AsyncWebServerRequest *request)
{

  AsyncJsonResponse * response = new AsyncJsonResponse();

  JsonObject &root = response->getRoot();

  JsonObject &modeinfo = root.createNestedObject(F("modeinfo"));
  modeinfo[F("count")] = strip->getModeCount();

  JsonObject &modeinfo_modes = modeinfo.createNestedObject(F("modes"));
  for (uint8_t i = 0; i < strip->getModeCount(); i++)
  {
    modeinfo_modes[strip->getModeName(i)] = i;
  }
  response->setLength();
  request->send(response);
}

void handleGetPals(AsyncWebServerRequest *request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse();

  JsonObject &root = response->getRoot();

  JsonObject &modeinfo = root.createNestedObject(F("palinfo"));
  modeinfo[F("count")] = strip->getPalCount();

  JsonObject &modeinfo_modes = modeinfo.createNestedObject(F("pals"));
  for (uint8_t i = 0; i < strip->getPalCount(); i++)
  {
    modeinfo_modes[strip->getPalName(i)] = i;
  }
  response->setLength();
  request->send(response);
}

void handleStatus(AsyncWebServerRequest *request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse();

  JsonObject &answerObj = response->getRoot();
  JsonObject& currentStateAnswer = answerObj.createNestedObject(F("currentState"));
  JsonObject& sunriseAnswer = answerObj.createNestedObject(F("sunRiseState"));
  JsonObject& statsAnswer = answerObj.createNestedObject(F("Stats"));

  uint16_t num_leds_on = strip->getLedsOn();

  currentStateAnswer[F("power")] = strip->getPower();
  if (strip->getPower())
  {
    currentStateAnswer[F("state")] = F("on");
  }
  else
  {
    currentStateAnswer[F("state")] = F("off");
  }


  currentStateAnswer[F("Buildversion")] = build_version;
  currentStateAnswer[F("Git_Revision")] = git_revision;
  currentStateAnswer[F("Lampname")] = LED_NAME;
  currentStateAnswer[F("LED_Count")] = LED_COUNT;
  currentStateAnswer[F("Lamp_max_current")] = strip->getMilliamps();
  currentStateAnswer[F("Lamp_max_power")] = strip->getVoltage() * strip->getMilliamps();
  currentStateAnswer[F("Lamp_current_power")] = strip->getCurrentPower();
  currentStateAnswer[F("LEDs_On")] = num_leds_on;
  currentStateAnswer[F("mode_Name")] = strip->getModeName(strip->getMode());
  currentStateAnswer[F("wsfxmode")] = strip->getModeName(strip->getMode());
  currentStateAnswer[F("wsfxmode_Num")] = strip->getMode();
  currentStateAnswer[F("wsfxmode_count")] = strip->getModeCount();
  currentStateAnswer[F("beat88")] = strip->getBeat88();
  currentStateAnswer[F("speed")] = strip->getBeat88();
  currentStateAnswer[F("brightness")] = strip->getBrightness();
  // Palettes and Colors
  currentStateAnswer[F("palette_count")] = strip->getPalCount();
  currentStateAnswer[F("palette_num")] = strip->getTargetPaletteNumber();
  currentStateAnswer[F("palette_name")] = strip->getPalName(strip->getTargetPaletteNumber());
  CRGB col = CRGB::Black;
  // We return either black (strip effectively off)
  // or the color of the first pixel....
  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    if (strip->_bleds[i])
    {
      col = strip->_bleds[i];
      break;
    }
  }
  currentStateAnswer[F("rgb")] = (((col.r << 16) | (col.g << 8) | (col.b << 0)) & 0xffffff);
  currentStateAnswer[F("rgb_red")] = col.r;
  currentStateAnswer[F("rgb_green")] = col.g;
  currentStateAnswer[F("rgb_blue")] = col.b;

  if (strip->getSegment()->blendType == NOBLEND)
  {
    currentStateAnswer[F("BlendType")] = F("No Blend");
  }
  else if (strip->getSegment()->blendType == LINEARBLEND)
  {
    currentStateAnswer[F("BlendType")] = F("Linear Blend");
  }
  else
  {
    currentStateAnswer[F("BlendType")] = F("Unknown Blend");
  }

  currentStateAnswer[F("Reverse")] = strip->getReverse();;
  currentStateAnswer[F("HueChangeInt")] = strip->getHueTime();
  currentStateAnswer[F("HueDeltaHue")] = strip->getDeltaHue();
  switch(strip->getAutoplay())
  {
    case AUTO_MODE_OFF:
      currentStateAnswer[F("AutoPlayMode")] = F("Off");
    break;
    case AUTO_MODE_UP:
      currentStateAnswer[F("AutoPlayMode")] = F("Up");
    break;
    case AUTO_MODE_DOWN:
      currentStateAnswer[F("AutoPlayMode")] = F("Down");
    break;
    case AUTO_MODE_RANDOM:
      currentStateAnswer[F("AutoPlayMode")] = F("Random");
    break;
    default:
      currentStateAnswer[F("AutoPlayMode")] = F("unknown error");
    break;
  }
  currentStateAnswer[F("AutoPlayModeIntervall")] = strip->getAutoplayDuration();
  switch(strip->getAutopal())
  {
    case AUTO_MODE_OFF:
      currentStateAnswer[F("AutoPalette")] = F("Off");
    break;
    case AUTO_MODE_UP:
      currentStateAnswer[F("AutoPalette")] = F("Up");
    break;
    case AUTO_MODE_DOWN:
      currentStateAnswer[F("AutoPalette")] = F("Down");
    break;
    case AUTO_MODE_RANDOM:
      currentStateAnswer[F("AutoPalette")] = F("Random");
    break;
    default:
      currentStateAnswer[F("AutoPalette")] = F("unknown error");
    break;
  }
  currentStateAnswer[F("AutoPaletteInterval")] = strip->getAutoplayDuration();


  if (strip->getMode() == FX_MODE_SUNRISE)
  {
    sunriseAnswer[F("sunRiseMode")] = F("Sunrise");
  }
  else if (strip->getMode() == FX_MODE_SUNSET)
  {
    sunriseAnswer[F("sunRiseMode")] = F("Sunset");
  }
  else
  {
    sunriseAnswer[F("sunRiseMode")] = F("None");
  }
  
  if (strip->getMode() == FX_MODE_SUNRISE || strip->getMode() == FX_MODE_SUNSET)
  {
    if(num_leds_on)
    {
      sunriseAnswer[F("sunRiseActive")] = F("on");
    }
    else
    {
      sunriseAnswer[F("sunRiseActive")] = F("off");
    }
  }
  else
  {
    sunriseAnswer[F("sunRiseActive")] = F("off");
  }
    sunriseAnswer[F("sunRiseCurrStep")] = strip->getCurrentSunriseStep();
    
    sunriseAnswer[F("sunRiseTotalSteps")] = DEFAULT_SUNRISE_STEPS;
    
    sunriseAnswer[F("sunRiseTimeToFinish")] = strip->getSunriseTimeToFinish();

    sunriseAnswer[F("sunRiseTime")] = strip->getSunriseTime();
  
  
  switch (getResetReason())
  {
  case REASON_DEFAULT_RST:
    
    statsAnswer[F("Chip_ResetReason")] = F("Normal Boot");
    break;
  case REASON_WDT_RST:
    statsAnswer[F("Chip_ResetReason")] = F("WDT Reset");
    break;
  case REASON_EXCEPTION_RST:
    statsAnswer[F("Chip_ResetReason")] = F("Exception");
    break;
  case REASON_SOFT_WDT_RST:
    statsAnswer[F("Chip_ResetReason")] = F("Soft WDT Reset");
    break;
  case REASON_SOFT_RESTART:
    statsAnswer[F("Chip_ResetReason")] = F("Restart");
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    statsAnswer[F("Chip_ResetReason")] = F("Sleep Awake");
    break;
  case REASON_EXT_SYS_RST:
    statsAnswer[F("Chip_ResetReason")] = F("External Trigger");
    break;

  default:
    statsAnswer[F("Chip_ResetReason")] = F("Unknown Cause");
    break;
  }
  statsAnswer[F("Chip_ID")] = String(ESP.getChipId());
  statsAnswer[F("WIFI_IP")] =  WiFi.localIP().toString();
  statsAnswer[F("WIFI_CONNECT_ERR_COUNT")] = wifi_disconnect_counter;
  statsAnswer[F("WIFI_SIGNAL")] = String(WiFi.RSSI());  // for #14
  statsAnswer[F("WIFI_CHAN")] = String(WiFi.channel());  // for #14
  
  statsAnswer[F("FPS")] = FastLED.getFPS();
  response->setLength();
  request->send(response);
}

void factoryReset(void)
{
  // on factory reset, each led will be red
  // increasing from led 0 to max.
  for (uint16_t i = 0; i < strip->getStripLength(); i++)
  {
    strip->leds[i] = 0xa00000;
    strip->show();
    delay(2);
  }
  strip->show();
  DNSServer dns;
  AsyncWiFiManager wifimgr(&server, &dns);
  delay(INITDELAY);
  wifimgr.resetSettings();
  // wifimgr.erase(); // seems to be removed from WiFiManager
  delay(INITDELAY);
  clearEEPROM();
  WiFi.persistent(false);
  
//reset and try again
  ESP.restart();
}

uint8_t getResetReason(void)
{
  return (uint8_t)ESP.getResetInfoPtr()->reason;
}

/*
 * Clear the CRC to startup Fresh....
 * Used in case we end up in a WDT reset (either SW or HW)
 */
void clearCRC(void)
{
// invalidating the CRC - in case somthing goes terribly wrong...
  EEPROM.begin(strip->getCRCsize());
  EEPROM.put(0,(uint16_t)0);
  EEPROM.commit();
  EEPROM.end();
  delay(100);
  ESP.restart();
}

void clearEEPROM(void)
{
//Clearing EEPROM
  EEPROM.begin(strip->getSegmentSize());
  for (uint32_t i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}

// Received Factoryreset request.
// To be sure we check the related parameter....
void handleResetRequest(AsyncWebServerRequest * request)
{
  if (request->getParam(F("rst"))->value() == F("FactoryReset"))
  {
    request->send(200, F("text/plain"), F("Will now Reset to factory settings. You need to connect to the WLAN AP afterwards...."));
    factoryReset();
  }
  else if (request->getParam(F("rst"))->value() == F("Defaults"))
  {

    strip->setTargetPalette(0);
    strip->setMode(0);
    strip->stop();
    strip->resetDefaults();
    request->send(200, F("text/plain"), F("Strip was reset to the default values..."));
    shouldSaveRuntime = true;
  }
}

void updateConfigFile(void)
{
  DynamicJsonBuffer buffer;
  JsonArray& root = buffer.createArray();   
  for (uint8_t i = 0; i < fieldCount; i++)
  {
    Field field = fields[i];
    JsonObject& obj = root.createNestedObject();
    obj[F("name")]  = field.name;
    obj[F("label")] = field.label;
    obj[F("type")]  = (int)field.type;
    if (field.getValue)
    {
      if (field.type == ColorFieldType)//(const char *)"Color")
      {
        CRGB solidColor = (*strip->getTargetPalette()).entries[0];
        obj[F("value")] = String(solidColor.r) + String(",") + String(solidColor.g)+","+String(solidColor.b);
      }
      else
      {
        obj[F("value")] = field.getValue();
      }
    }

    if (field.type == NumberFieldType) //(const char *)"Number")
    {
      obj["min"] = field.min;
      obj["max"] = field.max;
    }

    if (field.getOptions)
    {
      JsonArray &arr = obj.createNestedArray("options");
      field.getOptions(arr);
    }
  }

  File config_Json = LittleFS.open("/config_all.json", "w");

  root.printTo(config_Json);

  config_Json.close();
}

void setupWebServer(void)
{
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  

  server.on("/all", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!LittleFS.exists("/config_all.json"))
    {
      updateConfigFile();
    }
    request->send(LittleFS, "/config_all.json", "application/json");
  });


  server.on("/allvalues", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();

    JsonObject &root = response->getRoot();
    JsonArray &arr = root.createNestedArray("values");
    for(uint8_t i=0; i<fieldCount; i++)
    {
      if(fields[i].type < TitleFieldType)
      {
        JsonObject &obj = arr.createNestedObject();
        obj["name"] =  fields[i].name;
        obj["value"] = fields[i].getValue();
      }
    }
    response->setLength();
    request->send(response);
  });

  server.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    String name = request->getParam(F("name"))->value();
    String response = getFieldValue(name.c_str(), fields, fieldCount);
    request->send(200, F("text/plain"), response);
  });

  
  // keepAlive
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, F("text/plain"), F("OK"));
  });

  server.on("/set", handleSet);
  server.on("/getmodes", handleGetModes);
  server.on("/getpals", handleGetPals);
  server.on("/status", handleStatus);
  server.on("/reset", handleResetRequest);
  server.onNotFound(handleNotFound);

  server.on("/", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    request->send(200,  F("text/plain"), "" );
  });

  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
    String path = "/edit.htm";
    
    String pathWithGz = path + ".gz";
    if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){
      if(LittleFS.exists(pathWithGz))
        path += ".gz";
      //File file = LittleFS.open(path, "r");
      String contentType = getContentType(path);
      //size_t sent = server.streamFile(file, contentType);
      request->send(LittleFS, path, contentType);
      //server.streamFile(file, contentType);
      //file.close();
    }
    else
    {
      request->send(404, "text/plain", "FileNotFound: " + path);
    }
    Dir dir = LittleFS.openDir("/");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, handleFileUpload);

  server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=1");
  delay(INITDELAY);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "1");

  if(webSocketsServer == NULL) webSocketsServer = new AsyncWebSocket("/ws");
  webSocketsServer->enable(true);
  //webSocketsServer->begin();
  
  webSocketsServer->onEvent(webSocketEvent);

  server.addHandler(webSocketsServer);

  server.begin();

  showInitColor(CRGB::Yellow);
  delay(INITDELAY);

  
 
  if (!MDNS.begin(LED_NAME)) {

  }
  else
  {
    MDNS.addService("http", "tcp", 80);
  }

  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}
// Ping-Pong Struct
// holding the iD, current Ping and Current Pong
struct pingPong {
  uint32_t iD = 0;
  uint8_t pong = 0;
  uint8_t ping = 0;
  // Array holding the clients PingPongs
}my_pingPongs[DEFAULT_MAX_WS_CLIENTS+1]; // as Array


// add a client to the pingpong list
uint8_t addClient(uint32_t iD)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled()) return DEFAULT_MAX_WS_CLIENTS;
  #endif
  if(webSocketsServer->count() >= DEFAULT_MAX_WS_CLIENTS)
    return DEFAULT_MAX_WS_CLIENTS;
  for(const auto& c: webSocketsServer->getClients())
  {
    for(uint8_t i=0; i<DEFAULT_MAX_WS_CLIENTS; i++)
    {
      if(c->id() == iD)
        return i;
    }
    for(uint8_t i=0; i<DEFAULT_MAX_WS_CLIENTS; i++)
    {
        if(my_pingPongs[i].iD == 0)
        {
          my_pingPongs[i].iD = iD;
          my_pingPongs[i].ping = 0;
          my_pingPongs[i].pong = 0;
          return i;
        }
    }
  }
  return DEFAULT_MAX_WS_CLIENTS;
}

// get a client number based on the id.
uint8_t getClient(uint32_t iD)
{
  for(uint8_t i=0; i<DEFAULT_MAX_WS_CLIENTS; i++)
  {
    if(my_pingPongs[i].iD == iD)
    {
      return i;
    }
  }
  return DEFAULT_MAX_WS_CLIENTS;
}

// remove a client from the ping pong list
void removeClient(uint32_t iD)
{
  for(uint8_t i=0; i<DEFAULT_MAX_WS_CLIENTS; i++)
  {
    if(iD == my_pingPongs[i].iD)
    {
      my_pingPongs[i].iD = 0;
      my_pingPongs[i].ping = 0;
      my_pingPongs[i].pong = 0;
    }
  }
}

// TODO: Make something useful with the Websocket Event
void webSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled()) return;
  #endif
  if(type == WS_EVT_CONNECT){
    //client connected
    #ifdef DEBUG
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    #endif
    client->printf("{\"Client\":%u}", client->id());
    uint8_t i = addClient(client->id());
    my_pingPongs[i].ping = random8();
    client->ping(&my_pingPongs[i].ping, sizeof(uint8_t));
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    #ifdef DEBUG
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    #endif
    removeClient(client->id());
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    #ifdef DEBUG
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    #endif
  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    #ifdef DEBUG
    Serial.printf("ws[%s][%u] pong[%u]: %u\n", server->url(), client->id(), len, (len)?*data:0);
    #endif
    uint8_t i = getClient(client->id());
    my_pingPongs[i].pong = (len)?*data:0;
    
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      #ifdef DEBUG
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      #endif
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        #ifdef DEBUG
        Serial.printf("%s\n", (char*)data);
        #endif
      } else {
        for(size_t i=0; i < info->len; i++){
          #ifdef DEBUG
          Serial.printf("%02x ", data[i]);
          #endif
        }
        #ifdef DEBUG
        Serial.printf("\n");
        #endif
      }
      #ifdef DEBUG
      if(info->opcode == WS_TEXT)
       client->text("{\"message\":\"I got your text message\"}");
      else
        client->binary("{\"message\":\"I got your binary message\"}");
      #endif
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        #ifdef DEBUG
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
        #endif
      }

      #ifdef DEBUG
        Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      #endif
      if(info->message_opcode == WS_TEXT){
        data[len] = 0;
        #ifdef DEBUG
        Serial.printf("%s\n", (char*)data);
        #endif
      } else {
        for(size_t i=0; i < len; i++){
          #ifdef DEBUG
          Serial.printf("%02x ", data[i]);
          #endif
        }
        #ifdef DEBUG
        Serial.printf("\n");
        #endif
      }

      if((info->index + len) == info->len){
        #ifdef DEBUG
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        #endif
        if(info->final){
          #ifdef DEBUG
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("{\"message\":\"I got your framed text message\"}");
          else
            client->binary("{\"message\":\"I got your framed binary message\"}");
          #endif
        }
      }
    }
  }
}

#ifdef HAS_KNOB_CONTROL

uint16_t maxVal = 65535;
uint16_t minVal = 0;
uint16_t curVal = 0;
uint16_t steps  = 1;

void setupKnobControl(void)
{ 
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

uint8_t drawtxtline10(uint8_t y, uint8_t fontheight, String txt)
{
  if(txt == "") return y;
  uint8_t txtLines = 1;
  txtLines = (((display.getStringWidth(txt) - 2) / 128) + 1) * fontheight;
  display.drawStringMaxWidth (0,  y, 128, txt);
  y+=txtLines;
  return y;
}


enum displayStates {
  Display_Off,
  Display_ShowInfo,
  Display_ShowMenu,
  Display_ShowSectionMenue,
  Display_ShowBoolMenu,
  Display_ShowNumberMenu,
  Display_ShowSelectMenue
} mDisplayState;
#endif

// setup network and output pins
void setup()
{
  
  // Sanity delay to get everything settled....
  delay(INITDELAY);
  LittleFS.begin();
  #ifdef HAS_KNOB_CONTROL
  const uint8_t font_height = 12;

  setupKnobControl();

  uint8_t cursor = 0;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  cursor = drawtxtline10(cursor, font_height, F("Booting... Bitte Warten"));
  display.display();
  
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  #endif

  mDisplayState = Display_ShowInfo;

  switch (getResetReason())
  {
  case REASON_DEFAULT_RST:
    cursor = drawtxtline10(cursor, font_height, F("REASON_DEFAULT_RST"));
    display.display();
    break;
  case REASON_WDT_RST:
    cursor = drawtxtline10(cursor, font_height, F("REASON_WDT_RST"));
    display.display();
    delay(2000);
    clearCRC(); // should enable default start in case of
    break;
  case REASON_EXCEPTION_RST:
    cursor = drawtxtline10(cursor, font_height, F("REASON_EXCEPTION_RST"));
    display.display();
    delay(2000);
    clearCRC();
    break;
  case REASON_SOFT_WDT_RST:
    cursor = drawtxtline10(cursor, font_height, F("REASON_SOFT_WDT_RST"));
    display.display();
    delay(2000);
    clearCRC();
    break;
  case REASON_SOFT_RESTART:
    cursor = drawtxtline10(cursor, font_height, F("REASON_SOFT_RESTART"));
    display.display();
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    cursor = drawtxtline10(cursor, font_height, F("REASON_DEEP_SLEEP_AWAKE"));
    display.display();
    break;
  case REASON_EXT_SYS_RST:
    cursor = drawtxtline10(cursor, font_height, F("REASON_EXT_SYS_RST"));
    display.display();
    break;

  default:
    cursor = drawtxtline10 (cursor, 10, F("Unknown cause..."));
    display.display();
    break;
  }
  cursor = drawtxtline10 (cursor, 10, F("LED Stripe init"));
  display.display();
  

  stripe_setup(STRIP_VOLTAGE,
               UncorrectedColor); //TypicalLEDStrip);

  updateConfigFile();

  EEPROM.begin(strip->getSegmentSize());

  //EEPROM.get(0, seg);
  readRuntimeDataEEPROM();

  if(!seg.wifiDisabled)
  {
    cursor = drawtxtline10(cursor, font_height, F("WiFi-Setup"));
    display.display();
    setupWiFi();
    cursor = drawtxtline10(cursor, font_height, F("WebServer Setup"));
    display.display();
    setupWebServer();
    cursor = drawtxtline10(cursor, font_height, F("OTA Setup"));
    display.display();
    initOverTheAirUpdate();
  }  
  
  //readRuntimeDataEEPROM();

  updateConfigFile();

  delay(KNOB_BOOT_DELAY);
  display.clear();
  cursor = 0;
  cursor = drawtxtline10(cursor, font_height, "Boot fertig!");
  cursor = drawtxtline10(cursor, font_height, "Name: " + String(LED_NAME));
  cursor = drawtxtline10(cursor, font_height, "IP: " + WiFi.localIP().toString());
  cursor = drawtxtline10(cursor, font_height, "LEDs: " + String(LED_COUNT));
  display.display();
  delay(KNOB_BOOT_DELAY);

  display.clear();  
  
#else // HAS_KNOB_CONTROL

  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  #endif

  switch (getResetReason())
  {
    case REASON_DEFAULT_RST:
      
      break;
    case REASON_WDT_RST:
      
      clearCRC(); // should enable default start in case of
      break;
    case REASON_EXCEPTION_RST:
      
      clearCRC();
      break;
    case REASON_SOFT_WDT_RST:
      
      clearCRC();
      break;
    case REASON_SOFT_RESTART:
      
      break;
    case REASON_DEEP_SLEEP_AWAKE:
      
      break;
    case REASON_EXT_SYS_RST:
      
      break;

    default:
      
      break;
  }
  delay(10);
   
  stripe_setup(STRIP_VOLTAGE,
               UncorrectedColor); //TypicalLEDStrip);

  updateConfigFile();

  EEPROM.begin(strip->getSegmentSize());
  
  // internal LED can be light up when current is limited by FastLED
  
  pinMode(2, OUTPUT);

  EEPROM.begin(strip->getSegmentSize());

  delay(10);
  
  setupWiFi();

  setupWebServer();

  if (!MDNS.begin(LED_NAME)) {

  }
  else
  {
    MDNS.addService("http", "tcp", 80);
  }

  initOverTheAirUpdate();

  // if we got that far, we show by a nice little animation
  // as setup finished signal....
  const uint16_t mindelay = map(NUM_INFORMATION_LEDS, LED_COUNT>300?LED_COUNT:300, 0, 1, 100);
  for (uint8_t a = 0; a < 1; a++)
  {
    for (uint16_t c = 0; c < 32; c += 3)
    {
      for (uint16_t i = 0; i < NUM_INFORMATION_LEDS; i++)
      {
        strip->leds[i].green = c;
      }
      strip->show();
      
      delay(mindelay);
    }
    delay(20);
    for (int16_t c = strip->leds[0].green; c > 0; c -= 3)
    {
      for (uint16_t i = 0; i < NUM_INFORMATION_LEDS; i++)
      {
        strip->leds[i].subtractFromRGB(4);
      }
      strip->show();
      delay(mindelay);
    }
  }
  //strip->stop();
  delay(INITDELAY);

  // Show the IP Address at the beginning
  // so one can take a picture. 
  // one needs to know the structure of the leds...

  if(LED_COUNT >= 40)
  {
    // can show the complete IP Address on the first 40 LEDs
    for(uint8_t j=0; j<4; j++)
    {
      for(uint8_t i=0; i<8; i++)
      {
        if((WiFi.localIP()[j] >> i) & 0x01)
        {
          strip->_bleds[j * 10 + 7 - i] = CRGB::Red;
        }
        else
        {
          strip->_bleds[j * 10 + 7 - i] = CRGB(16,16,16);
        }
      }
    }
  }
  else if(LED_COUNT >= 8)
  {
    // can show the last ocet i.e. 192.168.2.XXX where XXX will be shown
  }
  else
  {
    // will show the IP as a "hue" on the first LED.
  }
  FastLED.show();

  delay(2000);

  readRuntimeDataEEPROM();

  updateConfigFile();

  #endif // HAS_KNOB_CONTROL
}

#ifdef HAS_KNOB_CONTROL
uint8_t get_next_field(uint8_t curr_field, bool up) //, fieldtypes *m_fieldtypes)
{
  uint8_t ret = curr_field;
  uint8_t first_field = 0;
  uint8_t last_field = fieldCount-1;
  for(uint8_t i=0; i<fieldCount; i++)
  {
    //if(m_fieldtypes[i] <= SelectFieldType)
    if(fields[i].type <= SelectFieldType)
    {
      last_field = i;
    }
    //if(m_fieldtypes[fieldCount-1-i] <= SelectFieldType)
    if(fields[fieldCount-1-i].type <= SelectFieldType)
    {
      first_field = fieldCount-1-i;
    }
  }
  uint8_t sanity = 0;
  //while(ret == curr_field || m_fieldtypes[ret] > SelectFieldType)
  while(ret == curr_field || fields[ret].type > SelectFieldType)
  {
    // sanity break....
    if(sanity++ > fieldCount) break;

    if(up)
    {
      ret++;
      if(ret >= last_field)
      {
        ret = last_field;
        //ret = fieldCount-1;
      }
    }
    else
    {
      if(ret > first_field) 
      {
        ret--;
      }
      else
      {
        ret = first_field; //fieldCount-1;
      } 
    }
  }
  return ret;
}


uint16_t setEncoderValues(uint8_t curr_field)//, fieldtypes * m_fieldtypes)
{
  uint16_t curr_value = 0;
  //switch (m_fieldtypes[curr_field])
  switch (fields[curr_field].type)
  {
    case TitleFieldType :
      // nothing?
    break;
    case NumberFieldType :
      curr_value = (uint16_t)fields[curr_field].getValue();
      steps = (fields[curr_field].max - fields[curr_field].min) / 100;
      if(steps == 0) steps = 1;
      maxVal = fields[curr_field].max;
      minVal = fields[curr_field].min;
      curVal = curr_value;
    break;
    case BooleanFieldType :
      curr_value = (uint16_t)fields[curr_field].getValue();
      steps = 1;
      maxVal = 1;
      minVal = 0;
      curVal = curr_value;
    break;
    case SelectFieldType :
      curr_value =(uint16_t)fields[curr_field].getValue();
      steps = 1;
      maxVal = fields[curr_field].max;
      minVal = fields[curr_field].min;
      curVal = curr_value;
    break;
    case ColorFieldType :
      // nothing? - to be done later...
    break;
    case SectionFieldType :
      // nothing?
    break;
    default:
    break;
  }
  return curr_value;
}


void showDisplay(uint8_t curr_field)//, fieldtypes *fieldtype)
{
  static bool toggle;
  EVERY_N_MILLISECONDS(KNOB_CURSOR_BLINK)
  {
    toggle = !toggle;
  }
  EVERY_N_MILLISECONDS(1000/KNOB_DISPLAY_FPS)
  {
    uint16_t val = (uint16_t)fields[curr_field].getValue();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if(mDisplayState) display.displayOn();
    else display.displayOff();
    switch(mDisplayState)
    {
      case Display_Off:
        display.displayOff();
        display_was_off = true;
      break;
      case Display_ShowMenu:
        {
          uint8_t prev_field = get_next_field(curr_field, false);//, fieldtype);
          uint8_t pre_prev_field = get_next_field(prev_field, false);//, fieldtype);
          uint8_t next_field = get_next_field(curr_field, true);//, fieldtype);
          uint8_t next_next_field = get_next_field(next_field, true);//, fieldtype);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0,  0, F("Menu:")); 

          display.setTextAlignment(TEXT_ALIGN_CENTER);
          
          int16_t width = display.getStringWidth(fields[prev_field].label);
          
          if(width > 128) width = width - 128;
          else width = 0;

          if(prev_field != curr_field)
            display.drawString(64+width, 20, fields[prev_field].label);
          
          width = display.getStringWidth(fields[pre_prev_field].label);
          
          if(width > 128) width = width - 128;
          else width = 0;
          
          if(pre_prev_field != prev_field)
            display.drawString(64+width, 10, fields[pre_prev_field].label);
          
          if(toggle)
          {
            display.fillRect(2, 32, 124, 10); 
            display.setColor((OLEDDISPLAY_COLOR)0);
          }
          static bool left = false;
          static int16_t offset = 0;
          width = display.getStringWidth(fields[curr_field].label);
          EVERY_N_MILLISECONDS(30)
          {
            left?offset--:offset++;
          }
          if (width > 128)
          {
            if(left)
            {
              if(offset < 0-(10 + width - 128)) left = false;
            }
            else
            {
              if(offset > (10)) left = true;
            }
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(offset, 30, fields[curr_field].label);
            
          }
          else
          {
            left = true;
            offset = 20;
            display.drawString(64, 30, fields[curr_field].label);
          }
          display.setColor((OLEDDISPLAY_COLOR)1);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          
          width = display.getStringWidth(fields[next_field].label);
          
          if(width > 128) width = width - 128;
          else width = 0;

          if(next_field != curr_field)
            display.drawString(64+width, 40, fields[next_field].label);

          width = display.getStringWidth(fields[next_next_field].label);
          
          if(width > 128) width = width - 128;
          else width = 0;

          if(next_next_field != next_field)
            display.drawString(64+width, 50, fields[next_next_field].label);
        }
      break;
      case Display_ShowBoolMenu:
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawRect(32, 28, 28, 28);
        display.drawRect(68, 28, 28, 28);
        display.setFont(ArialMT_Plain_16);
        if(val)
        {
          display.setColor((OLEDDISPLAY_COLOR)1);
          display.drawString(46, 32, "Off");  
          display.fillRect(68, 28, 28, 28);
          display.setColor((OLEDDISPLAY_COLOR)0);
          display.drawString(82, 32, "On");  
          display.setColor((OLEDDISPLAY_COLOR)1);
        }
        else
        {
          
          display.setColor((OLEDDISPLAY_COLOR)1);
          display.fillRect(32, 28, 28, 28);
          display.setColor((OLEDDISPLAY_COLOR)0);
          display.drawString(46, 32, "Off");  
          display.setColor((OLEDDISPLAY_COLOR)1);
          display.drawString(82, 32, "On");  
          display.setColor((OLEDDISPLAY_COLOR)1);
        }
      break;
      case Display_ShowNumberMenu:
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.setFont(ArialMT_Plain_16);
        display.drawProgressBar(0, 28, 127, 20, map(val, fields[curr_field].min, fields[curr_field].max, (uint16_t)0, (uint16_t)100));
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setColor((OLEDDISPLAY_COLOR)2);
        display.drawString(63, 30, String(fields[curr_field].getValue()));
        display.setColor((OLEDDISPLAY_COLOR)1);
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, 52, String(fields[curr_field].min));
        display.setTextAlignment(TEXT_ALIGN_RIGHT); 
        display.drawString(127, 52, String(fields[curr_field].max));
          
      break;
      case Display_ShowSelectMenue:
      {
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label);
        StaticJsonBuffer<1600> myJsonBuffer;
        
        JsonArray& myValues = myJsonBuffer.createArray();
        fields[curr_field].getOptions(myValues);
        
        //display.drawString(128, 20, myValues[val]);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        uint16_t p_val = 0;
        uint16_t n_val = 0;
        if(val > fields[curr_field].min)
        {
          p_val = val - 1;
        }
        else
        {
          p_val = val; //fields[curr_field].max;
        }
        if(val < fields[curr_field].max)
        {
          n_val = val + 1;
        }
        else
        {
          n_val = val;
        }
        if(val != p_val) {
          display.drawStringMaxWidth(0, 25, 128, F(" -"));
          display.drawStringMaxWidth(10, 25, 128, myValues[p_val]);
        }
        if(val != n_val) {
          display.drawStringMaxWidth(0, 45, 128, F(" +"));
          display.drawStringMaxWidth(10, 45, 128, myValues[n_val]);
        }
        if(toggle)
        {
          display.fillRect(0,36,128,11);
          display.setColor((OLEDDISPLAY_COLOR)0);
        }
        display.drawStringMaxWidth(0, 35, 128, F(" >"));
        display.drawStringMaxWidth(10, 35, 128, myValues[  val]);
        display.setColor((OLEDDISPLAY_COLOR)1); 
      }
      break;
      case Display_ShowSectionMenue:
        display.drawStringMaxWidth(0, 30, 128, fields[curr_field].label);

      break;
      case Display_ShowInfo:
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0,  0, LED_NAME);

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0,  10, F("Brightness:"));
        display.drawString(0,  20, F("Speed:"));
        display.drawString(0,  30, F("Mode:"));
        display.drawString(0,  40, F("Pal:"));
        display.drawString(0,  50, F("mA:"));

        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        //static uint16_t FPS = 0;
        //static uint16_t num_leds_on = strip->getLedsOn(); // fixes issue #18
        /*EVERY_N_MILLISECONDS(200)
        {
          FPS = strip->getFPS();
          //num_leds_on = strip->getLedsOn();
        }
        FastLED.getFPS();
        */
        /*
        if(strip->getPower())
        {        
          display.drawString(127,  20, String(maxHeap)); //String(FPS));
          //display.drawString(127,  30, String(strip->getCurrentPower()/5)); //ESP.getFreeHeap()));
        }
        else
        {
          display.drawString(127,  20, String(maxHeap));// display.drawString(127,  20, F("Off"));
          //display.drawString(127,  30, F("Off"));
        }
        */
        display.drawString(127,  10, String(strip->getTargetBrightness()));
        display.drawString(127,  20, String(strip->getBeat88()));
        display.drawString(127,  30, strip->getPower()?strip->getModeName(strip->getMode()):F("Off"));
        display.drawString(127,  40, strip->getPalName(strip->getTargetPaletteNumber()));
        display.drawString(127,  50, String(strip->getCurrentPower()/5)); 
      default:
        display.displayOn();
      break;
    }
    if(!strip->getWiFiDisabled() && WiFiConnected)
    {
      int32_t rssi = WiFi.RSSI();
      uint8_t bars = 0; //map(rssi, -110, -55, 0, 5);
      if(rssi >= -55)
        bars = 5;
      else if (rssi >= -70)
        bars = 4;
      else if (rssi >= -85)
        bars = 3;
      else if (rssi >= -100)
        bars = 2;
      else if (rssi >= -110)
        bars = 1;
      else
        bars = 0;
      
      for(uint8_t i=0; i<=bars; i++)
      {
        display.drawVerticalLine(115+2*i, 10-i*2, i*2);
      }
    }
    else
    {
      display.drawLine(117, 9, 123, 3);
      display.drawLine(123, 9, 117, 3);
    }
    display.drawHorizontalLine(64-(TimeoutBar/2),63, TimeoutBar);
    display.setColor((OLEDDISPLAY_COLOR)1);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
  }
}

void knob_service(uint32_t now)
{

  static uint8_t curr_field = get_next_field(0, true); 
  static uint32_t last_btn_press = 0;
  
  static uint16_t old_val = 0;

  static bool in_submenu = false;
  static bool newfield_selected = false;
  static bool setnewValue = false;
 
  if (digitalRead(KNOB_C_BTN) == LOW && now > last_btn_press + KNOB_BTN_DEBOUNCE)
  {
    last_btn_press = now;
    
    if(display_was_off)
    {
      display_was_off = false;
      last_control_operation = now - KNOB_TIMEOUT_OPERATION - (KNOB_TIMEOUT_OPERATION/10);
      mDisplayState = Display_ShowInfo;
      showDisplay(curr_field);
      return;
    }
    last_control_operation = now;
    in_submenu = !in_submenu;

    if(!in_submenu)
    {
      maxVal = fieldCount-1;
      minVal = 0;
      steps = 1;
      curVal  = curr_field;
      old_val = curr_field;
    }
    else
    {
      old_val = setEncoderValues(curr_field);
    }
  }
  EVERY_N_MILLISECONDS(KNOB_ROT_DEBOUNCE)
  {  
    uint16_t val = 0;
    int8_t add = myEnc.direction();
    if(add != 0)
    {
      if(display_was_off)
      {
        display_was_off = false;
        last_control_operation = now - KNOB_TIMEOUT_OPERATION - (KNOB_TIMEOUT_OPERATION/10);
        mDisplayState = Display_ShowInfo;
        return;
      }
      last_control_operation = now;
    } 

    if(add < 0 && (curVal <= (minVal + steps)))
    {
      curVal = minVal;
    }
    else if(add > 0 && curVal >= (maxVal-steps))
    {
      curVal = maxVal;
    }
    else
    {
      curVal = curVal + add * steps;
    }
    val = curVal;

    if(old_val != val)
    {
      if(!in_submenu)
      {
        if(val > curr_field)
        {
          val = get_next_field(curr_field, true);
        }
        else
        {
          val = get_next_field(curr_field, false);
        }
        curr_field = val;
        newfield_selected = true;
      }
      else
      {
        if(newfield_selected)
        {
          old_val = setEncoderValues(curr_field);
          newfield_selected = false;
        }
        setnewValue = true;
        
      }
      old_val = val;
    }
    curVal = val;
    EVERY_N_MILLIS(KNOB_ROT_DEBOUNCE)
    {
      if(setnewValue)
      {
        setnewValue = false;
        if(fields[curr_field].setValue)
        {
          fields[curr_field].setValue(val);
        }
      }
    }
    
    if(now > last_control_operation + KNOB_TIMEOUT_DISPLAY)
    {
      mDisplayState = Display_Off;
      display_was_off = true;
    }
    else if(now > last_control_operation + KNOB_TIMEOUT_OPERATION)
    {
      TimeoutBar = map((uint16_t)(now-last_control_operation), (uint16_t)0, (uint16_t)KNOB_TIMEOUT_DISPLAY, (uint16_t)127 ,(uint16_t)0);
      curr_field = get_next_field(0, true);
      in_submenu = true;
      old_val = setEncoderValues(curr_field);
      mDisplayState = Display_ShowInfo;
      if((strip->getAutoplay() || strip->getAutopal()) && strip->getPower()) last_control_operation = now - KNOB_TIMEOUT_OPERATION - (KNOB_TIMEOUT_OPERATION/10); // keeps the display on as long as we change automatically the mode or the palette
    }
    else
    {
      TimeoutBar = map((uint16_t)(now-last_control_operation), (uint16_t)0, (uint16_t)KNOB_TIMEOUT_DISPLAY, (uint16_t)127 ,(uint16_t)0);
      if(!in_submenu)
      {
        mDisplayState = Display_ShowMenu;
      }
      else
      {
        switch (fields[curr_field].type)
        {
          case TitleFieldType : 
          case SectionFieldType :
          case ColorFieldType : 
          mDisplayState = Display_ShowInfo;
          break;
          case NumberFieldType :
          mDisplayState = Display_ShowNumberMenu;
          break;
          case BooleanFieldType :
          mDisplayState = Display_ShowBoolMenu;
          break;
          case SelectFieldType :
          mDisplayState = Display_ShowSelectMenue;
          default:
          break;
        }
      }
    }
    showDisplay(curr_field);
  }
}
#endif


void loop()
{
  uint32_t now = millis();
  static uint32_t wifi_check_time = 0;

  if (OTAisRunning)
    return;

  #ifndef HAS_KNOB_CONTROL
  // Checking WiFi state every WIFI_TIMEOUT
  // Reset on disconnection
  if (now > wifi_check_time)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      wifi_err_counter+=2;
      wifi_disconnect_counter+=4;
    }
    else
    {
      if(wifi_err_counter > 0) wifi_err_counter--;
      if(wifi_disconnect_counter > 0) wifi_disconnect_counter--;
    }

    if(wifi_err_counter > 20)
    {
      delay(2000);
      WiFi.mode(WIFI_OFF);
      delay(2000);
      setupWiFi();
    }

    if(wifi_err_counter > 40)
    {
      for (uint16_t i = 0; i < NUM_INFORMATION_LEDS; i++)
      {
        strip->leds[i] = 0x202000;
      }
      strip->show();
      // Reset after 3 seconds....
      delay(3000);
      ESP.restart();
    }

    wifi_check_time = now + (WIFI_TIMEOUT);
  }

  ArduinoOTA.handle(); // check and handle OTA updates of the code....

  MDNS.update();

  EVERY_N_SECONDS(2)
  {
    DynamicJsonBuffer jB;
    for(const auto& c: webSocketsServer->getClients())
    {
      uint8_t i = getClient(c->id());     
      c->text("{\"Client\": " + String(c->id()) + "}");
      my_pingPongs[i].ping = random8();
      c->ping( &my_pingPongs[i].ping, sizeof(uint8_t));
    }
    webSocketsServer->cleanupClients();
  }

  #else

  static bool WiFiIsDisabled = strip->getWiFiDisabled();
  
  bool wifiDisabled = strip->getWiFiDisabled();
  if(wifiDisabled != WiFiIsDisabled)
  {
    if(!wifiDisabled)
    {
      checkSegmentChanges();
      shouldSaveRuntime = true;
      saveEEPROMData();
      ESP.restart();
    }
    else
    {
      //delete webSocketsServer;
      delay(INITDELAY);
      WiFi.mode(WIFI_OFF);
    }
    WiFiIsDisabled = wifiDisabled;
  }
  
  if(WiFiIsDisabled)
  {
    WiFiConnected = false;
    wifi_check_time = now;
  } 
    
    // Checking WiFi state every WIFI_TIMEOUT
    // since we have Knob-Control. We do not care about "No Connection"
  if (!WiFiIsDisabled && now > wifi_check_time)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      if(WiFiConnected) last_control_operation = now;    // Will switch the display on. Only needed when we had connection and now lose it...
      wifi_err_counter+=1;
      //wifi_disconnect_counter+=2;
      WiFiConnected = false;
    }
    else
    {
      if(wifi_err_counter > 0) wifi_err_counter--;
      // if(wifi_disconnect_counter > 0) wifi_disconnect_counter--;
      // Maybe we implement one line as status message (e.g. "WiFi Reconnected")
      if(!WiFiConnected) last_control_operation = now;  // Will switch the display on (also on reconnection (once)..
      WiFiConnected = true;
    }

    if(wifi_err_counter > (10 * (wifi_disconnect_counter%12)))
    {
      WiFi.mode(WIFI_OFF);
      setupWiFi();
    }
    wifi_check_time = now + (WIFI_TIMEOUT);
  }

  if(!WiFiIsDisabled && WiFiConnected)
  {
    ArduinoOTA.handle(); // check and handle OTA updates of the code....
    MDNS.update();

    EVERY_N_SECONDS(2)
    {
      DynamicJsonBuffer jB;
      for(const auto& c: webSocketsServer->getClients())
      {
        uint8_t i = getClient(c->id());     
        c->text("{\"Client\": " + String(c->id()) + "}");
        my_pingPongs[i].ping = random8();
        c->ping( &my_pingPongs[i].ping, sizeof(uint8_t));
      }
      webSocketsServer->cleanupClients(2);
    }

  }


  #endif

  strip->service();

  EVERY_N_MILLIS(50)
  {
    checkSegmentChanges();
  }

  EVERY_N_MILLIS(EEPROM_SAVE_INTERVAL_MS)
  {
    saveEEPROMData();
    shouldSaveRuntime = false;
  }
  #ifdef HAS_KNOB_CONTROL
  knob_service(now);
  #endif
}
