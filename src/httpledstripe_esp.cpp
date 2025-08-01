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
   - fhem module for this project on https://github.com/tobi01001/FHEM-LED_CONTROL-
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
// Workaround for ESP8266 Arduino core str macro that conflicts with ArduinoJson v6
#ifdef str
#undef str
#endif
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM_Rotate.h>
#include <ESP8266mDNS.h>

#ifdef HAS_KNOB_CONTROL
  
  #include "Encoder.h"
  #include "SSD1306Brzo.h"
  // The Knob Control version allows operation without WiFi
  bool WiFiConnected = false;
#endif

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

EEPROM_Rotate EEPROM;

#include "defaults.h"

extern "C"
{
  #include "user_interface.h"
}

// new approach starts here:
#include "LED_strip/led_strip.h"

#ifdef HAS_KNOB_CONTROL
  // the roating knob controller as input device
  Encoder myEnc(KNOB_C_PNA, KNOB_C_PNB);
  // the I2C connected Display
  SSD1306Brzo display(KNOB_C_I2C, KNOB_C_SDA, KNOB_C_SCL);

  // Holding the different display states to be shown
  enum displayStates {
    Display_Off,
    Display_ShowInfo,
    Display_ShowMenu,
    Display_ShowSectionMenue,
    Display_ShowBoolMenu,
    Display_ShowNumberMenu,
    Display_ShowSelectMenue
  } mDisplayState;

  // timer to store when then Knob was operated last
  // this is used to check when to switch back / off the display
  uint32_t last_control_operation = 0;
  // stores if the display was off
  bool display_was_off = false;
  // lenght of the bar to indicate the timeout on the display
  uint8_t TimeoutBar = 0;

  // Will initialise the display
  void setupKnobControl(void);
  // Will draw the given text line at the given line y
  // returns the next line after drawing
  uint8_t drawtxtline10(uint8_t y, uint8_t fontheight, String txt);
  // will display the "current field" being provided
  void showDisplay(uint8_t curr_field);
  // will handle the encoder value / current field
  uint16_t setEncoderValues(uint8_t curr_field, uint16_t * knb_maxVal,uint16_t * knb_minVal, uint16_t * knb_curVal, uint16_t * knb_steps);
  // service routing for the knob/display control
  void knob_service(uint32_t now);
  // returns the next field in the direction up or down
  uint8_t get_next_field(uint8_t curr_field, bool up);
#endif

#ifdef DEBUG
  #ifdef HAS_KNOB_CONTROL
    // Build version, branch and wheter or not with control Knob (set in platformio.ini)
    const char * build_version PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH "_KNOB"; 
  #else
    // Build version, branch and wheter or not with control Knob (set in platformio.ini)
    const char * build_version PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH;
  #endif
#else
  #ifdef HAS_KNOB_CONTROL
    // Build version, branch and wheter or not with control Knob (set in platformio.ini)
    const char * build_version PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH "_KNOB"; 
  #else
    // Build version, branch and wheter or not with control Knob (set in platformio.ini)
    const char * build_version PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH;
  #endif
#endif

// Git revision being "hardcoded" (extracted befor compilation)
const char * git_revision PROGMEM = BUILD_GITREV;

/* Definitions for network usage */
/* maybe move all wifi stuff to separate files.... */


// the AP-Name if we start in AP_Mode with WiFiManager
const char * AP_SSID PROGMEM = LED_NAME;
// The async web server handling all the web interface stuff 
AsyncWebServer server(80);
// The Websocket part of the web handling
AsyncWebSocket *webSocketsServer;

// we do not want anything to distrub the OTA
// therefore there is a flag which could be used to prevent from that...
// However, seems that a connection being active via websocket interrupts
// even if the web socket server is stopped??
bool OTAisRunning = false;

// Ping-Pong Struct
// holding the iD, current Ping and Current Pong
struct pingPong {
  uint32_t iD = 0;
  uint8_t pong = 0;
  uint8_t ping = 0;
  // Array holding the clients PingPongs
}my_pingPongs[DEFAULT_MAX_WS_CLIENTS+1]; // as Array


/* END Network Definitions */

CRGB pLeds[LED_COUNT_TOT+1]; // 2024-01-24 Try to prevent from sudden restarts but I have no idea of the source...
CRGB eLeds[LED_COUNT];

#include "../lib/FileEditor/FileEditorLittleFS.h"

// helpers
// counts the errors on the wifi connection
uint8_t wifi_err_counter = 0;
// error counter for the wifi disconnects (50 per check up every 5s, 1 down every 30s)
uint16_t wifi_disconnect_counter = 0;
// the adress of the gateway being connected to
IPAddress gateway_ip;
// stats counter (increased as sine function) when stats is called
// can be used as kind of life check
uint8_t status_counter = 0; //((uint8_t)(ESP.getChipId()>>16)) + ((uint8_t)(ESP.getChipId()>>8)) + ((uint8_t)(ESP.getChipId()>>0));

// holds the current reset reason
String cStrReason = "";
// holds the last reset reason (interesting in case of faults / WD resets)
String lStrReason = "";

//flag for saving data 
bool shouldSaveRuntime = false;

bool newSolidColor = false;

// check if we need to Reset
// We had to use a flag as the Async Responce caused
// Watchdog reset.
enum doResets {
  LED_CTRL_NO_RESET,
  LED_CTRL_SAVE_RESTART,
  LED_CTRL_RESET_DEFAULTS,
  LED_CTRL_RESET_FACTORY
} ledCtrlDoResets;

// "local" copy of the segment data (to be saved)
// used for comparison with the actual segment data
WS2812FX::segment seg;

// measures / stores the runtime of the ESP since the last boot
// just because....
struct ESPrunTime {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint16_t days;
}mESPrunTime;


// function Definitions

