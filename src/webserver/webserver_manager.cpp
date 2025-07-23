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
extern struct ESPrunTime {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint16_t days;
} mESPrunTime;

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