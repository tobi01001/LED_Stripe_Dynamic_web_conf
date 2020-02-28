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
#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

#ifdef HAS_KNOB_CONTROL
  
  #include "Encoder.h"
  #include "SSD1306Brzo.h"
  bool WiFiConnected = true;
#endif

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

#include "debug_help.h"

#include "defaults.h"

extern "C"
{
#include "user_interface.h"
}

// new approach starts here:
#include "led_strip.h"

#ifdef HAS_KNOB_CONTROL

Encoder myEnc(KNOB_C_PNA, KNOB_C_PNB);

//SSD1306Brzo display(KNOB_C_I2C, KNOB_C_SDA, KNOB_C_SCL);
SSD1306Brzo display(0x3c, 4, 5);




#endif


#ifdef DEBUG
const String build_version = (String(BUILD_VERSION) + String("DEBUG ") + String(__TIMESTAMP__);
#else
#ifdef HAS_KNOB_CONTROL
const String build_version = (String(BUILD_VERSION) + String("_KNOB")); // + String(__TIMESTAMP__);
#else
const String build_version = (String(BUILD_VERSION));; // + String(__TIMESTAMP__);
#endif
#endif
const String git_revision  = BUILD_GITREV;

/* Definitions for network usage */
/* maybe move all wifi stuff to separate files.... */

ESP8266WebServer server(80);
WebSocketsServer *webSocketsServer = NULL; // webSocketsServer = WebSocketsServer(81);

const String AP_SSID = LED_NAME + String("-") + String(ESP.getChipId());

IPAddress myIP;

/* END Network Definitions */

// helpers
uint8_t wifi_err_counter = 0;
uint16_t wifi_disconnect_counter = 0;

//flag for saving data
bool shouldSaveRuntime = false;

WS2812FX::segment seg;

StaticJsonBuffer<3000> jsonBuffer;

#include "FSBrowser.h"

// function Definitions
void saveEEPROMData(void),
    initOverTheAirUpdate(void),
    setupWiFi(void),
    handleSet(void),
    handleNotFound(void),
    handleGetModes(void),
    handleStatus(void),
    factoryReset(void),
    handleResetRequest(void),
    setupWebServer(void),
    showInitColor(CRGB Color),
    sendInt(String name, uint16_t value),
    sendString(String name, String value),
    sendAnswer(String jsonAnswer),
    broadcastInt(String name, uint16_t value),
    broadcastString(String name, String value),
    webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length),
    checkSegmentChanges(void),
    clearEEPROM(void);

const String
pals_setup(void);

uint32
getResetReason(void);

#ifdef HAS_KNOB_CONTROL

enum fieldtypes {
    NumberFieldType,
    BooleanFieldType,
    SelectFieldType,
    ColorFieldType,
    TitleFieldType,
    SectionFieldType,
    InvalidFieldType
  };


fieldtypes *m_fieldtypes;
  
void set_fieldTypes(fieldtypes *m_fieldtypes) {
  if(!m_fieldtypes) return;
  for(uint8_t i=0; i<fieldCount; i++)
  {
    m_fieldtypes[i] = InvalidFieldType;
    if(fields[i].type == "Title")
      m_fieldtypes[i] = TitleFieldType;
    if(fields[i].type == "Number")
      m_fieldtypes[i] = NumberFieldType;
    if(fields[i].type == "Boolean")
      m_fieldtypes[i] = BooleanFieldType;
    if(fields[i].type == "Select")
      m_fieldtypes[i] = SelectFieldType;
    if(fields[i].type == "Color")
      m_fieldtypes[i] = ColorFieldType;
    if(fields[i].type == "Section")
      m_fieldtypes[i] = SectionFieldType;
  }
}
#endif


// used to send an answer as INT to the calling http request
// TODO: Use one answer function with parameters being overloaded
void sendInt(String name, uint16_t value)
{
  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& answer = answerObj.createNestedObject("returnState");
  answer[name] = value;
  String ret;
  
  /*
  String answer = F("{ ");
  answer += F("\"currentState\" : { \"");
  answer += name;
  answer += F("\": ");
  answer += value;
  answer += " } }";
  */
  #ifdef DEBUG
    ret.reserve(answerObj.measurePrettyLength());
    answerObj.prettyPrintTo(ret);
  #else
    ret.reserve(answerObj.measureLength());
    answerObj.printTo(ret);
  #endif
  jsonBuffer.clear();
  DEBUGPRNT("Send HTML respone 200, application/json with value: " + ret);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", ret);
}

// used to send an answer as String to the calling http request
// TODO: Use one answer function with parameters being overloaded
void sendString(String name, String value)
{
  /*
  String answer = F("{ ");
  answer += F("\"currentState\" : { \"");
  answer += name;
  answer += F("\": \"");
  answer += value;
  answer += "\" } }";
  */
  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& answer = answerObj.createNestedObject("returnState");
  answer[name] = value;
  String ret;
  
  #ifdef DEBUG
    ret.reserve(answerObj.measurePrettyLength());
    answerObj.prettyPrintTo(ret);
  #else
    ret.reserve(answerObj.measureLength());
    answerObj.printTo(ret);
  #endif

  jsonBuffer.clear();

  DEBUGPRNT("Send HTML respone 200, application/json with value: " + ret);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", ret);
}

// used to send an answer as JSONString to the calling http request
// send answer can embed a complete json string instead of a single name / value pair.
void sendAnswer(String jsonAnswer)
{
  String answer = "{ \"returnState\": { " + jsonAnswer + "} }";
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", answer);
}

// broadcasts the name and value to all websocket clients
void broadcastInt(String name, uint16_t value)
{
  if(webSocketsServer == NULL)
  {
    return;
  }
  /*
  String json = "{\"name\":\"" + name + "\",\"value\":" + String(value) + "}";
  DEBUGPRNT("Send websocket broadcast with value: " + json);
  webSocketsServer->broadcastTXT(json);
  */
  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& answer = answerObj.createNestedObject("currentState");
  answer["name"] = name;
  answer["value"] = value;

  String json;
  
  #ifdef DEBUG
    json.reserve(answer.measurePrettyLength());
    answer.prettyPrintTo(json);
  #else
    json.reserve(answer.measureLength());
    answer.printTo(json);
  #endif
  jsonBuffer.clear();
  DEBUGPRNT("Send websocket broadcast with value: " + json);
  webSocketsServer->broadcastTXT(json);

}

// broadcasts the name and value to all websocket clients
// TODO: One function with parameters being overloaded.
void broadcastString(String name, String value)
{
  if(webSocketsServer == NULL)
  {
    return;
  }
  /*
  String json = "{\"name\":\"" + name + "\",\"value\":\"" + String(value) + "\"}";
  DEBUGPRNT("Send websocket broadcast with value: " + json);
  webSocketsServer->broadcastTXT(json);
  */
  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& answer = answerObj.createNestedObject("currentState");
  answer["name"] = name;
  answer["value"] = value;

  answer["name"] = name;
  answer["value"] = value;
  String json;
  
  #ifdef DEBUG
    json.reserve(answer.measurePrettyLength());
    answer.prettyPrintTo(json);
  #else
    json.reserve(answer.measureLength());
    answer.printTo(json);
  #endif
  jsonBuffer.clear();
  DEBUGPRNT("Send websocket broadcast with value: " + json);
  webSocketsServer->broadcastTXT(json);
}

void checkSegmentChanges(void) {

  #ifdef DEBUG
  bool save = shouldSaveRuntime;
  #endif

  if(seg.power != strip->getPower()) {
    seg.power = strip->getPower();
    broadcastInt("power", seg.power);
    shouldSaveRuntime = true;
  }
  if(seg.isRunning != strip->isRunning()) {
    seg.isRunning = strip->isRunning();
    broadcastInt("isRunning", seg.isRunning);
    shouldSaveRuntime = true;
  }
  if(seg.targetBrightness != strip->getTargetBrightness()) {
    seg.targetBrightness = strip->getTargetBrightness();
    broadcastInt("br", seg.targetBrightness);
    shouldSaveRuntime = true;
  }
  if(seg.mode != strip->getMode()){
    seg.mode = strip->getMode();
    broadcastInt("mo", seg.mode);
    shouldSaveRuntime = true;
  }
  if(seg.targetPaletteNum != strip->getTargetPaletteNumber()) {
    seg.targetPaletteNum = strip->getTargetPaletteNumber();
    broadcastInt("pa", seg.targetPaletteNum);
    shouldSaveRuntime = true;
  }
  if(seg.beat88 != strip->getBeat88()) {
    seg.beat88 = strip->getBeat88();
    broadcastInt("sp", seg.beat88);
    shouldSaveRuntime = true;
  }
  if(seg.blendType != strip->getBlendType()) {
    seg.blendType = strip->getBlendType();
    broadcastInt("blendType", seg.blendType);
    shouldSaveRuntime = true;
  }
  if(seg.colorTemp != strip->getColorTemperature())
  {
    seg.colorTemp = strip->getColorTemperature();
    broadcastInt("ColorTemperature", strip->getColorTemp());
    shouldSaveRuntime = true;
  }
  if(seg.blur != strip->getBlurValue())
  {
    seg.blur = strip->getBlurValue();
    broadcastInt("LEDblur", seg.blur);
    shouldSaveRuntime = true;
  }
  if(seg.reverse != strip->getReverse())
  {
    seg.reverse = strip->getReverse();
    broadcastInt("reverse", seg.reverse);
    shouldSaveRuntime = true;
  }
  if(seg.segments != strip->getSegments())
  {
    seg.segments = strip->getSegments();
    broadcastInt("segments", seg.segments);
    shouldSaveRuntime = true;
  }
  if(seg.mirror != strip->getMirror())
  {
    seg.mirror = strip->getMirror();
    broadcastInt("mirror", seg.mirror);
    shouldSaveRuntime = true;
  }
  if(seg.inverse != strip->getInverse())
  {
    seg.inverse = strip->getInverse();
    broadcastInt("inverse",seg.inverse);
    shouldSaveRuntime = true;
  }
  if(seg.hueTime != strip->getHueTime())
  {
    seg.hueTime = strip->getHueTime();
    broadcastInt("huetime", seg.hueTime);
    shouldSaveRuntime = true;
  }
  if(seg.deltaHue != strip->getDeltaHue())
  { 
    seg.deltaHue = strip->getDeltaHue();
    broadcastInt("deltahue", seg.deltaHue);
    shouldSaveRuntime = true;
  }
  if(seg.autoplay != strip->getAutoplay())
  {
    seg.autoplay = strip->getAutoplay();
    broadcastInt("autoplay", seg.autoplay);
    shouldSaveRuntime = true;
  }
  if(seg.autoplayDuration != strip->getAutoplayDuration())
  {
    seg.autoplayDuration = strip->getAutoplayDuration();
    broadcastInt("autoplayDuration", seg.autoplayDuration);
    shouldSaveRuntime = true;
  }
  if(seg.autoPal != strip->getAutopal())
  {
    seg.autoPal = strip->getAutopal();
    broadcastInt("autopal", seg.autoPal);
    shouldSaveRuntime = true;
  }
  if(seg.autoPalDuration != strip->getAutopalDuration())
  {
    seg.autoPalDuration = strip->getAutopalDuration();
    broadcastInt("autopalDuration", seg.autoPalDuration);
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
    broadcastInt("cooling", seg.cooling);
    shouldSaveRuntime = true;
  }
  if(seg.sparking != strip->getSparking())
  {
    seg.sparking = strip->getSparking();
    broadcastInt("sparking", seg.sparking);
    shouldSaveRuntime = true;
  }
  if(seg.twinkleSpeed != strip->getTwinkleSpeed())
  {
    seg.twinkleSpeed = strip->getTwinkleSpeed();
    broadcastInt("twinkleSpeed", seg.twinkleSpeed);
    shouldSaveRuntime = true;
  }
  if(seg.twinkleDensity != strip->getTwinkleDensity())
  {
    seg.twinkleDensity = strip->getTwinkleDensity();
    broadcastInt("twinkleDensity", seg.twinkleDensity);
    shouldSaveRuntime = true;
  }
  if(seg.numBars != strip->getNumBars())
  {
    seg.numBars = strip->getNumBars();
    broadcastInt("numBars", seg.numBars);
    shouldSaveRuntime = true;
  }
  if(seg.damping != strip->getDamping())
  {
    seg.damping = strip->getDamping();
    broadcastInt("damping", seg.damping);
    shouldSaveRuntime = true;
  }
  if(seg.sunrisetime != strip->getSunriseTime())
  {
    seg.sunrisetime = strip->getSunriseTime();
    broadcastInt("sunriseset", seg.sunrisetime);
    shouldSaveRuntime = true;
  }
  if(seg.milliamps != strip->getMilliamps())
  {
    seg.milliamps = strip->getMilliamps();
    broadcastInt("current", seg.milliamps);
    shouldSaveRuntime = true;
  }
  if(seg.fps != strip->getMaxFPS())
  {
    seg.fps = strip->getMaxFPS();
    broadcastInt("fps", seg.fps);
    shouldSaveRuntime = true;
  }
  if(seg.dithering != strip->getDithering())
  {
    seg.dithering = strip->getDithering();
    broadcastInt("dithering", seg.dithering);
    shouldSaveRuntime = true;
  }
  if(seg.addGlitter != strip->getAddGlitter())
  {
    seg.addGlitter= strip->getAddGlitter();
    broadcastInt("addGlitter", seg.addGlitter);
    shouldSaveRuntime = true;
  }
  if(seg.whiteGlitter != strip->getWhiteGlitter())
  {
    seg.whiteGlitter= strip->getWhiteGlitter();
    broadcastInt("WhiteOnly", seg.whiteGlitter);
    shouldSaveRuntime = true;
  }
  if(seg.onBlackOnly != strip->getOnBlackOnly())
  {
    seg.onBlackOnly = strip->getOnBlackOnly();
    broadcastInt("onBlackOnly", seg.onBlackOnly);
    shouldSaveRuntime = true;
  }
  if(seg.chanceOfGlitter != strip->getChanceOfGlitter())
  {
    seg.chanceOfGlitter = strip->getChanceOfGlitter();
    broadcastInt("glitterChance", seg.chanceOfGlitter);
    shouldSaveRuntime = true;
  }


  #ifdef DEBUG  
    if(save != shouldSaveRuntime)
    {
      DEBUGPRNT("changes detected by checkSegmentChanges. EEPROM save at the next possibility...");
    }
  #endif
}