// will initialise the OTA anf the related callback functions
void initOverTheAirUpdate  (void);
// used to setup the WiFi connection
// this can be called with a specific timeout for the captive portal (defaults to 240 seconds i.e. 4 minutes)
void setupWiFi             (uint16_t timeout);
// initialises the Web sever with the callback / handler functions
void setupWebServer        (void);
// handler for http://..../set calls (usually needs parameter and value)
// will return the parameter and values being set as json
void handleSet             (AsyncWebServerRequest *request);
// not found handler - called when the uri is not managed
void handleNotFound        (AsyncWebServerRequest *request);
// will return all available modes (effects) in an json array
void handleGetModes        (AsyncWebServerRequest *request);
// will return all available color palettes in an json array
void handleGetPals         (AsyncWebServerRequest *request);
// will return the current status, listing all neccessary parameters 
// as well as meta information
void handleStatus          (AsyncWebServerRequest *request);
// when called with the right parameter, it will reset 
// either to default values or perform a factory reset.
// Parameter rst=
//    "FactoryReset" --> Factory Reset
//    "Defaults"     --> Reset to default values (without WiFi settings)
void handleResetRequest    (AsyncWebServerRequest *request);
// broadcasts the name and value to all websocket clients
// name : the Parameter as pointer in Flash (const __FlashStringHelper* )
// value: the parameter value as uint16_t
void broadcastInt          (const __FlashStringHelper* name, uint16_t value);
// handles requests received via web sockets.
// this is currently only used to (de)register new clients
// and to manage the "ping/pong" mechanism checking if WS is alive
void webSocketEvent        (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
// will delete the CRC stored in EEPROM 
// used in case of e.g. WD reset
void clearCRC              (void);
// will delete the complete EEPROM 
// used in case of e.g. factory reset
void clearEEPROM           (void);
// deletes the EEPRORM as wel as WiFi settings
// and restarts fresh
void checkFactoryReset          (void);
// reads the runtime data from EEPROM
// and checks for validity (CRC)
// if CRC does not match it will reset to default values
void readRuntimeDataEEPROM (void);
// shows status information during boot / OTA
// not used when using "Knob control with display"
void showInitColor         (CRGB Color);
// cyclic check for changes to parameters.
// when something is changed, it will enable the
// "saveRuntimeData"-flag in order to store to EEPROM
// at the next EEPROM update time
// it will also broadcast the change to connected WS-Clients
void checkSegmentChanges   (void);
// will write the current data to EEPROM (in case the "saveRuntimeData"-flag is true)
void saveEEPROMData        (void);
// removes the Websocket Client from the list of 
// connected clients
void removeClient          (uint32_t iD);

// will delete the configFile (which gets recreated once /all is called)
void deleteConfigFile      (void);

// will update the file containing all parameters and the structure  (JSON)
// this is used by the website to generate the menu, boundaries etc...
// As the amount of data is too much for the Async-Webserver 
// to manage during runtime, we use a file on the Filesystem for this.
// And it is quite static as long as the structure does not change
void updateConfigFile      (void);

// will return a "relatively" increased or decreased value by the given percentage
// used by the up and down parameters and is there more or less for backward compatibility
uint8_t changebypercentage    (uint8_t value, uint8_t percentage);
// will add the Client with iD to the client list.
// iD: the WS client iD
// return: the number in the list of connected WS
uint8_t addClient             (uint32_t iD);
// will return the list number of Client with iD.
// iD: the WS client iD
// return: the number in the list of connected WS
uint8_t getClient             (uint32_t iD);
// returns the uint8_t equivalent of the 
// reset reason
uint8_t getResetReason        (void);
  
// returns the name ["string" (char* in flash)] of the
// reset reason with "number" resetReason
const __FlashStringHelper * 
    getResetReasonStr(uint8_t resetReason);

// returns the last ResetReason from the file system
String readLastResetReason(void);

// writes the given Reset Reason as "last one"
// to the file system
void writeLastResetReason(const String reason);

// adds entropy to the random16
void addEntropy(void);

void broadcastInt(const __FlashStringHelper* name, uint16_t value)
{
  // if we do have Knob control, we check if WiFi is supposed to be enabled or not.
  // the check if there is a WS server is always done
  #ifdef HAS_KNOB_CONTROL
  if(webSocketsServer == NULL  || strip->getWiFiDisabled() || !WiFiConnected)
  #else
  if(webSocketsServer == NULL)
  #endif
  {
    // nothing to be done -> return
    return;
  }
  // store the name / value pair in a json document 
  DynamicJsonDocument jsonDoc(256);
  jsonDoc[F("name")] = name;
  jsonDoc[F("value")] = value;

  size_t len = measureJson(jsonDoc);
  // create a message buffer, write to it and send it to the WS clients
  AsyncWebSocketMessageBuffer * buffer = webSocketsServer->makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
      serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
      webSocketsServer->textAll(buffer);
  }
}

void broadcastColor(const __FlashStringHelper* name, CRGB Col)
{
  // if we do have Knob control, we check if WiFi is supposed to be enabled or not.
  // the check if there is a WS server is always done
  #ifdef HAS_KNOB_CONTROL
  if(webSocketsServer == NULL  || strip->getWiFiDisabled() || !WiFiConnected)
  #else
  if(webSocketsServer == NULL)
  #endif
  {
    // nothing to be done -> return
    return;
  }
  // store the name / value pair in a json document 
  DynamicJsonDocument jsonDoc(256);
  jsonDoc[F("name")] = name;
  jsonDoc[F("value")] = (((Col.r << 16) | (Col.g << 8) | (Col.b << 0)) & 0xffffff);;

  size_t len = measureJson(jsonDoc);
  // create a message buffer, write to it and send it to the WS clients
  AsyncWebSocketMessageBuffer * buffer = webSocketsServer->makeBuffer(len); //  creates a buffer (len + 1) for you.
  if (buffer) {
      serializeJson(jsonDoc, (char *)buffer->get(), len + 1);
      webSocketsServer->textAll(buffer);
  }
}

void checkSegmentChanges(void) 
{
  // check the segment changes
  // broadcast the change and 
  // set the shouldSaveRuntime = true where applicable
  // ToDo: Check and implement based on the "fields"?
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
    broadcastInt(F("running"), seg.isRunning);
    shouldSaveRuntime = true;
  }
  if(seg.targetBrightness != strip->getTargetBrightness()) {
    seg.targetBrightness = strip->getTargetBrightness();
    broadcastInt(F("brightness"), seg.targetBrightness);
    shouldSaveRuntime = true;
  }
  if(seg.mode != strip->getMode()){
    seg.mode = strip->getMode();
    broadcastInt(F("effect"), seg.mode);
    shouldSaveRuntime = true;
  }
  if(seg.targetPaletteNum != strip->getTargetPaletteNumber()) {
    seg.targetPaletteNum = strip->getTargetPaletteNumber();
    broadcastInt(F("colorPalette"), seg.targetPaletteNum);
    shouldSaveRuntime = true;
  }
  if(seg.beat88 != strip->getBeat88()) {
    seg.beat88 = strip->getBeat88();
    broadcastInt(F("speed"), seg.beat88);
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
    broadcastInt(F("colorTemperature"), strip->getColorTemp());
    shouldSaveRuntime = true;
  }
  if(seg.blur != strip->getBlurValue())
  {
    seg.blur = strip->getBlurValue();
    broadcastInt(F("ledBlur"), seg.blur);
    shouldSaveRuntime = true;
  }
  if(seg.reverse != strip->getReverse())
  {
    seg.reverse = strip->getReverse();
    broadcastInt(F("reversed"), seg.reverse);
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
    broadcastInt(F("mirrored"), seg.mirror);
    shouldSaveRuntime = true;
  }
  if(seg.hueTime != strip->getHueTime())
  {
    seg.hueTime = strip->getHueTime();
    broadcastInt(F("hueTime"), seg.hueTime);
    shouldSaveRuntime = true;
  }
  if(seg.deltaHue != strip->getDeltaHue())
  { 
    seg.deltaHue = strip->getDeltaHue();
    broadcastInt(F("deltaHue"), seg.deltaHue);
    shouldSaveRuntime = true;
  }
  if(seg.autoplay != strip->getAutoplay())
  {
    seg.autoplay = strip->getAutoplay();
    broadcastInt(F("autoPlay"), seg.autoplay);
    shouldSaveRuntime = true;
  }
  if(seg.autoplayDuration != strip->getAutoplayDuration())
  {
    seg.autoplayDuration = strip->getAutoplayDuration();
    broadcastInt(F("autoPlayInterval"), seg.autoplayDuration);
    shouldSaveRuntime = true;
  }
  if(seg.autoPal != strip->getAutopal())
  {
    seg.autoPal = strip->getAutopal();
    broadcastInt(F("autoPalette"), seg.autoPal);
    shouldSaveRuntime = true;
  }
  if(seg.autoPalDuration != strip->getAutopalDuration())
  {
    seg.autoPalDuration = strip->getAutopalDuration();
    broadcastInt(F("autoPalInterval"), seg.autoPalDuration);
    shouldSaveRuntime = true;
  }
    /*
  "solidColor" -> currently not possible to save?
  --> for this to work we need to add 32bit color to the structure 
      and to "set" this including palette creation...
      ...and the palette creation depends on the current palette number?
  */
  if(newSolidColor)
  {
    newSolidColor = false;
    broadcastColor(F("solidColor"), ColorFromPalette(*(strip->getTargetPalette()), 0, 255, NOBLEND));
  }

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
    broadcastInt(F("numEffectBars"), seg.numBars);
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
    broadcastInt(F("currentLimit"), seg.milliamps);
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
    broadcastInt(F("whiteGlitter"), seg.whiteGlitter);
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
    broadcastInt(F("backgroundHue"), seg.backgroundHue);
    shouldSaveRuntime = true;
  }
  if(seg.backgroundSat != strip->getBckndSat())
  {
    seg.backgroundSat = strip->getBckndSat();
    broadcastInt(F("backgroundSat"), seg.backgroundSat);
    shouldSaveRuntime = true;
  }
  if(seg.backgroundBri != strip->getBckndBri())
  {
    seg.backgroundBri = strip->getBckndBri();
    broadcastInt(F("backgroundBri"), seg.backgroundBri);
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

void saveEEPROMData(void)
{
  // write runtime data to EEPROM (when required by "shouldSave Runtime")

  // nothing to do if flag is not set
  if (!shouldSaveRuntime) {
    return;
  }
  
  shouldSaveRuntime = false;

  // update the CRC according to current segment data
  seg.CRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a,(unsigned char *)&seg + 2, sizeof(seg) - 2);
  strip->setCRC(seg.CRC);

  // write the data to the EEPROM
  EEPROM.put(0, seg);
  // as we work on ESP, we also need to commit the written data.
  EEPROM.commit();
}

