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