void print_segment(WS2812FX::segment * s) {
  DEBUGPRNT("\nPrinting segment content");
  #ifdef DEBUG
  Serial.println("\t\tCRC:\t\t\t" + String(s->CRC));
  Serial.println("\t\tPower:\t\t\t" + String(s->power));
  Serial.println("\t\tisRunning:\t\t" + String(s->isRunning));
  Serial.println("\t\tReverse:\t\t" + String(s->reverse));
  Serial.println("\t\tInverse:\t\t" + String(s->inverse));
  Serial.println("\t\tMirror:\t\t\t" + String(s->mirror));
  Serial.println("\t\tAutoplay:\t\t" + String(s->autoplay));  
  Serial.println("\t\tAutopal:\t\t" + String(s->autoPal));
  Serial.println("\t\tbeat88:\t\t\t" + String(s->beat88));
  Serial.println("\t\thueTime:\t\t" + String(s->hueTime));
  Serial.println("\t\tmilliamps:\t\t" + String(s->milliamps));
  Serial.println("\t\tautoplayduration:\t" + String(s->autoplayDuration));
  Serial.println("\t\tautopalduration:\t" + String(s->autoPalDuration));
  Serial.println("\t\tsegements:\t\t" + String(s->segments));
  Serial.println("\t\tcooling:\t\t" + String(s->cooling));
  Serial.println("\t\tsparking:\t\t" + String(s->sparking));
  Serial.println("\t\ttwinkleSpeed:\t\t" + String(s->twinkleSpeed));
  Serial.println("\t\ttwinkleDensity:\t\t" + String(s->twinkleDensity));
  Serial.println("\t\tnumBars:\t\t" + String(s->numBars));
  Serial.println("\t\tmode:\t\t\t" + String(s->mode));
  Serial.println("\t\tfps:\t\t\t" + String(s->fps));
  Serial.println("\t\tdeltaHue:\t\t" + String(s->deltaHue));
  Serial.println("\t\tblur:\t\t\t" + String(s->blur));
  Serial.println("\t\tdamping:\t\t" + String(s->damping));
  Serial.println("\t\tdithering:\t\t" + String(s->dithering));
  Serial.println("\t\tsunrisetime:\t\t" + String(s->sunrisetime));
  Serial.println("\t\ttargetBrightness:\t" + String(s->targetBrightness));
  Serial.println("\t\ttargetPaletteNum:\t" + String(s->targetPaletteNum));
  Serial.println("\t\tcurrentPaletteNum:\t" + String(s->currentPaletteNum));
  Serial.println("\t\tblendType:\t\t" + String(s->blendType));
  Serial.println("\t\tcolorTemp:\t\t" + String(s->colorTemp));
  Serial.println("\n\t\tDONE\n");
  #endif
}

// write runtime data to EEPROM (when required by "shouldSave Runtime")
void saveEEPROMData(void)
{
  if (!shouldSaveRuntime)
    return;
  shouldSaveRuntime = false;
  DEBUGPRNT("Going to store runtime on EEPROM...");
  // we will store the complete segment data
  
  //now in "checkSegment"
  //WS2812FX::segment seg = *strip->getSegment();

  //DEBUGPRNT("WS2812 segment:");
  //print_segment(strip->getSegment());
  //DEBUGPRNT("copied segment:");
  //print_segment(&seg);


  seg.CRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a,(unsigned char *)&seg + 2, sizeof(seg) - 2);

  strip->setCRC(seg.CRC);

  DEBUGPRNT("\tSegment size: " + String(strip->getSegmentSize())+ "\tCRC size: " + String(strip->getCRCsize()));
  DEBUGPRNT("\tCRC calculated " + String(seg.CRC) + "\tCRC stored " + String(strip->getCRC()));

  //DEBUGPRNT("WS2812 segment with new CRC:");
  //print_segment(strip->getSegment());

  // write the data to the EEPROM
  EEPROM.put(0, seg);
  // as we work on ESP, we also need to commit the written data.
  EEPROM.commit();

  DEBUGPRNT("EEPROM write finished...");
}


// reads the stored runtime data from EEPROM
// must be called after everything else is already setup to be working
// otherwise this may terribly fail and could override what was read already
void readRuntimeDataEEPROM(void)
{
  DEBUGPRNT("\tReading Config From EEPROM...");
  // copy segment...
  
  // now separate global....
  //WS2812FX::segment seg;
  
  //read the configuration from EEPROM into RAM
  EEPROM.get(0, seg);

  DEBUGPRNT("Segment after Reading:");
  print_segment(&seg);

  // calculate the CRC over the data being stored in the EEPROM (except the CRC itself)
  uint16_t mCRC = (uint16_t)WS2812FX::calc_CRC16(0x5a5a, (unsigned char *)&seg + 2, sizeof(seg) - 2);

  
  DEBUGPRNT("\tCRC calculated " + String(mCRC) + "\tCRC stored " + String(seg.CRC));
  
  // we have a matching CRC, so we update the runtime data.
  if (seg.CRC == mCRC)
  {
    DEBUGPRNT("\tWe got a CRC match!...");
    
    (*strip->getSegment()) = seg;
    strip->init();
  }
  else // load defaults
  {
    DEBUGPRNT("\tWe got NO NO NO CRC match!!!");
    DEBUGPRNT("Loading default Data...");
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
  DEBUGPRNT("Initializing OTA capabilities....");
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // TODO: Implement Hostname in config and WIFI Settings?

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(LED_NAME);

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    DEBUGPRNT("OTA start");
    display.clear();
    display.drawString(0, 0, "Starte OTA...");
    display.displayOn();
    display.display();
    // we need to delete the websocket server in order to have OTA correctly running.
    delete webSocketsServer;
    // we stop the webserver to not get interrupted....
    server.stop();
    // we indicate our sktch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });

  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    DEBUGPRNT("OTA end");
    // OTA finished.
    display.drawString(0, 53, "OTA beendet!");
    display.displayOn();
    display.display();
    // indicate that OTA is no longer running.
    OTAisRunning = false;
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress on the strips as well to be informed if anything gets stuck...
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUGPRNT("Progress: " + String((progress / (total / 100))));

    unsigned int prog = (progress / (total / 100));
    display.clear();
    display.drawString(0, 0, "Starte OTA...");
    display.drawStringMaxWidth(0, 12, 128, "Prog: " + String(progress) + " / " + String(total));
    display.drawProgressBar(1,33, 126, 7, prog);
    display.displayOn();
    display.display();

    
  });

  // something went wrong, we gonna show an error "message" via LEDs.
  ArduinoOTA.onError([](ota_error_t error) {
    String err = "OTA Fehler: ";

    DEBUGPRNT("Error[%u]: " + String(error));
    if (error == OTA_AUTH_ERROR) {
      DEBUGPRNT("Auth Failed");
      err = err + "Auth Failed";
    }
    else if (error == OTA_BEGIN_ERROR) {
      DEBUGPRNT("Begin Failed");
      err = err + "Begin Failed";
    }
    else if (error == OTA_CONNECT_ERROR) {
      DEBUGPRNT("Connect Failed");
      err = err + ("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      DEBUGPRNT("Receive Failed");
      err = err + ("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      DEBUGPRNT("End Failed");
      err = err + ("End Failed");
    }

    display.clear();
    display.drawStringMaxWidth(0, 0,  128, "Update fehlgeschlagen!");
    display.drawStringMaxWidth(0, 22, 128, err);
    display.drawStringMaxWidth(0, 43, 128, "Reset in 10 Sek");
    delay(KNOB_BOOT_DELAY);
    ESP.restart();
  });
  // start the service
  ArduinoOTA.begin();
  DEBUGPRNT("OTA capabilities initialized....");
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
  #else // HAS_KNOB_CONTROL
  DEBUGPRNT("Initializing OTA capabilities....");
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // TODO: Implement Hostname in config and WIFI Settings?

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(LED_NAME);

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    DEBUGPRNT("OTA start");

    strip->setIsRunning(false);
    strip->setPower(false);
    // the following is just to visually indicate the OTA start
    // which is done by blinking the complete stripe in different colors from yellow to green
    uint8_t factor = 85;
    for (uint8_t c = 0; c < 4; c++)
    {

      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
        uint8_t r = 256 - (c * factor);
        uint8_t g = c > 0 ? (c * factor - 1) : (c * factor);
        //strip.setPixelColor(i, r, g, 0);
        strip->leds[i] = CRGB(strip_color32(r, g, 0));
      }
      strip->show();
      delay(250);
      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
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
    DEBUGPRNT("OTA end");
    // OTA finished.
    // We fade out the green Leds being activated during OTA.
    for (uint8_t i = strip->leds[0].green; i > 0; i--)
    {
      for (uint16_t p = 0; p < strip->getStripLength(); p++)
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
    DEBUGPRNT("Progress: " + String((progress / (total / 100))));

    // OTA Update will show increasing green LEDs during progress:
    uint16_t progress_value = progress * 100 / (total / strip->getStripLength());
    uint16_t pixel = (uint16_t)(progress_value / 100);
    uint16_t temp_color = progress_value - (pixel * 100);
    if (temp_color > 255)
      temp_color = 255;

    strip->leds[pixel] = strip_color32(0, (uint8_t)temp_color, 0);
    strip->show();
  });

  // something went wrong, we gonna show an error "message" via LEDs.
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUGPRNT("Error[%u]: " + String(error));
    if (error == OTA_AUTH_ERROR) {
      DEBUGPRNT("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      DEBUGPRNT("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      DEBUGPRNT("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      DEBUGPRNT("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      DEBUGPRNT("End Failed");
    }
    // something went wrong during OTA.
    // We will fade in to red...
    for (uint16_t c = 0; c < 256; c++)
    {
      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
        //strip.setPixelColor(i,(uint8_t)c,0,0);
        strip->leds[i] = strip_color32((uint8_t)c, 0, 0);
      }
      strip->show();
      delay(2);
    }
    // We wait 10 seconds and then restart the ESP...
    delay(10000);
    ESP.restart();
  });
  // start the service
  ArduinoOTA.begin();
  DEBUGPRNT("OTA capabilities initialized....");
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
  #endif // HAS_KNOB_CONTROL
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
#endif
}