void readRuntimeDataEEPROM(void)
{

  // reads the stored runtime data from EEPROM
  // must be called after everything else is already setup to be working
  // otherwise this may terribly fail and could override what was read already

  //read the configuration from EEPROM into RAM (copy segment)
  EEPROM.get(0, seg);

  // calculate the CRC over the data being stored in the EEPROM (except the CRC itself)
  uint16_t mCRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a, (unsigned char *)&seg + 2, sizeof(seg) - 2);

  
  // we have a matching CRC, so we update the runtime data.
  if (seg.CRC == mCRC)
  {
    (*strip->getSegment()) = seg;
    
    // Ensure effectSpeeds are initialized for backward compatibility
    // Check if any effectSpeeds are outside valid boundaries
    bool needsInit = false;
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
      if (seg.effectSpeeds[i] < BEAT88_MIN || seg.effectSpeeds[i] > BEAT88_MAX) {
        needsInit = true;
        break;
      }
    }
    
    if (needsInit) {
      // Initialize per-effect speeds with current speed or default
      uint16_t currentSpeed = seg.beat88 > 0 ? seg.beat88 : DEFAULT_SPEED;
      for (uint8_t i = 0; i < MODE_COUNT; i++) {
        seg.effectSpeeds[i] = currentSpeed;
      }
      strip->getSegment()->effectSpeeds[seg.mode] = currentSpeed;
    }
    
    strip->init();
  }
  else // load defaults
  {
    strip->resetDefaults();
  }

  // we clear the local parameters / settings just to 
  // set them again via the "checkSegmentChanges()" functions
  // this will update them to the current ones 
  // and broadcast the current values via websocket
  memset(&seg, 0, sizeof(seg));

  checkSegmentChanges();

  // no need to save right now. next save should be after /set?....
  shouldSaveRuntime = false;
  // init with transition (and effect reset)...
  // this ensures that the LEDs will be black first
  strip->setTransition();
}

