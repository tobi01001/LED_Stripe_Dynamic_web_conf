#include "webserver_manager.h"
#include "defaults.h"
#include <LittleFS.h>
#include <FastLED.h>

// External dependencies from main file
extern const char * build_version;
extern const char * git_revision;
extern class LEDStripe * strip;
extern bool getAllValuesJSON(JsonObject& obj);
extern bool getAllValuesJSONArray(JsonArray& arr);
extern void updateConfigFile(void);
extern uint16_t wifi_disconnect_counter;
extern IPAddress gateway_ip;
extern uint8_t status_counter;
extern String cStrReason;
extern String lStrReason;
extern struct ESPrunTime {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint16_t days;
} mESPrunTime;

// External function dependencies
extern bool isField(const char* name);
extern void setFieldValue(const char* name, uint16_t value);
extern uint16_t getFieldValue(const char* name);
extern class Field getField(const char* name);
extern bool newSolidColor;
extern CRGB pLeds[];

// Constants
#ifndef DEFAULT_MAX_WS_CLIENTS
#define DEFAULT_MAX_WS_CLIENTS 8
#endif

#ifndef FX_MODE_VOID
#define FX_MODE_VOID 0
#endif

#ifndef FX_MODE_STATIC  
#define FX_MODE_STATIC 1
#endif

#ifndef FX_MODE_SUNRISE
#define FX_MODE_SUNRISE 50
#endif

#ifndef FX_MODE_SUNSET
#define FX_MODE_SUNSET 51
#endif

#ifndef DEFAULT_SUNRISE_STEPS
#define DEFAULT_SUNRISE_STEPS 100
#endif

// Field type enum
enum FieldType {
  TitleFieldType = 0,
  NumberFieldType = 1, 
  BooleanFieldType = 2,
  SelectFieldType = 3,
  ColorFieldType = 4,
  SectionFieldType = 5
};

// Reset control enum
extern enum doResets {
  LED_CTRL_NO_RESET,
  LED_CTRL_SAVE_RESTART,  
  LED_CTRL_RESET_DEFAULTS,
  LED_CTRL_RESET_FACTORY
} ledCtrlDoResets;

#ifdef HAS_KNOB_CONTROL
extern bool WiFiConnected;
#endif

// File editor from library
extern "C" {
#include "../lib/FileEditor/FileEditorLittleFS.h"
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

void handleWebSocketMaintenance(void)
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

void setupWebServer(void)
{
  extern void showInitColor(CRGB Color);
  
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