// setup the Wifi connection with Wifi Manager...
void setupWiFi(void)
{

  showInitColor(CRGB::Blue);
  delay(INITDELAY);

  WiFi.hostname(LED_NAME);

  WiFi.mode(WIFI_STA);

  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  WiFi.persistent(true);

  WiFiManager wifiManager;

#ifndef HAS_KNOB_CONTROL
  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(240);
 
  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

//tries to connect to last known settings
//if it does not connect it starts an access point with the specified name
//and goes into a blocking loop awaiting configuration
  DEBUGPRNT("Going to autoconnect and/or Start AP");
  if (!wifiManager.autoConnect(AP_SSID.c_str()))
  {
    DEBUGPRNT("Config saved (or timed out), we should reset as see if it connects");
    showInitColor(CRGB::Yellow);
    delay(3000);
    showInitColor(CRGB::Red);
    ESP.restart();
    delay(5000);
  }

  if(WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
  }

  WiFi.setAutoReconnect(true);
//if we get here we have connected to the WiFi
  DEBUGPRNT("local ip: ");
  DEBUGPRNT(WiFi.localIP());


#else // We have a control knob / button

  // If we are in button control mode
  // we only need WiFi for "convenience"
  if(WiFiConnected)
    wifiManager.setConfigPortalTimeout(120);
  else
    wifiManager.setConfigPortalTimeout(10);
 
  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

//tries to connect to last known settings
//if it does not connect it starts an access point with the specified name
//and goes into a blocking loop awaiting configuration
  DEBUGPRNT("Going to autoconnect and/or Start AP");
  if (!wifiManager.autoConnect(AP_SSID.c_str()))
  {
    DEBUGPRNT("Config saved (or timed out), we should reset as see if it connects");
    WiFiConnected = false;
  }
  else
  {
    WiFiConnected = true;
    if(WiFi.getMode() != WIFI_STA)
    {
      WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(true);
  //if we get here we have connected to the WiFi
    DEBUGPRNT("local ip: ");
    DEBUGPRNT(WiFi.localIP());
  }

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
void handleSet(void)
{
  // Debug only
  #ifdef DEBUG
  IPAddress add = server.client().remoteIP();
  DEBUGPRNT("The HTTP Request was received by " + add.toString());
  DEBUGPRNT("<Begin>Server Args:");
  for (uint8_t i = 0; i < server.args(); i++)
  {
    DEBUGPRNT(server.argName(i) + "\t" + server.arg(i));
  }
  DEBUGPRNT("<End> Server Args");
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

  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& answer = answerObj.createNestedObject("currentState");
  
  if (server.hasArg("mo"))
  {
    // flag to decide if this is an library effect
    bool isWS2812FX = false;
    // current library effect number
    uint8_t effect = strip->getMode();

    DEBUGPRNT("got Argument mo....");

    // just switch to the next if we get an "u" for up
    if (server.arg("mo")[0] == 'u')
    {
      DEBUGPRNT("got Argument mode up....");
      //effect = effect + 1;
      isWS2812FX = true;
      effect = strip->nextMode(AUTO_MODE_UP);
    }
    // switch to the previous one if we get a "d" for down
    else if (server.arg("mo")[0] == 'd')
    {
      DEBUGPRNT("got Argument mode down....");
      //effect = effect - 1;
      isWS2812FX = true;
      effect = strip->nextMode(AUTO_MODE_DOWN);
    }
    // if we get an "o" for off, we switch off
    else if (server.arg("mo")[0] == 'o')
    {
      DEBUGPRNT("got Argument mode Off....");
      strip->setPower(false);
      //sendString("state", "off");
      //broadcastInt("power", false);
    }
    // for backward compatibility and FHEM:
    // --> activate fire flicker
    else if (server.arg("mo")[0] == 'f')
    {

      DEBUGPRNT("got Argument fire....");

      effect = FX_MODE_FIRE_FLICKER;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate rainbow effect
    else if (server.arg("mo")[0] == 'r')
    {
      DEBUGPRNT("got Argument mode rainbow cycle....");
      effect = FX_MODE_RAINBOW_CYCLE;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate the K.I.T.T. (larson scanner)
    else if (server.arg("mo")[0] == 'k')
    {
      DEBUGPRNT("got Argument mode KITT....");
      effect = FX_MODE_LARSON_SCANNER;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox
    else if (server.arg("mo")[0] == 's')
    {
      DEBUGPRNT("got Argument mode Twinkle Fox....");
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // for backward compatibility and FHEM:
    // --> activate Twinkle Fox in white...
    else if (server.arg("mo")[0] == 'w')
    {
      DEBUGPRNT("got Argument mode White Twinkle....");
      strip->setColor(CRGBPalette16(CRGB::White));
      effect = FX_MODE_TWINKLE_FOX;
      isWS2812FX = true;
    }
    // sunrise effect
    else if (server.arg("mo") == "Sunrise")
    {
      DEBUGPRNT("got Argument mode sunrise....");
      // sunrise time in seconds
      if (server.hasArg("sec"))
      {
        DEBUGPRNT("got Argument sec....");
        strip->setSunriseTime(((uint16_t)strtoul(server.arg("sec").c_str(), NULL, 10)) / 60);
      }
      // sunrise time in minutes
      else if (server.hasArg("min"))
      {
        DEBUGPRNT("got Argument min....");
        strip->setSunriseTime(((uint16_t)strtoul(server.arg("min").c_str(), NULL, 10)));
      }
      isWS2812FX = true;
      effect = FX_MODE_SUNRISE;
      strip->setTransition();
      answer.set("sunRiseTime", strip->getSunriseTime());
      answer.set("sunRiseTimeToFinish", strip->getSunriseTimeToFinish());
      answer.set("sunRiseMode", "sunrise");
      answer.set("sunRiseActive", "on");
      //broadcastInt("sunriseset", strip->getSunriseTime());
      //sendStatus = true;
    }
    // the same for sunset....
    else if (server.arg("mo") == "Sunset")
    {
      DEBUGPRNT("got Argument mode sunset....");
      // sunrise time in seconds
      if (server.hasArg("sec"))
      {
        DEBUGPRNT("got Argument sec....");
        strip->setSunriseTime(((uint16_t)strtoul(server.arg("sec").c_str(), NULL, 10)) / 60);
      }
      // sunrise time in minutes
      else if (server.hasArg("min"))
      {
        DEBUGPRNT("got Argument min....");
        strip->setSunriseTime( ((uint16_t)strtoul(server.arg("min").c_str(), NULL, 10)));
      }

      // answer for the "calling" party
      isWS2812FX = true;
      effect = FX_MODE_SUNSET;
      strip->setTransition();
      //broadcastInt("sunriseset", strip->getSunriseTime());
      //sendStatus = true;
      answer.set("sunRiseTime", strip->getSunriseTime());
      answer.set("sunRiseTimeToFinish", strip->getSunriseTimeToFinish());
      answer.set("sunRiseMode", "sunset");
      answer.set("sunRiseActive", "on");
    }
    // finally - if nothing matched before - we switch to the effect  being provided.
    // we don't care if its actually an int or not
    // because it will be zero anyway if not.
    else
    {
      DEBUGPRNT("got Argument mode and seems to be an Effect....");
      effect = (uint8_t)strtoul(server.arg("mo").c_str(), NULL, 10);
      isWS2812FX = true;
    }
    // sanity only, actually handled in the library...
    if (effect >= strip->getModeCount())
    {
      DEBUGPRNT("Effect to high....");
      effect = 0;
    }
    // activate the effect...
    if (isWS2812FX)
    {
      strip->setMode(effect);
      // in case it was stopped before
      // seems to be obsolete but does not hurt anyway...
      strip->start();

      DEBUGPRNT("gonna send mo response....");
      //sendInt("mo", strip->getMode() );
      //broadcastInt("power", true);
      answer.set("wsfxmode_Num", effect);
      answer.set("wsfxmode", strip->getModeName(effect));
      answer.set("state", strip->getPower() ? "on" : "off");
      answer.set("power", strip->getPower());
    }
    else
    {
      answer.set("state", strip->getPower() ? "on" : "off");
      answer.set("power", strip->getPower());
    }
    
  }
  // global on/off
  if (server.hasArg("power"))
  {
    DEBUGPRNT("got Argument power....");
    if (server.arg("power")[0] == '0')
    {
      strip->setPower(false);
    }
    else
    {
      strip->setPower(true);
      strip->setMode(strip->getMode());
    }
    answer.set("state", strip->getPower() ? "on" : "off");
    answer.set("power", strip->getPower());
  }

  if(server.hasArg("isRunning"))
  {
    DEBUGPRNT("got Argument \"isRunning\"....");
    if (server.arg("isRunning")[0] == '0')
    {
      strip->setIsRunning(false);
    }
    else
    {
      strip->setIsRunning(true);
    }
    answer.set("isRunning", strip->isRunning() ? "running" : "paused");
    //sendString("isRunning", strip->isRunning() ? "running" : "paused");
  }

  // if we got a palette change
  if (server.hasArg("pa"))
  {
    // TODO: Possibility to setColors and new Palettes...
    uint8_t pal = (uint8_t)strtoul(server.arg("pa").c_str(), NULL, 10);
    DEBUGPRNT("New palette with value: " + String(pal));
    strip->setTargetPalette(pal);
    //  sendAnswer(   "\"palette\": " + String(pal) + ", \"palette name\": \"" +
    //                (String)strip->getPalName(pal) + "\"");
    //  broadcastInt("pa", pal);
    answer.set("palette_num", strip->getTargetPaletteNumber());
    answer.set("palette_name", strip->getTargetPaletteName());
    answer.set("palette_count", strip->getPalCount());
  }

  // if we got a new brightness value
  if (server.hasArg("br"))
  {
    DEBUGPRNT("got Argument brightness....");
    uint8_t brightness = strip->getBrightness();
    if (server.arg("br")[0] == 'u')
    {
      brightness = changebypercentage(brightness, 110);
    }
    else if (server.arg("br")[0] == 'd')
    {
      brightness = changebypercentage(brightness, 90);
    }
    else
    {
      brightness = constrain((uint8_t)strtoul(server.arg("br").c_str(), NULL, 10), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    }
    strip->setBrightness(brightness);
    //sendInt("brightness", brightness);
    //broadcastInt("br", strip->getBrightness());
    answer.set("brightness", strip->getBrightness());
  }

  // if we got a speed value
  // for backward compatibility.
  // is beat88 value anyway
  if (server.hasArg("sp"))
  {
#ifdef DEBUG
    DEBUGPRNT("got Argument speed....");
#endif
    uint16_t speed = strip->getBeat88();
    if (server.arg("sp")[0] == 'u')
    {
      uint16_t ret = max((speed * 115) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else if (server.arg("sp")[0] == 'd')
    {
      uint16_t ret = max((speed * 80) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else
    {
      speed = constrain((uint16_t)strtoul(server.arg("sp").c_str(), NULL, 10), BEAT88_MIN, BEAT88_MAX);
    }
    strip->setSpeed(speed);
    strip->show();
    //sendAnswer("\"speed\": " + String(speed) + ", \"beat88\": \"" + String(speed));
    answer.set("speed", strip->getSpeed());
    answer.set("beat88", strip->getSpeed());
    //broadcastInt("sp", strip->getBeat88());
    strip->setTransition();
  }

  // if we got a speed value (as beat88)
  if (server.hasArg("be"))
  {
#ifdef DEBUG
    DEBUGPRNT("got Argument speed (beat)....");
#endif
    uint16_t speed = strip->getBeat88();
    if (server.arg("be")[0] == 'u')
    {
      uint16_t ret = max((speed * 115) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else if (server.arg("be")[0] == 'd')
    {
      uint16_t ret = max((speed * 80) / 100, 10);
      if (ret > BEAT88_MAX)
        ret = BEAT88_MAX;
      speed = ret;
    }
    else
    {
      speed = constrain((uint16_t)strtoul(server.arg("be").c_str(), NULL, 10), BEAT88_MIN, BEAT88_MAX);
    }
    strip->setSpeed(speed);
    strip->show();
    answer.set("speed", strip->getSpeed());
    answer.set("beat88", strip->getSpeed());
    //sendAnswer("\"speed\": " + String(speed) + ", \"beat88\": \"" + String(speed));
    //broadcastInt("sp", strip->getBeat88());
    strip->setTransition();
  }

  // color handling
  // this is a bit tricky, as it handles either RGB as one or different values.

  // current color (first value from palette)
  uint32_t color = strip->getColor(0);
  bool setColor = false;
  // we got red
  if (server.hasArg("re"))
  {
    setColor = true;
#ifdef DEBUG
    DEBUGPRNT("got Argument red....");
#endif
    uint8_t re = Red(color);
    if (server.arg("re")[0] == 'u')
    {
      re = changebypercentage(re, 110);
    }
    else if (server.arg("re")[0] == 'd')
    {
      re = changebypercentage(re, 90);
    }
    else
    {
      re = constrain((uint8_t)strtoul(server.arg("re").c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0x00ffff) | (re << 16);
  }
  // we got green
  if (server.hasArg("gr"))
  {
    setColor = true;
#ifdef DEBUG
    DEBUGPRNT("got Argument green....");
#endif
    uint8_t gr = Green(color);
    if (server.arg("gr")[0] == 'u')
    {
      gr = changebypercentage(gr, 110);
    }
    else if (server.arg("gr")[0] == 'd')
    {
      gr = changebypercentage(gr, 90);
    }
    else
    {
      gr = constrain((uint8_t)strtoul(server.arg("gr").c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0xff00ff) | (gr << 8);
  }
  // we got blue
  if (server.hasArg("bl"))
  {
    setColor = true;
#ifdef DEBUG
    DEBUGPRNT("got Argument blue....");
#endif
    uint8_t bl = Blue(color);
    if (server.arg("bl")[0] == 'u')
    {
      bl = changebypercentage(bl, 110);
    }
    else if (server.arg("bl")[0] == 'd')
    {
      bl = changebypercentage(bl, 90);
    }
    else
    {
      bl = constrain((uint8_t)strtoul(server.arg("bl").c_str(), NULL, 10), 0, 255);
    }
    color = (color & 0xffff00) | (bl << 0);
  }
  // we got a 32bit color value (24 actually)
  if (server.hasArg("co"))
  {
    setColor = true;
#ifdef DEBUG
    DEBUGPRNT("got Argument color....");
#endif
    color = constrain((uint32_t)strtoul(server.arg("co").c_str(), NULL, 16), 0, 0xffffff);
  }
  // we got one solid color value as r, g, b
  if (server.hasArg("solidColor"))
  {
    setColor = true;
#ifdef DEBUG
    DEBUGPRNT("got Argument solidColor....");
#endif
    uint8_t r, g, b;
    r = constrain((uint8_t)strtoul(server.arg("r").c_str(), NULL, 10), 0, 255);
    g = constrain((uint8_t)strtoul(server.arg("g").c_str(), NULL, 10), 0, 255);
    b = constrain((uint8_t)strtoul(server.arg("b").c_str(), NULL, 10), 0, 255);
    color = (r << 16) | (g << 8) | (b << 0);
    // CRGB solidColor(color); // obsolete?

    //broadcastInt("pa", strip->getPalCount()); // this reflects a "custom palette"
  }
  // a signle pixel...
  //FIXME: Does not yet work. Lets simplyfy all of this!
  if (server.hasArg("pi"))
  {
#ifdef DEBUG
    DEBUGPRNT("got Argument pixel....");
#endif
    //setEffect(FX_NO_FX);
    uint16_t pixel = constrain((uint16_t)strtoul(server.arg("pi").c_str(), NULL, 10), 0, strip->getStripLength() - 1);

    strip->setMode(FX_MODE_VOID);
    strip->leds[pixel] = CRGB(color);
    //sendStatus = true;
    // a range of pixels from start rnS to end rnE
    answer.set("wsfxmode_Num", FX_MODE_VOID);
    answer.set("wsfxmode", strip->getModeName(FX_MODE_VOID));
    answer.set("state", strip->getPower() ? "on" : "off");
    answer.set("power", strip->getPower());
  }
  //FIXME: Does not yet work. Lets simplyfy all of this!
  else if (server.hasArg("rnS") && server.hasArg("rnE"))
  {
#ifdef DEBUG
    DEBUGPRNT("got Argument range start / range end....");
#endif
    uint16_t start = constrain((uint16_t)strtoul(server.arg("rnS").c_str(), NULL, 10), 0, strip->getStripLength());
    uint16_t end = constrain((uint16_t)strtoul(server.arg("rnE").c_str(), NULL, 10), start, strip->getStripLength());

    strip->setMode(FX_MODE_VOID);
    for (uint16_t i = start; i <= end; i++)
    {
      strip->leds[i] = CRGB(color);
    }
    answer.set("wsfxmode_Num", FX_MODE_VOID);
    answer.set("wsfxmode", strip->getModeName(FX_MODE_VOID));
    answer.set("state", strip->getPower() ? "on" : "off");
    answer.set("power", strip->getPower());
    //sendStatus = true;
    // one color for the complete strip
  }
  else if (server.hasArg("rgb"))
  {
#ifdef DEBUG
    DEBUGPRNT("got Argument rgb....");
#endif
    strip->setColor(color);
    strip->setMode(FX_MODE_STATIC);
    // finally set a new color
    answer.set("wsfxmode_Num", FX_MODE_STATIC);
    answer.set("wsfxmode", strip->getModeName(FX_MODE_STATIC));
    answer.set("state", strip->getPower() ? "on" : "off");
    answer.set("power", strip->getPower());
  }
  else
  {
    if (setColor)
    {
      strip->setColor(color);
      answer.set("rgb", color);
      answer.set("rgb_blue", Blue(color));
      answer.set("rgb_green", Green(color));
      answer.set("rgb_red", Red(color));
      //sendStatus = true;
    }
  }

  // autoplay flag changes
  if (server.hasArg("autoplay"))
  {
    uint16_t value = String(server.arg("autoplay")).toInt();
    strip->setAutoplay((AUTOPLAYMODES)value);
    //sendInt("Autoplay Mode", value);
    //broadcastInt("autoplay", value);
    switch(strip->getAutoplay())
  {
    case AUTO_MODE_OFF:
      answer["AutoPlayMode"] = "Off";
    break;
    case AUTO_MODE_UP:
      answer["AutoPlayMode"] = "Up";
    break;
    case AUTO_MODE_DOWN:
      answer["AutoPlayMode"] = "Down";
    break;
    case AUTO_MODE_RANDOM:
      answer["AutoPlayMode"] = "Random";
    break;
    default:
      answer["AutoPlayMode"] = "unknown error";
    break;
  }
  
  }

  // autoplay duration changes
  if (server.hasArg("autoplayDuration"))
  {
    uint16_t value = String(server.arg("autoplayDuration")).toInt();
    strip->setAutoplayDuration(value);
    //sendInt("Autoplay Mode Interval", value);
    //broadcastInt("autoplayDuration", value);
    answer["AutoPlayModeIntervall"] = strip->getAutoplayDuration();
 
  }

  // auto plaette change
  if (server.hasArg("autopal"))
  {
    uint16_t value = String(server.arg("autopal")).toInt();
    strip->setAutopal((AUTOPLAYMODES)value);
    //sendInt("Autoplay Palette", value);
    //broadcastInt("autopal", value);
    switch(strip->getAutopal())
    {
      case AUTO_MODE_OFF:
        answer["AutoPalette"] = "Off";
      break;
      case AUTO_MODE_UP:
        answer["AutoPalette"] = "Up";
      break;
      case AUTO_MODE_DOWN:
        answer["AutoPalette"] = "Down";
      break;
      case AUTO_MODE_RANDOM:
        answer["AutoPalette"] = "Random";
      break;
      default:
        answer["AutoPalette"] = "unknown error";
      break;
    }
  }

  // auto palette change duration changes
  if (server.hasArg("autopalDuration"))
  {
    uint16_t value = String(server.arg("autopalDuration")).toInt();
    strip->setAutopalDuration(value);
    //sendInt("Autoplay Palette Interval", value);
    //broadcastInt("autopalDuration", value);
    answer["AutoPaletteInterval"] = strip->getAutoplayDuration();
  }

  // time for cycling through the basehue value changes
  if (server.hasArg("huetime"))
  {
    uint16_t value = String(server.arg("huetime")).toInt();
    //sendInt("Hue change time", value);
    //broadcastInt("huetime", value);
    strip->setHuetime(value);
    answer["HueChangeInt"] = strip->getHueTime();
  }

#pragma message "We could implement a value to change how a palette is distributed accross the strip"

  // the hue offset for a given effect (if - e.g. not spread across the whole strip)
  if (server.hasArg("deltahue"))
  {
    uint16_t value = constrain(String(server.arg("deltahue")).toInt(), 0, 255);
    //sendInt("Delta hue per change", value);
    //broadcastInt("deltahue", value);
    strip->setDeltaHue(value);
    strip->setTransition();
    answer["HueDeltaHue"] = strip->getDeltaHue();
  }

  // parameter for teh "fire" - flame cooldown
  if (server.hasArg("cooling"))
  {
    uint16_t value = String(server.arg("cooling")).toInt();
    //sendInt("Fire Cooling", value);
    //broadcastInt("cooling", value);
    strip->setCooling(value);
    strip->setTransition();
    answer["Cooling"] = strip->getCooling();
  }

  // parameter for the sparking - new flames
  if (server.hasArg("sparking"))
  {
    uint16_t value = String(server.arg("sparking")).toInt();
    //sendInt("Fire sparking", value);
    //broadcastInt("sparking", value);
    strip->setSparking(value);
    strip->setTransition();
    answer["Sparking"] = strip->getSparking();
  }

  // parameter for twinkle fox (speed)
  if (server.hasArg("twinkleSpeed"))
  {
    uint16_t value = String(server.arg("twinkleSpeed")).toInt();
    //sendInt("Twinkle Speed", value);
    //broadcastInt("twinkleSpeed", value);
    strip->setTwinkleSpeed(value);
    strip->setTransition();
    answer["TwinkleSpeed"] = strip->getTwinkleSpeed();
  }

  // parameter for twinkle fox (density)
  if (server.hasArg("twinkleDensity"))
  {
    uint16_t value = String(server.arg("twinkleDensity")).toInt();
    //sendInt("Twinkle Density", value);
    //broadcastInt("twinkleDensity", value);
    strip->setTwinkleDensity(value);
    strip->setTransition();
    answer["TwinkleDensity"] = strip->getTwinkleDensity();
  }

  // parameter for number of bars (beat sine glows etc...)
  if (server.hasArg("numBars"))
  {
    uint16_t value = String(server.arg("numBars")).toInt();
    if (value > MAX_NUM_BARS)
      value = max(MAX_NUM_BARS, 1);
    //sendInt("Number of Bars", value);
    //broadcastInt("numBars", value);
    strip->setNumBars(value);
    strip->setTransition();
    answer["NumBars"] = strip->getNumBars();
  }

  // parameter to change the palette blend type for cetain effects
  if (server.hasArg("blendType"))
  {
    uint16_t value = String(server.arg("blendType")).toInt();

    //broadcastInt("blendType", value);
    if (value)
    {
      strip->setBlendType(LINEARBLEND);
      //sendString("BlendType", "LINEARBLEND");
      answer["BlendType"] = "Linear Blend";
    }
    else
    {
      strip->setBlendType(NOBLEND);
      //sendString("BlendType", "NOBLEND");
      answer["BlendType"] = "No Blend";
    }
    strip->setTransition();
  }

  // parameter to change the Color Temperature of the Strip
  if (server.hasArg("ColorTemperature"))
  {
    uint8_t value = String(server.arg("ColorTemperature")).toInt();

    //broadcastInt("ColorTemperature", value);
    //sendString("ColorTemperature", strip->getColorTempName(value));
    strip->setColorTemperature(value);
    strip->setTransition();
    answer["ColorTemperature"] = strip->getColorTempName(strip->getColorTemp());
  }

  // parameter to change direction of certain effects..
  if (server.hasArg("reverse"))
  {
    uint16_t value = String(server.arg("reverse")).toInt();
    //sendInt("reverse", value);
    //broadcastInt("reverse", value);
    strip->getSegment()->reverse = value;
    strip->setTransition();
    answer["Reverse"] = strip->getReverse();
  }

  // parameter to invert colors of all effects..
  if (server.hasArg("inverse"))
  {
    uint16_t value = String(server.arg("inverse")).toInt();
    //sendInt("inverse", value);
    //broadcastInt("inverse", value);
    strip->setInverse(value);
    strip->setTransition();
    answer["Inverse"] = strip->getInverse();
  }

  // parameter to divide LEDS into two equal halfs...
  if (server.hasArg("mirror"))
  {
    uint16_t value = String(server.arg("mirror")).toInt();
    //sendInt("mirror", value);
    //broadcastInt("mirror", value);
    strip->setMirror(value);
    strip->setTransition();
    answer["Mirrored"] = strip->getMirror();
  }

  // parameter so set the max current the leds will draw
  if (server.hasArg("current"))
  {
    uint16_t value = String(server.arg("current")).toInt();
    //sendInt("Lamp Max Current", value);
    //broadcastInt("current", value);
    strip->setMilliamps(value);
    answer["Lamp_max_current"] = strip->getMilliamps();
  }

  // parameter for the blur against the previous LED values
  if (server.hasArg("LEDblur"))
  {
    uint8_t value = String(server.arg("LEDblur")).toInt();
    //sendInt("LEDblur", value);
    //broadcastInt("LEDblur", value);
    strip->setBlur(value);
    strip->setTransition();
    answer["Led_blur"] = strip->getBlurValue();
  }

  // parameter for the frames per second (FPS)
  if (server.hasArg("fps"))
  {
    uint8_t value = String(server.arg("fps")).toInt();
    //sendInt("fps", value);
    //broadcastInt("fps", value);
    strip->setMaxFPS(value);
    strip->setTransition();
    answer["max_FPS"] = strip->getMaxFPS();
  }

  if (server.hasArg("dithering"))
  {
    uint8_t value = String(server.arg("dithering")).toInt();
    //sendInt("dithering", value);
    //broadcastInt("dithering", value);
    strip->setDithering(value);
    answer["Dithering"] = strip->getDithering();
  }

  if (server.hasArg("sunriseset"))
  {
    uint8_t value = String(server.arg("sunriseset")).toInt();
    //sendInt("sunriseset", value);
    //broadcastInt("sunriseset", value);
    strip->getSegment()->sunrisetime = value;
    answer["sunRiseTime"] = strip->getSunriseTime();
  }

  // reset to default values
  if (server.hasArg("resetdefaults"))
  {
    uint8_t value = String(server.arg("resetdefaults")).toInt();
    if (value)
      strip->resetDefaults();
    //sendInt("resetdefaults", 0);
    //broadcastInt("resetdefaults", 0);
    //sendStatus = true;
    strip->setTransition();
  }

  if (server.hasArg("damping"))
  {
    uint8_t value = constrain(String(server.arg("damping")).toInt(), 0, 100);
    //sendInt("damping", value);
    //broadcastInt("damping", value);
    strip->getSegment()->damping = value;
    answer["Damping"] = strip->getDamping();
  }

  if (server.hasArg("addGlitter"))
  {
    uint8_t value = constrain(String(server.arg("addGlitter")).toInt(), 0, 100);
    strip->setAddGlitter(value);
    answer["Glitter_Add"] = strip->getAddGlitter();
  }
  if (server.hasArg("WhiteOnly"))
  {
    uint8_t value = constrain(String(server.arg("WhiteOnly")).toInt(), 0, 100);
    strip->setWhiteGlitter(value);
    answer["Glitter_White"] = strip->getWhiteGlitter();
  }
  if (server.hasArg("onBlackOnly"))
  {
    uint8_t value = constrain(String(server.arg("onBlackOnly")).toInt(), 0, 100);
    strip->setOnBlackOnly(value);
    answer["Glitter_OnBlackOnly"] = strip->getOnBlackOnly();
  }
  if (server.hasArg("glitterChance"))
  {
    uint8_t value = constrain(String(server.arg("glitterChance")).toInt(), 0, 100);
    strip->setChanceOfGlitter(value);
    answer["Glitter_Chance"] = strip->getChanceOfGlitter();
  }


#ifdef DEBUG
  // Testing different Resets
  // can then be triggered via web interface (at the very bottom)
  if (server.hasArg("resets"))
  {
    uint8_t value = String(server.arg("resets")).toInt();
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

  // parameter for number of segemts
  if (server.hasArg("segments"))
  {
    uint16_t value = String(server.arg("segments")).toInt();
    //sendInt("segments", value);
    //broadcastInt("segments", value);
    strip->getSegment()->segments = constrain(value, 1, MAX_NUM_SEGMENTS);
    strip->setTransition();
    answer["Segments"] = strip->getSegments();
  }

  // new parameters, it's time to save
  //shouldSaveRuntime = true;
  /*
  if (sendStatus)
  {
    handleStatus();
  }
  */
  /// strip->setTransition();  <-- this is not wise as it removes the smooth fading for colors. So we need to set it case by case
  String json = "";
  json.reserve(answerObj.measureLength());
  answerObj.printTo(json);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
  jsonBuffer.clear();
}

// if something unknown was called...
void handleNotFound(void)
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleGetModes(void)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(56) + 1070;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();

  JsonObject &modeinfo = root.createNestedObject("modeinfo");
  modeinfo["count"] = strip->getModeCount();

  JsonObject &modeinfo_modes = modeinfo.createNestedObject("modes");
  for (uint8_t i = 0; i < strip->getModeCount(); i++)
  {
    modeinfo_modes[strip->getModeName(i)] = i;
  }
  String message = "";
#ifdef DEBUG
  message.reserve(root.measurePrettyLength());
  root.prettyPrintTo(message);
#else
  message.reserve(root.measureLength());
  root.printTo(message);
#endif
  DEBUGPRNT(message);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void handleGetPals(void)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(56) + 1070;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();

  JsonObject &modeinfo = root.createNestedObject("palinfo");
  modeinfo["count"] = strip->getPalCount();

  JsonObject &modeinfo_modes = modeinfo.createNestedObject("pals");
  for (uint8_t i = 0; i < strip->getPalCount(); i++)
  {
    modeinfo_modes[strip->getPalName(i)] = i;
  }
String message = "";
#ifdef DEBUG
  message.reserve(root.measurePrettyLength());
  root.prettyPrintTo(message);
#else
  message.reserve(root.measureLength());
  root.printTo(message);
#endif
  DEBUGPRNT(message);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void handleStatus(void)
{
  
  JsonObject& answerObj = jsonBuffer.createObject();
  JsonObject& currentStateAnswer = answerObj.createNestedObject("currentState");
  JsonObject& sunriseAnswer = answerObj.createNestedObject("sunRiseState");
  JsonObject& statsAnswer = answerObj.createNestedObject("Stats");
  #ifdef DEBUG
  uint32_t answer_time = micros();
  JsonObject& debugAnswer = answerObj.createNestedObject("ESP_Data");
  #endif

  uint16_t num_leds_on = strip->getLedsOn();

  currentStateAnswer["power"] = strip->getPower();
  if (strip->getPower())
  {
    currentStateAnswer["state"] = "on";
  }
  else
  {
    currentStateAnswer["state"] = "off";
  }
  #ifdef DEBUG
  
  debugAnswer["AnswerTime_ms_01"] = (float)((float)(micros()-answer_time)/1000.0);
  #endif

  currentStateAnswer["Buildversion"] = build_version;
  currentStateAnswer["Git_Revision"] = git_revision;
  currentStateAnswer["Lampname"] = LED_NAME;
  currentStateAnswer["LED_Count"] = strip->getStripLength();
  currentStateAnswer["Lamp_max_current"] = strip->getMilliamps();
  currentStateAnswer["Lamp_max_power"] = strip->getVoltage() * strip->getMilliamps();
  currentStateAnswer["Lamp_current_power"] = strip->getCurrentPower();
  currentStateAnswer["LEDs_On"] = num_leds_on;
  currentStateAnswer["mode_Name"] = strip->getModeName(strip->getMode());
  currentStateAnswer["wsfxmode"] = strip->getModeName(strip->getMode());
  currentStateAnswer["wsfxmode_Num"] = strip->getMode();
  currentStateAnswer["wsfxmode_count"] = strip->getModeCount();
  currentStateAnswer["beat88"] = strip->getBeat88();
  currentStateAnswer["speed"] = strip->getBeat88();
  currentStateAnswer["brightness"] = strip->getBrightness();
  // Palettes and Colors
  currentStateAnswer["palette_count"] = strip->getPalCount();
  currentStateAnswer["palette_num"] = strip->getTargetPaletteNumber();
  currentStateAnswer["palette_name"] = strip->getTargetPaletteName();
  CRGB col = CRGB::Black;
  // We return either black (strip effectively off)
  // or the color of the first pixel....
  for (uint16_t i = 0; i < strip->getStripLength(); i++)
  {
    if (strip->leds[i])
    {
      col = strip->leds[i];
      break;
    }
  }
  currentStateAnswer["rgb"] = (((col.r << 16) | (col.g << 8) | (col.b << 0)) & 0xffffff);
  currentStateAnswer["rgb_red"] = col.r;
  currentStateAnswer["rgb_green"] = col.g;
  currentStateAnswer["rgb_blue"] = col.b;

  #ifdef DEBUG
  debugAnswer["AnswerTime_ms_02"] = (float)((float)(micros()-answer_time)/1000.0);
  #endif

  if (strip->getSegment()->blendType == NOBLEND)
  {
    currentStateAnswer["BlendType"] = "No Blend";
  }
  else if (strip->getSegment()->blendType == LINEARBLEND)
  {
    currentStateAnswer["BlendType"] = "Linear Blend";
  }
  else
  {
    currentStateAnswer["BlendType"] = "Unknown Blend";
  }

  currentStateAnswer["Reverse"] = strip->getReverse();;
  currentStateAnswer["HueChangeInt"] = strip->getHueTime();
  currentStateAnswer["HueDeltaHue"] = strip->getDeltaHue();
  switch(strip->getAutoplay())
  {
    case AUTO_MODE_OFF:
      currentStateAnswer["AutoPlayMode"] = "Off";
    break;
    case AUTO_MODE_UP:
      currentStateAnswer["AutoPlayMode"] = "Up";
    break;
    case AUTO_MODE_DOWN:
      currentStateAnswer["AutoPlayMode"] = "Down";
    break;
    case AUTO_MODE_RANDOM:
      currentStateAnswer["AutoPlayMode"] = "Random";
    break;
    default:
      currentStateAnswer["AutoPlayMode"] = "unknown error";
    break;
  }
  currentStateAnswer["AutoPlayModeIntervall"] = strip->getAutoplayDuration();
  switch(strip->getAutopal())
  {
    case AUTO_MODE_OFF:
      currentStateAnswer["AutoPalette"] = "Off";
    break;
    case AUTO_MODE_UP:
      currentStateAnswer["AutoPalette"] = "Up";
    break;
    case AUTO_MODE_DOWN:
      currentStateAnswer["AutoPalette"] = "Down";
    break;
    case AUTO_MODE_RANDOM:
      currentStateAnswer["AutoPalette"] = "Random";
    break;
    default:
      currentStateAnswer["AutoPalette"] = "unknown error";
    break;
  }
  currentStateAnswer["AutoPaletteInterval"] = strip->getAutoplayDuration();

  #ifdef DEBUG
  debugAnswer["AnswerTime_ms_03"] = (float)((float)(micros()-answer_time)/1000.0);
  #endif

  if (strip->getMode() == FX_MODE_SUNRISE)
  {
    sunriseAnswer["sunRiseMode"] = "Sunrise";
  }
  else if (strip->getMode() == FX_MODE_SUNSET)
  {
    sunriseAnswer["sunRiseMode"] = "Sunset";
  }
  else
  {
    sunriseAnswer["sunRiseMode"] = "None";
  }
  
  if (strip->getMode() == FX_MODE_SUNRISE || strip->getMode() == FX_MODE_SUNSET)
  {
    if(num_leds_on)
    {
      sunriseAnswer["sunRiseActive"] = "on";
    }
    else
    {
      sunriseAnswer["sunRiseActive"] = "off";
    }
  }
  else
  {
    sunriseAnswer["sunRiseActive"] = "off";
  }
    sunriseAnswer["sunRiseCurrStep"] = strip->getCurrentSunriseStep();
    
    sunriseAnswer["sunRiseTotalSteps"] = DEFAULT_SUNRISE_STEPS;
    
    sunriseAnswer["sunRiseTimeToFinish"] = strip->getSunriseTimeToFinish();

    sunriseAnswer["sunRiseTime"] = strip->getSunriseTime();
    
  #ifdef DEBUG
  debugAnswer["DBG_Debug code"] = "On";
  debugAnswer["Chip_CPU_FRQ"] = ESP.getCpuFreqMHz();
  debugAnswer["DBG_Flash Real Size"] = ESP.getFlashChipRealSize();
  debugAnswer["Chip_Free_RAM"] = ESP.getFreeHeap();
  debugAnswer["DBG_Free Sketch Space"] = ESP.getFreeSketchSpace();
  debugAnswer["DBG_Sketch Size"] = ESP.getSketchSize();
  #endif
  
  switch (getResetReason())
  {
  case REASON_DEFAULT_RST:
    
    statsAnswer["Chip_ResetReason"] = "Normal Boot";
    break;
  case REASON_WDT_RST:
    statsAnswer["Chip_ResetReason"] = "WDT Reset";
    break;
  case REASON_EXCEPTION_RST:
    statsAnswer["Chip_ResetReason"] = "Exception";
    break;
  case REASON_SOFT_WDT_RST:
    statsAnswer["Chip_ResetReason"] = "Soft WDT Reset";
    break;
  case REASON_SOFT_RESTART:
    statsAnswer["Chip_ResetReason"] = "Restart";
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    statsAnswer["Chip_ResetReason"] = "Sleep Awake";
    break;
  case REASON_EXT_SYS_RST:
    statsAnswer["Chip_ResetReason"] = "External Trigger";
    break;

  default:
    statsAnswer["Chip_ResetReason"] = "Unknown Cause";
    break;
  }
  statsAnswer["Chip_ID"] = String(ESP.getChipId());
  statsAnswer["WIFI_IP"] =  myIP.toString();
  statsAnswer["WIFI_CONNECT_ERR_COUNT"] = wifi_disconnect_counter;
  statsAnswer["WIFI_SIGNAL"] = String(WiFi.RSSI());  // for #14
  statsAnswer["WIFI_CHAN"] = String(WiFi.channel());  // for #14
  
  #ifdef DEBUG
  answer_time = micros() - answer_time;
  debugAnswer["AnswerTime_ms"] = (float)((float)(answer_time)/1000.0);
  #endif
  statsAnswer["FPS"] = FastLED.getFPS();

  String message;

#ifdef DEBUG
  message.reserve(answerObj.measurePrettyLength());
  answerObj.prettyPrintTo(message);
#else
  message.reserve(answerObj.measureLength());
  answerObj.printTo(message);
#endif
  DEBUGPRNT(message);
  server.sendHeader("Access-Control-Allow-Methods", "*");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
  jsonBuffer.clear();
}

void factoryReset(void)
{
  DEBUGPRNT("Someone requested Factory Reset");
  // on factory reset, each led will be red
  // increasing from led 0 to max.
  for (uint16_t i = 0; i < strip->getStripLength(); i++)
  {
    strip->leds[i] = 0xa00000;
    strip->show();
    delay(2);
  }
  strip->show();
  WiFiManager wifimgr;
  delay(INITDELAY);
  DEBUGPRNT("Reset WiFi Settings");
  wifimgr.resetSettings();
  // wifimgr.erase(); // seems to be removed from WiFiManager
  delay(INITDELAY);
  clearEEPROM();
  WiFi.persistent(false);
  
//reset and try again
  DEBUGPRNT("Reset ESP and start all over...");
  ESP.restart();
}

uint32 getResetReason(void)
{
  return ESP.getResetInfoPtr()->reason;
}

/*
 * Clear the CRC to startup Fresh....
 * Used in case we end up in a WDT reset (either SW or HW)
 */
void clearCRC(void)
{
// invalidating the CRC - in case somthing goes terribly wrong...
  DEBUGPRNT("Clearing CRC in EEPRom");
  EEPROM.begin(strip->getCRCsize());
  for (uint i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
  DEBUGPRNT("Reset ESP and start all over with default values...");
  delay(1000);
  ESP.restart();
}

void clearEEPROM(void)
{
//Clearing EEPROM
  DEBUGPRNT("Clearing EEPROM");
  EEPROM.begin(strip->getSegmentSize());
  for (uint i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}

// Received Factoryreset request.
// To be sure we check the related parameter....
void handleResetRequest(void)
{
  if (server.arg("rst") == "FactoryReset")
  {
    server.send(200, "text/plain", "Will now Reset to factory settings. You need to connect to the WLAN AP afterwards....");
    factoryReset();
  }
  else if (server.arg("rst") == "Defaults")
  {

    strip->setTargetPalette(0);
    strip->setMode(0);
    strip->stop();
    strip->resetDefaults();
    server.send(200, "text/plain", "Strip was reset to the default values...");
    shouldSaveRuntime = true;
  }
}

void setupWebServer(void)
{
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  SPIFFS.begin();
  {
#ifdef DEBUG
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DEBUGPRNT("FS File: " + fileName + ", size: " + String(fileSize) + "\n");
    }
    DEBUGPRNT("\n");
#endif
  }

  server.on("/all", HTTP_GET, []() {
#ifdef DEBUG
    DEBUGPRNT("Called /all!");
#endif
    String json = getFieldsJson(fields, fieldCount);
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", json);
  });

  server.on("/fieldValue", HTTP_GET, []() {
    String name = server.arg("name");
#ifdef DEBUG
    DEBUGPRNT("Called /fieldValue with arg name =");
    DEBUGPRNT(name);
#endif

    String value = getFieldValue(name, fields, fieldCount);
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", value);
  });

  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm"))
      server.send(404, "text/plain", "FileNotFound");
  });

  // keepAlive
  server.on("/ping", HTTP_GET, []() {
    server.send(200, "text/plain", "OK");
  });

  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  },
            handleFileUpload);

  server.on("/set", handleSet);
  server.on("/getmodes", handleGetModes);
  server.on("/getpals", handleGetPals);
  server.on("/status", handleStatus);
  server.on("/reset", handleResetRequest);
  server.onNotFound(handleNotFound);

  server.on("/", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "" );
  });


  server.serveStatic("/", SPIFFS, "/", "max-age=86400");
  delay(INITDELAY);
  server.begin();

  showInitColor(CRGB::Yellow);
  delay(INITDELAY);
#ifdef DEBUG
  DEBUGPRNT("HTTP server started.\n");
#endif
  webSocketsServer = new WebSocketsServer(81);
  webSocketsServer->begin();
  webSocketsServer->onEvent(webSocketEvent);

  showInitColor(CRGB::Green);
  delay(INITDELAY);
#ifdef DEBUG
  DEBUGPRNT("webSocketServer started.\n");
#endif
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if(webSocketsServer == NULL)
  {
    return;
  }
  DEBUGPRNT("Checking the websocket event!");
  server.arg(1);
  if(type == WStype_TEXT)
  {
    #ifdef DEBUG
    Serial.printf("[%u] get Text: %s\n", num, payload);
    #endif
    JsonObject& received = jsonBuffer.parse(payload);
    if(received.success())
    {
      received["Received"] = "OK";
      String myJSON;
      #ifdef DEBUG
      received.prettyPrintTo(myJSON);
      #else
      received.printTo(myJSON);
      #endif
      webSocketsServer->sendTXT(num, myJSON);
      DEBUGPRNT("WEBSOCKET: Received JSON:" + myJSON);
      #ifdef DEBUG
      for(JsonPair& p : received)
      {
        DEBUGPRNT("Key: " + String(p.key));
        DEBUGPRNT("Value: " + String(p.value.asString()));
      }
      #endif
    }
    else
    {
      #ifdef DEBUG
      webSocketsServer->sendTXT(num, "WS: Received non decodable value: \n\t" + String((const char *)payload));
      #else
      webSocketsServer->sendTXT(num, "{\"Received\":\"Failed\"}");
      #endif

    }
    jsonBuffer.clear();
  }
  else if(type == WStype_CONNECTED)
  {
    #ifdef DEBUG
      IPAddress ip = webSocketsServer->remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      // send message to client
      webSocketsServer->sendTXT(num, "Welcome to the LED Control world!");
    #endif
  }
  else if(type == WStype_DISCONNECTED)
  {
    #ifdef DEBUG
    Serial.printf("[%u] Disconnected!\n", num);
    #endif
  }
  else
  {
    #ifdef DEBUG
    webSocketsServer->sendTXT(num, "Don't know what you sent.");
    #endif
  }
}

#ifdef HAS_KNOB_CONTROL

uint16_t maxVal = 65535;
uint16_t minVal = 0;
uint16_t curVal = 0;
uint16_t steps  = 1;

void setupKnobControl(void)
{ 


  DEBUGPRNT("setup display...");
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Filedtypes:
  m_fieldtypes = new fieldtypes[fieldCount];
  set_fieldTypes(m_fieldtypes);
  DEBUGPRNT("setup done...");


}
#endif

#ifdef HAS_KNOB_CONTROL
uint8_t drawtxtline10(uint8_t y, uint8_t fontheight, String txt)
{
  if(txt == "") return y;
  uint8_t txtLines = 1;
  txtLines = (((display.getStringWidth(txt) - 1) / 128) + 1) * fontheight;
  display.drawStringMaxWidth (0,  y, 128, txt);
  y+=txtLines;
  return y;
}

#endif

enum displayStates {
  Display_Off,
  Display_ShowInfo,
  Display_ShowMenu,
  Display_ShowSectionMenue,
  Display_ShowBoolMenu,
  Display_ShowNumberMenu,
  Display_ShowSelectMenue
} mDisplayState;


// setup network and output pins
void setup()
{
  // Sanity delay to get everything settled....
  delay(50);

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

  DEBUGPRNT("\n");
  DEBUGPRNT(F("Booting"));

  DEBUGPRNT("");
  DEBUGPRNT(F("Checking boot cause:"));

 

  switch (getResetReason())
  {
  case REASON_DEFAULT_RST:
    DEBUGPRNT(F("\tREASON_DEFAULT_RST: Normal boot"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_DEFAULT_RST"));
    display.display();
    break;
  case REASON_WDT_RST:
    DEBUGPRNT(F("\tREASON_WDT_RST"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_WDT_RST"));
    display.display();
    clearCRC(); // should enable default start in case of
    break;
  case REASON_EXCEPTION_RST:
    DEBUGPRNT(F("\tREASON_EXCEPTION_RST"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_EXCEPTION_RST"));
    display.display();
    clearCRC();
    break;
  case REASON_SOFT_WDT_RST:
    DEBUGPRNT(F("\tREASON_SOFT_WDT_RST"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_SOFT_WDT_RST"));
    display.display();
    clearCRC();
    break;
  case REASON_SOFT_RESTART:
    DEBUGPRNT(F("\tREASON_SOFT_RESTART"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_SOFT_RESTART"));
    display.display();
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    DEBUGPRNT(F("\tREASON_DEEP_SLEEP_AWAKE"));
    cursor = drawtxtline10(cursor, font_height, F("REASON_DEEP_SLEEP_AWAKE"));
    display.display();
    break;
  case REASON_EXT_SYS_RST:
    DEBUGPRNT(F("\n\tREASON_EXT_SYS_RST: External trigger..."));
    cursor = drawtxtline10(cursor, font_height, F("REASON_EXT_SYS_RST"));
    display.display();
    break;

  default:
    DEBUGPRNT(F("\tUnknown cause..."));
    cursor = drawtxtline10 (cursor, 10, F("Unknown cause..."));
    display.display();
    break;
  }
  cursor = drawtxtline10 (cursor, 10, F("LED Stripe init"));
  display.display();
  

  stripe_setup(LED_COUNT,
               STRIP_MAX_FPS,
               STRIP_VOLTAGE,
               STRIP_MILLIAMPS,
               RainbowColors_p,
               F("Rainbow Colors"),
               UncorrectedColor); //TypicalLEDStrip);

  EEPROM.begin(strip->getSegmentSize());

  cursor = drawtxtline10(cursor, font_height, F("WiFi-Setup"));
  display.display();
  setupWiFi();
  cursor = drawtxtline10(cursor, font_height, F("WebServer Setup"));
  display.display();
  setupWebServer();

  if (!MDNS.begin(LED_NAME)) {
    DEBUGPRNT("Error setting up MDNS responder!");
    //ESP.restart();
  }
  else
  {
    MDNS.addService("http", "tcp", 80);
    DEBUGPRNT("mDNS responder started");
  }
  cursor = drawtxtline10(cursor, font_height, F("OTA Setup"));
  display.display();
  initOverTheAirUpdate();
    
  readRuntimeDataEEPROM(); 
  
  myIP = WiFi.localIP();
  DEBUGPRNT("Going to show IP Address " + myIP.toString()); 

  delay(KNOB_BOOT_DELAY);
  display.clear();
  cursor = 0;
  cursor = drawtxtline10(cursor, font_height, "Boot von " + String(LED_NAME) + " fertig!");
  cursor = drawtxtline10(cursor, font_height, "Name: " + String(LED_NAME));
  cursor = drawtxtline10(cursor, font_height, "IP: " + myIP.toString());
  cursor = drawtxtline10(cursor, font_height, "LEDs: " + String(LED_COUNT));
  display.display();
  delay(KNOB_BOOT_DELAY);

#else // HAS_KNOB_CONTROL
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  #endif

  DEBUGPRNT("\n");
  DEBUGPRNT(F("Booting"));

  DEBUGPRNT("");
  DEBUGPRNT(F("Checking boot cause:"));
  
  switch (getResetReason())
  {
  case REASON_DEFAULT_RST:
    DEBUGPRNT(F("\tREASON_DEFAULT_RST: Normal boot"));
    
    break;
  case REASON_WDT_RST:
    DEBUGPRNT(F("\tREASON_WDT_RST"));
    
    clearCRC(); // should enable default start in case of
    break;
  case REASON_EXCEPTION_RST:
    DEBUGPRNT(F("\tREASON_EXCEPTION_RST"));
    
    clearCRC();
    break;
  case REASON_SOFT_WDT_RST:
    DEBUGPRNT(F("\tREASON_SOFT_WDT_RST"));
    
    clearCRC();
    break;
  case REASON_SOFT_RESTART:
    DEBUGPRNT(F("\tREASON_SOFT_RESTART"));
    
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    DEBUGPRNT(F("\tREASON_DEEP_SLEEP_AWAKE"));
    
    break;
  case REASON_EXT_SYS_RST:
    DEBUGPRNT(F("\n\tREASON_EXT_SYS_RST: External trigger..."));
    
    break;

  default:
    DEBUGPRNT(F("\tUnknown cause..."));
    
    break;
  }
  delay(1000);
   

  stripe_setup(LED_COUNT,
               STRIP_MAX_FPS,
               STRIP_VOLTAGE,
               STRIP_MILLIAMPS,
               RainbowColors_p,
               F("Rainbow Colors"),
               UncorrectedColor); //TypicalLEDStrip);

  EEPROM.begin(strip->getSegmentSize());
  
  // internal LED can be light up when current is limited by FastLED
  
  pinMode(2, OUTPUT);


  //EEPROM.begin(strip->getSegmentSize());
  delay(1000);
  
  setupWiFi();

  setupWebServer();

  if (!MDNS.begin(LED_NAME)) {
    DEBUGPRNT("Error setting up MDNS responder!");
    //ESP.restart();
  }
  else
  {
    MDNS.addService("http", "tcp", 80);
    DEBUGPRNT("mDNS responder started");
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
    DEBUGPRNT("Init done - fading green out");
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
  myIP = WiFi.localIP();
  DEBUGPRNT("Going to show IP Address " + myIP.toString());
  if(LED_COUNT >= 40)
  {
    // can show the complete IP Address on the first 40 LEDs
    for(uint8_t j=0; j<4; j++)
    {
      for(uint8_t i=0; i<8; i++)
      {
        if((myIP[j] >> i) & 0x01)
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

  delay(4000);


#ifdef DEBUG
  DEBUGPRNT("Init finished.. Read runtime data");
#endif
  readRuntimeDataEEPROM();
#ifdef DEBUG
  DEBUGPRNT("Runtime Data loaded");
  FastLED.countFPS();
#endif
  //setEffect(FX_NO_FX);
#endif // HAS_KNOB_CONTROL

  //FastLED.setMaxRefreshRate(120);

}

#ifdef HAS_KNOB_CONTROL


uint8_t get_next_field(uint8_t curr_field, bool up, fieldtypes *m_fieldtypes)
{
  uint8_t ret = curr_field;
  uint8_t first_field = 0;
  uint8_t last_field = fieldCount-1;
  for(uint8_t i=0; i<fieldCount; i++)
  {
    if(m_fieldtypes[i] <= SelectFieldType)
    {
      last_field = i;
    }
    if(m_fieldtypes[fieldCount-1-i] <= SelectFieldType)
    {
      first_field = fieldCount-1-i;
    }
  }
  uint8_t sanity = 0;
  while(ret == curr_field || m_fieldtypes[ret] > SelectFieldType)
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


uint16_t setEncoderValues(uint8_t curr_field, fieldtypes * m_fieldtypes)
{
  uint16_t curr_value = 0;
  switch (m_fieldtypes[curr_field])
  {
    case TitleFieldType :
      // nothing?
    break;
    case NumberFieldType :
      curr_value = (uint16_t)strtoul(fields[curr_field].getValue().c_str(), NULL, 10);
      steps = (fields[curr_field].max - fields[curr_field].min) / 100;
      if(steps == 0) steps = 1;
      maxVal = fields[curr_field].max;
      minVal = fields[curr_field].min;
      curVal = curr_value;
    break;
    case BooleanFieldType :
      curr_value = (uint16_t)strtoul(fields[curr_field].getValue().c_str(), NULL, 10);
      steps = 1;
      maxVal = 1;
      minVal = 0;
      curVal = curr_value;
    break;
    case SelectFieldType :
      curr_value =(uint16_t)strtoul(fields[curr_field].getValue().c_str(), NULL, 10);
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


uint32_t last_control_operation = 0;
bool display_was_off = false;
uint8_t TimeoutBar = 0;

void showDisplay(uint8_t curr_field, fieldtypes *fieldtype)
{
  static bool toggle;
  EVERY_N_MILLISECONDS(KNOB_CURSOR_BLINK)
  {
    toggle = !toggle;
  }
  EVERY_N_MILLISECONDS(1000/KNOB_DISPLAY_FPS)
  {
    uint16_t val = (uint16_t)strtoul(fields[curr_field].getValue().c_str(), NULL, 10);
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
          uint8_t prev_field = get_next_field(curr_field, false, fieldtype);
          uint8_t pre_prev_field = get_next_field(prev_field, false, fieldtype);
          uint8_t next_field = get_next_field(curr_field, true, fieldtype);
          uint8_t next_next_field = get_next_field(next_field, true, fieldtype);
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0,  0, F("Menu:")); 
          //display.setTextAlignment(TEXT_ALIGN_RIGHT);
          //display.drawString(128,  53, myIP.toString());    
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
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label + ": ");
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
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label + ": ");
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.setFont(ArialMT_Plain_16);
        display.drawProgressBar(0, 28, 127, 20, map(val, fields[curr_field].min, fields[curr_field].max, 0, 100));
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setColor((OLEDDISPLAY_COLOR)2);
        display.drawString(63, 30, fields[curr_field].getValue());
        display.setColor((OLEDDISPLAY_COLOR)1);
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, 52, String(fields[curr_field].min));
        display.setTextAlignment(TEXT_ALIGN_RIGHT); 
        display.drawString(127, 52, String(fields[curr_field].max));
          
      break;
      case Display_ShowSelectMenue:
      {
        display.drawStringMaxWidth(0, 0, 128, fields[curr_field].label + ": ");
        StaticJsonBuffer<1600> myJsonBuffer;
        String json = "[";
        json += fields[curr_field].getOptions();
        json += "]";

        JsonArray& myValues = myJsonBuffer.parseArray(json.c_str(),2);
        
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
          display.drawStringMaxWidth(0, 25, 128, " -");
          display.drawStringMaxWidth(10, 25, 128, myValues[p_val]);
        }
        if(val != n_val) {
          display.drawStringMaxWidth(0, 45, 128, " +");
          display.drawStringMaxWidth(10, 45, 128, myValues[n_val]);
        }
        if(toggle)
        {
          display.fillRect(0,36,128,11);
          display.setColor((OLEDDISPLAY_COLOR)0);
        }
        display.drawStringMaxWidth(0, 35, 128, " >");
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

       /*  if(WiFiConnected)
        {
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(0,   10, WiFi.SSID());
        }
        else
        {
          display.drawString(0,   10, "No Network");
        } */
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0,  20, "FPS:");
        display.drawString(0,  30, "Leds on:");
        display.drawString(0,  40, "M:");
        display.drawString(0,  50, "C:");

        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        static uint16_t FPS = 0;
        static uint16_t num_leds_on = strip->getLedsOn(); // fixes issue #18
        EVERY_N_MILLISECONDS(200)
        {
          FPS = strip->myFPS;
          num_leds_on = strip->getLedsOn();
        }
        if(strip->getPower())
        {        
          display.drawString(127,  20, String(strip->myFPS));
          display.drawString(127,  30, String(num_leds_on));
        }
        else
        {
          display.drawString(127,  20, "Off");
          display.drawString(127,  30, "Off");
        }
        display.drawString(127,  40, strip->getModeName(strip->getMode()));
        display.drawString(127,  50, strip->getTargetPaletteName());
      default:
        display.displayOn();
      break;
    }
    if(WiFiConnected)
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
      display.drawLine(115, 10, 125, 0);
      display.drawLine(125, 10, 115, 0);
    }
    /* 
    uint8_t br64 =          map8(strip->getBrightness(), 0, 64);
    uint8_t sp64 = (uint8_t)map (strip->getSpeed(), BEAT88_MIN, BEAT88_MAX, 0, 64);
    display.drawVerticalLine(0,   63-br64, br64);
    display.drawVerticalLine(127, 63-sp64, sp64); 
    */
    display.drawHorizontalLine(64-(TimeoutBar/2),63, TimeoutBar);
    display.setColor((OLEDDISPLAY_COLOR)1);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
  }
}


void knob_service(uint32_t now)
{
 

  

  static uint8_t curr_field = get_next_field(0, true, m_fieldtypes);
  static uint32_t last_btn_press = 0;
  
  static uint16_t old_val = 0;

  static bool in_submenu = false;
  static bool newfield_selected = false;
 
  if (digitalRead(KNOB_C_BTN) == LOW && now > last_btn_press + KNOB_BTN_DEBOUNCE)
  {
    last_btn_press = now;
    
    if(display_was_off)
    {
      display_was_off = false;
      last_control_operation = now - KNOB_TIMEOUT_OPERATION - (KNOB_TIMEOUT_OPERATION/10);
      mDisplayState = Display_ShowInfo;
      showDisplay(curr_field, m_fieldtypes);
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
      old_val = setEncoderValues(curr_field, m_fieldtypes);
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
          val = get_next_field(curr_field, true, m_fieldtypes);
        }
        else
        {
          val = get_next_field(curr_field, false, m_fieldtypes);
        }
        curr_field = val;
        newfield_selected = true;
      }
      else
      {
        if(newfield_selected)
        {
          old_val = setEncoderValues(curr_field, m_fieldtypes);
          newfield_selected = false;
        }
        if(fields[curr_field].setValue)
        {
          fields[curr_field].setValue(val);
        }
      }
      old_val = val;
    }
    curVal = val;
    
    if(now > last_control_operation + KNOB_TIMEOUT_DISPLAY)
    {
      mDisplayState = Display_Off;
      display_was_off = true;
    }
    else if(now > last_control_operation + KNOB_TIMEOUT_OPERATION)
    {
      TimeoutBar = map(now-last_control_operation, 0, KNOB_TIMEOUT_DISPLAY, 127,0);
      curr_field = get_next_field(0, true, m_fieldtypes);
      in_submenu = true;
      old_val = setEncoderValues(curr_field, m_fieldtypes);
      mDisplayState = Display_ShowInfo;
      if((strip->getAutoplay() || strip->getAutopal()) && strip->getPower()) last_control_operation = now - KNOB_TIMEOUT_OPERATION - (KNOB_TIMEOUT_OPERATION/10); // keeps the display on as long as we change automatically the mode or the palette
    }
    else
    {
      TimeoutBar = map(now-last_control_operation, 0, KNOB_TIMEOUT_OPERATION, 127,0);
      if(!in_submenu)
      {
        mDisplayState = Display_ShowMenu;
      }
      else
      {
        switch (m_fieldtypes[curr_field])
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
    showDisplay(curr_field, m_fieldtypes);
  }
}
#endif



// request receive loop
void loop()
{
  uint32_t now = millis();
  static uint32_t wifi_check_time = 0;


#ifdef DEBUG
  static unsigned long last_status_msg = 0;
#endif
  if (OTAisRunning)
    return;
    // if someone requests a call to the factory reset...
    //static bool ResetRequested = false;

#ifdef DEBUG
  // Debug Watchdog. to be removed for "production".
  if (now - last_status_msg > 10000)
  {
    String msg = "";
    last_status_msg = now;
    if(strip->getPower())
    {
      msg += "Strip is ON";
    }
    else
    {
      msg += "Strip is OFF";
    }
    if(strip->isRunning())
    {
      msg += " and running.\n";
    }
    else
    {
      msg += " and paused.\n";
    }
    msg += "\t\tWS2812FX mode #" + String(strip->getMode()) + " - " + strip->getModeName(strip->getMode());
    msg += "\n\t\tC-Pal: " + strip->getCurrentPaletteName() + "\tT-Pal: " + strip->getTargetPaletteName() + "\n\t\tFPS: " + String(FastLED.getFPS());
    DEBUGPRNT(msg);
  }
#endif

  #ifndef HAS_KNOB_CONTROL
  // Checking WiFi state every WIFI_TIMEOUT
  // Reset on disconnection
  if (now > wifi_check_time)
  {
    //DEBUGPRNT("Checking WiFi... ");
    if (WiFi.status() != WL_CONNECTED)
    {
      DEBUGPRNT("Lost Wifi Connection. Counter is " + String(wifi_err_counter));
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
      DEBUGPRNT("Trying to reconnect now...");
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
      DEBUGPRNT("Reconnecting failed, resetting ESP....");
      ESP.restart();
    }

    wifi_check_time = now + (WIFI_TIMEOUT);
  }

  ArduinoOTA.handle(); // check and handle OTA updates of the code....

  webSocketsServer->loop();

  server.handleClient();

  strip->service();

  MDNS.update();

  EVERY_N_MILLIS(50)
  {
    checkSegmentChanges();
  }

  EVERY_N_MILLIS(EEPROM_SAVE_INTERVAL_MS)
  {
    saveEEPROMData();
    shouldSaveRuntime = false;
  }
  #else

    
    
    // Checking WiFi state every WIFI_TIMEOUT
    // since we have Knob-Control. We do not care about "No Connection"
  if (now > wifi_check_time)
  {
    //DEBUGPRNT("Checking WiFi... ");
    if (WiFi.status() != WL_CONNECTED)
    {
      last_control_operation = now;
      DEBUGPRNT("Lost Wifi Connection. Counter is " + String(wifi_err_counter));
      wifi_err_counter+=2;
      wifi_disconnect_counter+=4;
      WiFiConnected = false;
    }
    else
    {
      if(wifi_err_counter > 0) wifi_err_counter--;
      if(wifi_disconnect_counter > 0) wifi_disconnect_counter--;
      WiFiConnected = true;
    }

    if(wifi_err_counter > 20)
    {
      DEBUGPRNT("Trying to reconnect now...");
      WiFi.mode(WIFI_OFF);
      setupWiFi();
    }
    wifi_check_time = now + (WIFI_TIMEOUT);
  }

  if(WiFiConnected)
  {
    ArduinoOTA.handle(); // check and handle OTA updates of the code....
    
    if(webSocketsServer != NULL)
      webSocketsServer->loop();

    server.handleClient();

    MDNS.update();
  }

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

  knob_service(now);

  #endif
}