void initOverTheAirUpdate(void)
{
  // we distinguish between KNOB_CONTROL and no knob control
  // hte difference is that we either show the progress 
  // - on the display or 
  // - on the leds itself
  #ifdef HAS_KNOB_CONTROL
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // TODO: Implement Hostname in config and WIFI Settings?

  ArduinoOTA.setHostname(LED_NAME); // platformio currently works with IPs but who knows...

  // callback when the OTA starts
  ArduinoOTA.onStart([]() {
    FastLED.clear(true);
    display.clear();
    display.drawString(0, 0, F("Starting OTA..."));
    display.displayOn();
    display.display();
    // delete the configFile - gets recreated on request
    deleteConfigFile();
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();

    EEPROM.rotate(false);
    EEPROM.commit();

    // and we stop the websockets
    if(webSocketsServer)
    {
      webSocketsServer->textAll("OTA started!");
      webSocketsServer->closeAll();
      webSocketsServer->enable(false);
    }
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
        // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(F("OTA success"));
    // and we stop (unmount) the Filesystem again
    LittleFS.end();
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress  on the display
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int prog = (progress / (total / 100));
    display.clear();
    display.drawString(0, 0, F("Starte OTA..."));
    display.drawStringMaxWidth(0, 12, 128, "Prog: " + String(prog) + " % done");
    display.drawProgressBar(1,33, 126, 7, prog);
    display.displayOn();
    display.display();
  });

  // something went wrong, we gonna show an error "message".
  ArduinoOTA.onError([](ota_error_t error) {
    String err = F("OTA Error: ");

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
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(err);
    // and we stop (unmount) the Filesystem again
    LittleFS.end();
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
    // delete the configFile - gets recreated on request
    deleteConfigFile();
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();

    EEPROM.rotate(false);
    EEPROM.commit();

    // and we stop the websockets
    if(webSocketsServer)
    {
      webSocketsServer->textAll(F("OTA started!"));
      webSocketsServer->closeAll();
      webSocketsServer->enable(false);
    }
    // we indicate our skEtch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });
  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    // OTA finished.
    // We fade out the green Leds being activated during OTA.
    bool ledsActive = true;
    
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(F("OTA success"));
    // and we stop (unmount) the Filesystem again
    LittleFS.end();

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
    String err = F("OTA Error: ");

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
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(err);
    // and we stop (unmount) the Filesystem again
    LittleFS.end();

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

void showInitColor(CRGB Color)
{
  // for DEBUG purpose without Serial connection...
#ifdef DEBUG
  Color.r = Color.r&0x20;
  Color.g = Color.g&0x20;
  Color.b = Color.b&0x20;
  fill_solid(strip->leds, NUM_INFORMATION_LEDS, Color);
  strip->show();
  delay(INITDELAY);
#else
  delay(INITDELAY);
#endif

}

/// @brief setup WiFi connection with Wifi Manager...
/// @param timeout in seconds how long the WiFi captive portal should be active (default 240)
void setupWiFi(uint16_t timeout = 240)
{
// setup the Wifi connection with Wifi Manager...

// when wifi is disabled via parameter, then there is nothing to do
// only works with "knob control"
#ifdef HAS_KNOB_CONTROL
  if (strip->getWiFiDisabled())
    return;
#endif

  showInitColor(CRGB::Blue);

  delay(INITDELAY);
  // set the hostname of the ESP8266
  WiFi.hostname(LED_NAME);
  // set the mode to station
  WiFi.mode(WIFI_STA);
  // set the sleep mode to none
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // set the persistent mode to true to not loose the last configuration
  WiFi.persistent(true);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&server, &dns);

// ToDo: Do not see the difference in handling....
// should simplyfy those Knob-Control #ifdefs
#ifndef HAS_KNOB_CONTROL
  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(timeout);

  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

  // tries to connect to last known settings
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration

  if (!wifiManager.autoConnect(AP_SSID))
  {
    showInitColor(CRGB::Yellow);
    delay(6 * INITDELAY);
    showInitColor(CRGB::Red);
    writeLastResetReason(F("WifiManager Timeout"));
    ESP.restart();
  }
  // reset the disconnect Counter
  wifi_disconnect_counter = 0;
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
  }

  WiFi.setAutoReconnect(true);
  // if we get here we have connected to the WiFi

#else // We have a control knob / button

  // If we are in button control mode
  // we only need WiFi for "convenience"
  wifiManager.setConfigPortalTimeout(timeout);

  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

  // tries to connect to last known settings
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(AP_SSID))
  {
    WiFiConnected = false;
  }
  else
  {
    WiFiConnected = true;
    wifi_disconnect_counter = 0; // number of times we (re-)connected // = 0; // reset only in case we actually reconnected via setup routine
    if (WiFi.getMode() != WIFI_STA)
    {
      WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(true);
    // if we get here we have connected to the WiFi
  }
  wifi_err_counter = 0; // need to reset this regardless the connected state. Otherwise we try to reconntect every loop....

#endif
  gateway_ip = WiFi.gatewayIP();
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
}

uint8_t changebypercentage(uint8_t value, uint8_t percentage)
{
  // helper function to change an 8bit value by the given percentage
  // TODO: we could use 8bit fractions for performance
  uint16_t ret = max((value * percentage) / 100, 10);
  if (ret > 255)
    ret = 255;
  return (uint8_t)ret;
}

void handleSet(AsyncWebServerRequest *request)
{
  // if /set was called
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

  // every change (could be more than one) will be 
  // send back in the answer in JSON format
  // first create the response object
  // Buffer size: Support up to 10 parameters being set (10 * 100 chars average = ~1000 bytes)
  AsyncJsonResponse * response = new AsyncJsonResponse(false, 1024);
  
  // create the answer object
  JsonObject answerObj = response->getRoot();
  JsonObject answer = answerObj.createNestedObject(F("currentState"));
  uint32_t color = CRGB::Black;
  for(uint8_t i=0; i<request->params(); i++)
  {
    if(isField(request->getParam(i)->name().c_str()))
    {
      Field f = getField(request->getParam(i)->name().c_str());
      if(f.type == ColorFieldType)
      {
        // comes from the web-Page currently as rgb
        if(request->getParam(i)->value() == "solidColor")
        {
           uint8_t r,g,b;
           r=g=b=0;
           if(request->hasParam("r")) r = constrain((uint8_t)request->getParam("r")->value().toInt(), 0, 0xff);
           if(request->hasParam("g")) g = constrain((uint8_t)request->getParam("g")->value().toInt(), 0, 0xff);
           if(request->hasParam("b")) b = constrain((uint8_t)request->getParam("b")->value().toInt(), 0, 0xff);
           color = (((r << 16) | (g << 8) | (b << 0)) & 0xffffff);
        }
        else
        {
          color = constrain((uint32_t)strtoul(request->getParam(i)->value().c_str(), NULL, 16), 0, 0xffffff);
        }    
        strip->setSolidColor(color);
        strip->setColor(color);
        answer[f.name] = color;
        newSolidColor = true;
      }
      else
      {
        // normal parameter...
        setFieldValue(request->getParam(i)->name().c_str(),(uint16_t)(request->getParam(i)->value().toInt()));
        answer[request->getParam(i)->name()] = getFieldValue(request->getParam(i)->name().c_str());
      }
      
    }
  }
  // a single pixel...
  if(!newSolidColor)
  {
    color = CRGB::Red; 
  }
  
  #if LED_OFFSET > 0
  uint32_t fColor = 0x000000;
  if(request->hasParam(F("fC")))
  {
    fColor = constrain((uint32_t)strtoul(request->getParam(F("fC"))->value().c_str(), NULL, 16), 0, 0xffffff);
  }
  #endif

  // in VOID it should be possible to access the whole strip just including offset.
  // so we write to the physicalleds below
  if (request->hasParam(F("pi")))
  {
    uint16_t pixel = constrain((uint16_t)strtoul(request->getParam(F("pi"))->value().c_str(), NULL, 10), 0, LED_COUNT_TOT - 1);
    // set the VOID directly avoiding the call to "setTransition"
    // which would clear the (currently written data first
    strip->getSegment()->mode = FX_MODE_VOID;
    strip->setPower(true);
    pLeds[pixel] = CRGB(color);
    // a range of pixels from start rnS to end rnE
    answer[F("pixel")] = pixel;
    char col[10];
    sprintf(col, "%06X", color);    
    answer[F("color")] = col;
    answer[F("effect")] = strip->getModeName(FX_MODE_VOID);
    answer[F("power")] = strip->getPower() ? F("on") : F("off");
  }
  //FIXME: Does not yet work. Lets simplyfy all of this!
  else if (request->hasParam(F("rnS")) && request->hasParam(F("rnE")))
  {
    uint16_t start = constrain((uint16_t)strtoul(request->getParam(F("rnS"))->value().c_str(), NULL, 10), 0, LED_COUNT_TOT - 1);
    uint16_t end = constrain((uint16_t)strtoul(request->getParam(F("rnE"))->value().c_str(), NULL, 10), start, LED_COUNT_TOT - 1);

    // set the VOID directly avoiding the call to "setTransition"
    // which would clear the (currently written data first
    strip->getSegment()->mode = FX_MODE_VOID;
    strip->setPower(true);
    for (uint16_t i = start; i <= end; i++)
    {
      pLeds[i] = CRGB(color);
    }
    answer[F("rnS")] = start;
    answer[F("rnE")] = end;
    char col[10];
    sprintf(col, "%06X", color);    
    answer[F("color")] = col;
    answer[F("effect")] = strip->getModeName(FX_MODE_VOID);
    answer[F("power")] = strip->getPower() ? F("on") : F("off");
    // one color for the complete strip
  }
  else if (request->hasParam(F("rgb")))
  {
    strip->setColor(color);
    strip->setPower(true);
    strip->setMode(FX_MODE_STATIC);
    // finally set a new color
    char col[10];
    sprintf(col, "%06X", color);    
    answer[F("color")] = col;
    answer[F("effect")] = strip->getModeName(FX_MODE_STATIC);
    answer[F("power")] = strip->getPower() ? F("on") : F("off");
  }
  // if we work with offset (i.e. dead pixel at the beginning), we can use this for setting pixels without affecting the effects at all
  // this is the first try for that.
  // 
  #if LED_OFFSET > 0
  else if (request->hasParam(F("fpi")))
  {
    uint16_t pixel = constrain((uint16_t)strtoul(request->getParam(F("fpi"))->value().c_str(), NULL, 10), 0, LED_OFFSET - 1);
    pLeds[pixel] = CRGB(fColor);
    // a single pixel at fpi
    answer[F("fpi")] = pixel;
    char col[10];
    sprintf(col, "%06X", fColor);    
    answer[F("fColor")] = col;
    answer[F("power")] = strip->getPower() ? F("on") : F("off");
  }
  else if (request->hasParam(F("fS")) && request->hasParam(F("fE")))
  {
    uint16_t start = constrain((uint16_t)strtoul(request->getParam(F("fS"))->value().c_str(), NULL, 10), 0, LED_OFFSET - 1);
    uint16_t end = constrain((uint16_t)strtoul(request->getParam(F("fE"))->value().c_str(), NULL, 10), start, LED_OFFSET - 1);
    for (uint16_t i = start; i <= end; i++)
    {
      pLeds[i] = CRGB(fColor);
    }
    answer[F("fS")] = start;
    answer[F("fE")] = end;
    char col[10];
    sprintf(col, "%06X", fColor);    
    answer[F("color")] = col;
    answer[F("power")] = strip->getPower() ? F("on") : F("off");
    // one color for the complete strip
  }
  #endif
  response->setLength();
  request->send(response);
}

void handleNotFound(AsyncWebServerRequest * request)
{
  // if something unknown was called...
  AsyncWebServerResponse *response = request->beginResponse(404); //Sends 404 File Not Found
  response->addHeader(F("Server"),LED_NAME);
  request->send(response);
}

void handleGetModes(AsyncWebServerRequest *request)
{
  // will return all available effects in JSON as name, number 
  AsyncJsonResponse * response = new AsyncJsonResponse(false, 2048);

  JsonObject root = response->getRoot();

  JsonObject modeinfo = root.createNestedObject(F("modeinfo"));
  modeinfo[F("count")] = strip->getModeCount();

  JsonObject modeinfo_modes = modeinfo.createNestedObject(F("modes"));
  for (uint8_t i = 0; i < strip->getModeCount(); i++)
  {
    modeinfo_modes[strip->getModeName(i)] = i;
  }
  response->setLength();
  request->send(response);
}

void handleGetPals(AsyncWebServerRequest *request)
{
  // will return all available Color palettes in JSON as name, number 

  // Buffer size: 2048 bytes
  AsyncJsonResponse * response = new AsyncJsonResponse(false, 2048);

  JsonObject root = response->getRoot();

  JsonObject modeinfo = root.createNestedObject(F("palinfo"));
  modeinfo[F("count")] = strip->getPalCount();

  JsonObject modeinfo_modes = modeinfo.createNestedObject(F("pals"));
  for (uint8_t i = 0; i < strip->getPalCount(); i++)
  {
    modeinfo_modes[strip->getPalName(i)] = i;
  }
  response->setLength();
  request->send(response);
}

void handleStatus(AsyncWebServerRequest *request)
{
  // collects the current status and returns that
  AsyncJsonResponse * response = new AsyncJsonResponse(false, 2048);


  JsonObject answerObj = response->getRoot();
  JsonObject currentStateAnswer = answerObj.createNestedObject(F("currentState"));
  JsonObject sunriseAnswer = answerObj.createNestedObject(F("sunRiseState"));
  JsonObject statsAnswer = answerObj.createNestedObject(F("Stats"));

  uint16_t num_leds_on = strip->getLedsOn();

  if(!getAllValuesJSON(currentStateAnswer))
  {
    currentStateAnswer[F("ValueError")] = F("Could not read any Name Value Pair!");
  }
  currentStateAnswer[F("buildVersion")]     = build_version;
  currentStateAnswer[F("gitRevision")]      = git_revision;
  currentStateAnswer[F("lampName")]         = LED_NAME;
  currentStateAnswer[F("ledCount")]         = LED_COUNT;
  currentStateAnswer[F("lampMaxCurrent")]   = strip->getMilliamps();
  currentStateAnswer[F("lampMaxPower")]     = strip->getVoltage() * strip->getMilliamps();
  currentStateAnswer[F("lampCurrentPower")] = strip->getCurrentPower();
  currentStateAnswer[F("ledsOn")]           = num_leds_on;
  CRGB col = CRGB::Black;
  // We return either black (strip effectively off)
  // or the color of the first lid pixel....
  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    if (strip->_bleds[i])
    {
      col = strip->_bleds[i];
      break;
    }
  }

  currentStateAnswer[F("rgb")]        = (((col.r << 16) | (col.g << 8) | (col.b << 0)) & 0xffffff);
  currentStateAnswer[F("rgb_red")]    = col.r;
  currentStateAnswer[F("rgb_green")]  = col.g;
  currentStateAnswer[F("rgb_blue")]   = col.b;

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
  
  if(num_leds_on && (strip->getMode() == FX_MODE_SUNRISE || strip->getMode() == FX_MODE_SUNSET))
  {
    sunriseAnswer[F("sunRiseActive")]     = F("on");
  }
  else
  {
    sunriseAnswer[F("sunRiseActive")]       = F("off");
  }
  sunriseAnswer[F("sunRiseCurrStep")]       = strip->getCurrentSunriseStep();
  sunriseAnswer[F("sunRiseTotalSteps")]     = DEFAULT_SUNRISE_STEPS;
  sunriseAnswer[F("sunRiseTimeToFinish")]   = strip->getSunriseTimeToFinish();
  sunriseAnswer[F("sunRiseTime")]           = strip->getSunriseTime();
  statsAnswer[F("chip_ResetReason")]        = cStrReason;
  statsAnswer[F("chip_LastResetReason")]    = lStrReason; 
  uint32_t free = 0;
  uint16_t max  = 0;
  uint8_t  frag = 0;
  ESP.getHeapStats(&free, &max, &frag);
  statsAnswer[F("chip_FreeHeap")]           = free;
  statsAnswer[F("chip_MaxHeap")]            = max;
  statsAnswer[F("chip_HeapFrag")]           = frag;
  ESP.resetFreeContStack();
  statsAnswer[F("chip_FreeStack")]          = ESP.getFreeContStack();
  statsAnswer[F("chip_ID")]                 = ESP.getChipId();
  statsAnswer[F("wifi_IP")]                 = WiFi.localIP().toString();
  statsAnswer[F("wifi_CONNECT_ERR_COUNT")]  = wifi_disconnect_counter;
  statsAnswer[F("wifi_SIGNAL")]             = WiFi.RSSI();  // for #14
  statsAnswer[F("wifi_CHAN")]               = WiFi.channel();  // for #14
  statsAnswer[F("wifi_GATEWAY")]            = gateway_ip.toString();
  statsAnswer[F("wifi_BSSID")]              = WiFi.BSSIDstr();
  statsAnswer[F("wifi_BSSIDCRC")]           = strip->calc_CRC16((unsigned int)0x5555, (unsigned char*)WiFi.BSSID(), 6);
  statsAnswer[F("statsCounter")]            = sin8(status_counter++);
  statsAnswer[F("fps_FastLED")]             = FastLED.getFPS();
  statsAnswer[F("esp_Runtime_Days")]        = mESPrunTime.days;
  statsAnswer[F("esp_Runtime_Hours")]       = mESPrunTime.hours;
  statsAnswer[F("esp_Runtime_Minutes")]     = mESPrunTime.minutes;
  statsAnswer[F("esp_Runtime_Seconds")]     = mESPrunTime.seconds;

  response->setLength();
  request->send(response);
}

void checkFactoryReset()
{
  switch (ledCtrlDoResets)
  {
    case LED_CTRL_NO_RESET:
      break;
    case LED_CTRL_SAVE_RESTART:
    {
      shouldSaveRuntime = true;
      saveEEPROMData();
      EEPROM.end();
      ESP.restart();
      break;
    }
    case LED_CTRL_RESET_DEFAULTS:
    {
      strip->setTargetPalette(0);
      strip->setMode(0);
      strip->stop();
      strip->resetDefaults();

      deleteConfigFile();

      delay(INITDELAY);
      checkSegmentChanges();
      delay(INITDELAY);
      shouldSaveRuntime = true;
      delay(INITDELAY);
      saveEEPROMData();
      delay(INITDELAY);
      writeLastResetReason(F("Reset Default Values"));
      ESP.restart();
      break;
    }
      
    case LED_CTRL_RESET_FACTORY:
    {
      // on factory reset, each led will be red
      // increasing from led 0 to max.
      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
        // if we call strip->show(); then we can write data to leds array.
        // if FastLED.show() would have been directly called, we need tov write to the raw data in e.g. _bleds
        strip->leds[i] = 0xa00000;
        strip->show();
        delay(5);
      }

      deleteConfigFile();

      DNSServer dns;
      AsyncWiFiManager wifimgr(&server, &dns);
      delay(INITDELAY);
      wifimgr.resetSettings();
      delay(INITDELAY);
      clearEEPROM();
      WiFi.persistent(false); 
      writeLastResetReason(F("Factory Reset"));
      ESP.restart();
      break;
    }
    default:
      break;
  }
  ledCtrlDoResets = LED_CTRL_NO_RESET;
}

const __FlashStringHelper * getResetReasonStr(uint8_t resetReason)
{
  switch (resetReason)
  {
  case REASON_DEFAULT_RST:  
    return(F("Normal Boot"));
    break;
  case REASON_WDT_RST:
    return(F("WDT Reset"));
    break;
  case REASON_EXCEPTION_RST:
    return(F("Exception"));
    break;
  case REASON_SOFT_WDT_RST:
    return(F("Soft WDT Reset"));
    break;
  case REASON_SOFT_RESTART:
    return(F("Restart"));
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    return(F("Sleep Awake"));
    break;
  case REASON_EXT_SYS_RST:
    return(F("External Trigger"));
    break;

  default:
    return(F("Unknown Cause"));
    break;
  }
  return(F("Unknown Cause"));
}

void clearCRC(void)
{  
/*
 * Clear the CRC to startup Fresh....
 * Used in case we end up in a WDT reset (either SW or HW)
 */
  // we will reset anyway... so lets stop the server
  server.end();
  if(webSocketsServer)
  {
    webSocketsServer->enable(false);
  }
  deleteConfigFile();
// invalidating the CRC - in case somthing goes terribly wrong...
  EEPROM.begin(strip->getCRCsize());
  EEPROM.put(0,(uint16_t)0);
  EEPROM.commit();
  EEPROM.end();
  delay(100);
}

void clearEEPROM(void)
{
  deleteConfigFile();
  //Clearing EEPROM
  EEPROM.begin(strip->getSegmentSize());
  for (uint32_t i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}

void handleResetRequest(AsyncWebServerRequest * request)
{
  if(request->hasParam(F("rst")))
  {
    if ((request->getParam(F("rst"))->value() == F("Restart")))
    {
      request->redirect("/");
      //request->send(200, F("text/plain"), F("Will save EEPROM Data and restart..."));
      ledCtrlDoResets = LED_CTRL_SAVE_RESTART;
      return;
    }
    else if (request->getParam(F("rst"))->value() == F("FactoryReset"))
    {
      request->redirect("/");
      //request->send(200, F("text/plain"), F("Will now Reset to factory settings. You need to connect to the WLAN AP afterwards...."));
      ledCtrlDoResets = LED_CTRL_RESET_FACTORY;
      return;
    }
    else if (request->getParam(F("rst"))->value() == F("Defaults"))
    {
      request->redirect("/");
      //request->send(200, F("text/plain"), F("Strip was reset to the default values..."));
      ledCtrlDoResets = LED_CTRL_RESET_DEFAULTS;
      return;
    }
  }
  request->send(200, F("text/plain"), F("/reset needs a Parameter:\n\t - Restart (Will save EEPROM and restart the ESP)\n\t - Defaults (Will reset to default values and restart the ESP)\n\t - FactoryReset (Will reset everything to Factory values)"));
}


void deleteConfigFile(void)
{
  if(LittleFS.exists(F("/config_all.json")))
  {
    LittleFS.remove(F("/config_all.json"));
  }
}

void updateConfigFile(void)
{
  // Buffer size: 8192 bytes to not stress dynamic memory too much
  DynamicJsonDocument doc(8192);
  JsonArray root = doc.to<JsonArray>();
  
  getAllJSON(root);

  File config_Json = LittleFS.open(F("/config_all.json"), "w");

  serializeJson(root, config_Json);

  config_Json.close();
}

void setupWebServer(void)
{
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  

  server.on(("/all"), HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!LittleFS.exists(F("/config_all.json")))
    {
      updateConfigFile();
    }
    request->send(LittleFS, F("/config_all.json"), F("application/json"));
  });


  server.on("/allvalues", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Buffer size: 2048 bytes works great
    AsyncJsonResponse * response = new AsyncJsonResponse(false, 2048);

    JsonObject root = response->getRoot();
    JsonArray arr = root.createNestedArray(F("values"));
    if(!getAllValuesJSONArray(arr))
    {
      JsonObject obj = arr.createNestedObject();
      obj[F("ValueError")] = F("Did not read any values!");
    }
    response->setLength();
    request->send(response);
  });

  server.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    String name = request->getParam(F("name"))->value();
    String response = getFieldValue(name.c_str());
    request->send(200, F("text/plain"), response);
  });

  
  // keepAlive
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, F("text/plain"), F("Pong"));
  });

  server.on("/set", handleSet);
  server.on("/getmodes", handleGetModes);
  server.on("/getpals", handleGetPals);
  server.on("/status", handleStatus);
  server.on("/reset", handleResetRequest);
  
  // Serve index.htm as the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, F("/index.htm"), F("text/html"));
  });
  
  //server.onNotFound(handleNotFound);
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  /* should work without as we serve static below
  server.on("/", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    request->send(200,  F("text/plain"), "" );
  });
  */

  server.addHandler(new FileEditorLittleFS(LittleFS, String(), String()));

  server.serveStatic("/", LittleFS, "/").setCacheControl("max-age=60");
  delay(INITDELAY);

  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Max-Age"), "1");
  #ifndef DO_NOT_USE_WEBSOCKET
  if(webSocketsServer == NULL) webSocketsServer = new AsyncWebSocket("/ws");
  #endif
  if(webSocketsServer)
  {
    webSocketsServer->enable(true);
  
    webSocketsServer->onEvent(webSocketEvent);

    server.addHandler(webSocketsServer);
  }

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

uint8_t addClient(uint32_t iD)
{
  if(!webSocketsServer) return 0;
  // add a client to the pingpong list
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

uint8_t getClient(uint32_t iD)
{
  // get a client number based on the id.
  for(uint8_t i=0; i<DEFAULT_MAX_WS_CLIENTS; i++)
  {
    if(my_pingPongs[i].iD == iD)
    {
      return i;
    }
  }
  return DEFAULT_MAX_WS_CLIENTS;
}

void removeClient(uint32_t iD)
{
  // remove a client from the ping pong list
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

void webSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  // TODO: Make something useful with the Websocket Event
  #ifdef HAS_KNOB_CONTROL
  if(strip->getWiFiDisabled()) return;
  #endif
  
  if(!webSocketsServer) return;

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

uint8_t get_next_field(uint8_t curr_field, bool up) 
{
  const uint8_t fieldCount = getFieldCount();
  const Field * fields = getFields();
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
        ret = first_field; 
      } 
    }
  }
  return ret;
}

uint16_t setEncoderValues(uint8_t curr_field, uint16_t * knb_maxVal,uint16_t * knb_minVal, uint16_t * knb_curVal, uint16_t * knb_steps)
{
  const Field * fields = getFields();
  uint16_t curr_value = 0;
  //switch (m_fieldtypes[curr_field])
  switch (fields[curr_field].type)
  {
    case TitleFieldType :
      // nothing?
    break;
    case NumberFieldType :
      curr_value = (uint16_t)fields[curr_field].getValue();
      *knb_steps = (fields[curr_field].max - fields[curr_field].min) / 100;
      if(*knb_steps == 0) *knb_steps = 1;
      *knb_maxVal = fields[curr_field].max;
      *knb_minVal = fields[curr_field].min;
      *knb_curVal = curr_value;
    break;
    case BooleanFieldType :
      curr_value = (uint16_t)fields[curr_field].getValue();
      *knb_steps = 1;
      *knb_maxVal = 1;
      *knb_minVal = 0;
      *knb_curVal = curr_value;
    break;
    case SelectFieldType :
      curr_value =(uint16_t)fields[curr_field].getValue();
      *knb_steps = 1;
      *knb_maxVal = fields[curr_field].max;
      *knb_minVal = fields[curr_field].min;
      *knb_curVal = curr_value;
    break;
    case ColorFieldType :
      // nothing? - to be done later...
      // we could certainly add some preselected vbalues as list???
    break;
    case SectionFieldType :
      // nothing?
    break;
    default:
    break;
  }
  return curr_value;
}

void showDisplay(uint8_t curr_field)
{
  static bool toggle;
  EVERY_N_MILLISECONDS(KNOB_CURSOR_BLINK)
  {
    toggle = !toggle;
  }
  EVERY_N_MILLISECONDS(1000/KNOB_DISPLAY_FPS)
  {
    const Field * fields = getFields();
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
        // Buffer size: 2048 bytes 
        StaticJsonDocument<2048> myJsonBuffer;
        
        JsonArray myValues = myJsonBuffer.to<JsonArray>();
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
  // ToDo: Use a unique and better name. 
  // Maybe make this local within the functions?
  const uint8_t fieldCount  = getFieldCount();
  const Field * fields      = getFields();
  static uint16_t knb_maxVal = 65535;
  static uint16_t knb_minVal = 0;
  static uint16_t knb_curVal = 0;
  static uint16_t knb_steps  = 1;

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
      knb_maxVal = fieldCount-1;
      knb_minVal = 0;
      knb_steps = 1;
      knb_curVal  = curr_field;
      old_val = curr_field;
    }
    else
    {
      old_val = setEncoderValues(curr_field, &knb_maxVal, &knb_minVal, &knb_curVal, &knb_steps);
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

    if(add < 0 && (knb_curVal <= (knb_minVal + knb_steps)))
    {
      knb_curVal = knb_minVal;
    }
    else if(add > 0 && knb_curVal >= (knb_maxVal-knb_steps))
    {
      knb_curVal = knb_maxVal;
    }
    else
    {
      knb_curVal = knb_curVal + add * knb_steps;
    }
    val = knb_curVal;

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
          old_val = setEncoderValues(curr_field, &knb_maxVal, &knb_minVal, &knb_curVal, &knb_steps);
          newfield_selected = false;
        }
        setnewValue = true;
        
      }
      old_val = val;
    }
    knb_curVal = val;
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
      old_val = setEncoderValues(curr_field, &knb_maxVal, &knb_minVal, &knb_curVal, &knb_steps);
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


String readLastResetReason(void)
{
  File f = LittleFS.open(F("/lastReset.txt"), "r");
  if(!f) return F("FS upload? File not found");
  String r = f.readStringUntil((char)13);
  f.close();
  return r;
}

void addEntropy(void)
{
  random16_add_entropy(ESP.getChipId());
  random16_add_entropy(esp_get_cycle_count());
  random16_add_entropy(ESP.getFreeHeap());
  random16_add_entropy(WiFi.RSSI());
  random16_add_entropy(analogRead(PIN_A0));
}

void writeLastResetReason(const String reason)
{
  File f = LittleFS.open(F("/lastReset.txt"), "w");
  addEntropy();
  if(!f) return;
  f.println(reason + " " + String(random16()));
  f.close();
}

void setup()
{
  // setup network and output pins

  // Sanity delay to get everything settled....
  delay(INITDELAY);

  // only used to add entropy to the random numbers
  pinMode(A0, INPUT);

  addEntropy();

  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  #endif
  // init some values
  ledCtrlDoResets = LED_CTRL_NO_RESET;
  mESPrunTime.days    = 0;
  mESPrunTime.hours   = 0;
  mESPrunTime.minutes = 0;
  mESPrunTime.seconds = 0;

  LittleFS.begin();
  
  EEPROM.begin(strip->getSegmentSize());

  cStrReason = ESP.getResetInfo();
  lStrReason = readLastResetReason();
  writeLastResetReason(cStrReason);
  
  #ifdef HAS_KNOB_CONTROL
  const uint8_t font_height = 12;

  setupKnobControl();

  uint8_t cursor = 0;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  cursor = drawtxtline10(cursor, font_height, F("Booting... Bitte Warten"));
  display.display();
  

  mDisplayState = Display_ShowInfo;
  cursor = drawtxtline10(cursor, font_height, cStrReason);
  display.display();
  delay(200);
  switch (ESP.getResetInfoPtr()->reason)
  {
  case REASON_DEFAULT_RST:
    break;
  case REASON_WDT_RST:
    delay(2000);
    clearCRC(); // should enable default start in case of
    ESP.restart();
    break;
  case REASON_EXCEPTION_RST:
    delay(2000);
    clearCRC();
    ESP.restart();
    break;
  case REASON_SOFT_WDT_RST:
    delay(2000);
    clearCRC();
    ESP.restart();
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
  cursor = drawtxtline10 (cursor, 10, F("LED Stripe init"));
  display.display();

  FastLED.addLeds<WS2812, LED_PIN, GRB>(pLeds, LED_COUNT_TOT);  

  stripe_setup(pLeds, eLeds);

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

  delay(KNOB_BOOT_DELAY);
  display.clear();
  cursor = 0;
  cursor = drawtxtline10(cursor, font_height, F("System ready!"));
  cursor = drawtxtline10(cursor, font_height, "Name: " + String(F(LED_NAME)));
  cursor = drawtxtline10(cursor, font_height, "IP: " + WiFi.localIP().toString());
  cursor = drawtxtline10(cursor, font_height, "LEDs: " + String(LED_COUNT));
  display.display();
  delay(KNOB_BOOT_DELAY);

  display.clear();  
  
  #else // HAS_KNOB_CONTROL


  switch (ESP.getResetInfoPtr()->reason)
  {
    case REASON_DEFAULT_RST:
      break;
    case REASON_WDT_RST:
      clearCRC(); // should enable default start in case of

      ESP.restart();
      break;
    case REASON_EXCEPTION_RST:
      clearCRC();
      ESP.restart();
      break;
    case REASON_SOFT_WDT_RST:
      
      clearCRC();
      ESP.restart();
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
  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(pLeds, LED_COUNT_TOT);
  FastLED.setBrightness(255);
 
  stripe_setup(pLeds, eLeds);


  // internal LED can be light up when current is limited by FastLED
  
  pinMode(2, OUTPUT);


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


#endif // HAS_KNOB_CONTROL
}

void loop()
{
  uint32_t now = millis();
  static uint32_t wifi_check_time = 0;
  bool WLAN_Connected = (WiFi.status() == WL_CONNECTED);

  if (OTAisRunning)
    return;

  #ifndef HAS_KNOB_CONTROL
  // Checking WiFi state every WIFI_TIMEOUT
  // Reset on disconnection
  if (now > wifi_check_time)
  {
    if (!WLAN_Connected)
    {
      server.end();
      if(webSocketsServer) webSocketsServer->enable(false);
      wifi_err_counter+=2;
      wifi_disconnect_counter+=50; 
    }
    else
    {
      server.begin();
      if(webSocketsServer) webSocketsServer->enable(true);
      if(wifi_err_counter > 0) wifi_err_counter--;
      static uint8_t resetCnt = 0;
      resetCnt = (resetCnt + 1)%6;
      broadcastInt(F("resetCnt"), resetCnt);
      broadcastInt(F("wifiErrCnt"), wifi_disconnect_counter);
      if(wifi_disconnect_counter > 0 && !resetCnt) wifi_disconnect_counter--;
    }

    if(wifi_err_counter > 20)
    {
      delay(2000);
      WiFi.mode(WIFI_OFF);
      delay(2000);
      setupWiFi();

    }
    // restart if there is no WiFi for more than one minute
    // ToDo: check if we make this an increasing value (to end in restarts every minute....) 
    // but the reconnect currently should also wait in the config portal?!
    // also to be checked, why we do restart here while doing a new "setup" with the Knob Control?
    if(wifi_err_counter > (uint8_t)(60000/WIFI_TIMEOUT))
    {
      for (uint16_t i = 0; i < NUM_INFORMATION_LEDS; i++)
      {
        strip->leds[i] = 0x202000;
      }
      strip->show();
      // Reset after 3 seconds....
      delay(3000);
      writeLastResetReason(F("WiFi disconnect Timeout"));
      ESP.restart();
    }

    wifi_check_time = now + (WIFI_TIMEOUT);
  }

  if(WLAN_Connected)
  {
    ArduinoOTA.handle(); // check and handle OTA updates of the code....

    MDNS.update();

    EVERY_N_SECONDS(2)
    {
      if(webSocketsServer)
      {
        for(const auto& c: webSocketsServer->getClients())
        {
          uint8_t i = getClient(c->id());     
          c->text("{\"Client\": " + String(c->id()) + "}");
          my_pingPongs[i].ping = random8();
          c->ping( &my_pingPongs[i].ping, sizeof(uint8_t));
        }
        webSocketsServer->cleanupClients();
      }
    }
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
      writeLastResetReason(F("WiFi disabled toggle"));
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
    if (!WLAN_Connected)
    {
      server.end();
      if(webSocketsServer) webSocketsServer->enable(false);
      if(WiFiConnected) last_control_operation = now;    // Will switch the display on. Only needed when we had connection and now lose it...
      wifi_err_counter+=1;
      wifi_disconnect_counter+=50;
      WiFiConnected = false;
    }
    else
    {
      server.begin();
      if(webSocketsServer) webSocketsServer->enable(true);
      static uint8_t resetCnt = 0;
      resetCnt = (resetCnt + 1)%6;
      if(wifi_err_counter > 0) wifi_err_counter--;
      if(wifi_disconnect_counter > 0 && !resetCnt) wifi_disconnect_counter--;
      // Maybe we implement one line as status message (e.g. "WiFi Reconnected")
      if(!WiFiConnected) last_control_operation = now;  // Will switch the display on (also on reconnection (once)..
      WiFiConnected = true;
    }

    if(wifi_err_counter > (uint8_t)(60000/WIFI_TIMEOUT)) 
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
      if(webSocketsServer)
      {
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
  }


  #endif

  strip->service();

  if(WLAN_Connected)
  {
    EVERY_N_MILLIS(100)
    {
      checkSegmentChanges();
      checkFactoryReset();
    }
  }

  EVERY_N_MILLIS(EEPROM_SAVE_INTERVAL_MS)
  {
    saveEEPROMData();
    shouldSaveRuntime = false;
  }
  #ifdef HAS_KNOB_CONTROL
  knob_service(now);
  #endif
  EVERY_N_MILLISECONDS(1000)
  {
    // could have been done with mills directly and just counting the "wrapover"
    // but this is a bit more "readable" for the outside world
    if(++mESPrunTime.seconds == 60)
    {
      mESPrunTime.seconds = 0;
      if(++mESPrunTime.minutes == 60)
      {
        mESPrunTime.minutes = 0;
        if(++mESPrunTime.hours == 24)
        {
          mESPrunTime.hours = 0;
          mESPrunTime.days++;  
          // ToDo: What do we do if the days will wrap over 
          // to zero after >170 Years :-)
        }
      }
    }
  }
}