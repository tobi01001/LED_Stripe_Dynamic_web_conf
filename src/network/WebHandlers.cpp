/*************************************************************
 * WebHandlers.cpp - HTTP request handlers implementation
 *************************************************************/

#include "WebHandlers.h"
#include "NetworkManager.h"
#include "../config/ConfigManager.h"
#include "../Constants.h"
#include "../LED_strip/led_strip.h"
#include "../lib/FileEditor/FileEditor.h"

// External LED array
extern CRGB pLeds[];

// Static instance
WebHandlers* WebHandlers::instance = nullptr;

WebHandlers::WebHandlers(NetworkManager* network, ConfigManager* config) 
    : networkManager(network)
    , configManager(config) 
{
    instance = this;
}

WebHandlers::~WebHandlers() {
    instance = nullptr;
}

void WebHandlers::setupHandlers(AsyncWebServer* server) {
    if (!server) return;

    // Main configuration endpoint
    server->on("/all", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!LittleFS.exists(F("/config_all.json"))) {
            if (instance && instance->configManager) {
                instance->configManager->updateConfigFile();
            }
        }
        request->send(LittleFS, F("/config_all.json"), F("application/json"));
    });

    // Values endpoint
    server->on("/allvalues", HTTP_GET, handleAllValues);
    
    // Field value endpoint
    server->on("/fieldValue", HTTP_GET, handleFieldValue);
    
    // Keep alive endpoint
    server->on("/ping", HTTP_GET, handlePing);
    
    // Main API endpoints
    server->on("/set", HTTP_GET, handleSet);
    server->on("/getmodes", HTTP_GET, handleGetModes);
    server->on("/getpals", HTTP_GET, handleGetPalettes);
    server->on("/status", HTTP_GET, handleStatus);
    server->on("/reset", HTTP_GET, handleReset);
    
    // File editor (for development)
    server->addHandler(new FileEditor(LittleFS, String(), String()));
    
    // Default handler
    server->onNotFound(handleNotFound);
}

void WebHandlers::handleStatus(AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    
    JsonObject& currentState = root.createNestedObject(F("currentState"));
    JsonObject& sunriseState = root.createNestedObject(F("sunRiseState"));
    JsonObject& stats = root.createNestedObject(F("Stats"));

    if (!strip) {
        sendJsonError(request, F("LED strip not initialized"));
        return;
    }

    // Get current state from LED strip
    if (!getAllValuesJSON(currentState)) {
        currentState[F("ValueError")] = F("Could not read any Name Value Pair!");
    }
    
    // Add system information
    currentState[F("buildVersion")] = BUILD_VERSION;
    currentState[F("gitRevision")] = BUILD_GITREV;
    currentState[F("lampName")] = LED_NAME;
    currentState[F("ledCount")] = LED_COUNT;
    currentState[F("lampMaxCurrent")] = strip->getMilliamps();
    currentState[F("lampMaxPower")] = strip->getVoltage() * strip->getMilliamps();
    currentState[F("lampCurrentPower")] = strip->getCurrentPower();
    
    uint16_t ledsOn = strip->getLedsOn();
    currentState[F("ledsOn")] = ledsOn;

    // Get first lit LED color for RGB status
    CRGB color = CRGB::Black;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        if (strip->_bleds[i]) {
            color = strip->_bleds[i];
            break;
        }
    }

    uint32_t rgbValue = (((color.r << 16) | (color.g << 8) | (color.b << 0)) & 0xffffff);
    currentState[F("rgb")] = rgbValue;
    currentState[F("rgb_red")] = color.r;
    currentState[F("rgb_green")] = color.g;
    currentState[F("rgb_blue")] = color.b;

    // Sunrise/sunset state
    uint8_t mode = strip->getMode();
    if (mode == FX_MODE_SUNRISE) {
        sunriseState[F("sunRiseMode")] = F("Sunrise");
    } else if (mode == FX_MODE_SUNSET) {
        sunriseState[F("sunRiseMode")] = F("Sunset");
    } else {
        sunriseState[F("sunRiseMode")] = F("None");
    }
    
    bool sunriseActive = ledsOn && (mode == FX_MODE_SUNRISE || mode == FX_MODE_SUNSET);
    sunriseState[F("sunRiseActive")] = sunriseActive ? F("on") : F("off");
    sunriseState[F("sunRiseCurrStep")] = strip->getCurrentSunriseStep();
    sunriseState[F("sunRiseTotalSteps")] = EffectConstants::DEFAULT_SUNRISE_STEPS;
    sunriseState[F("sunRiseTimeToFinish")] = strip->getSunriseTimeToFinish();
    sunriseState[F("sunRiseTime")] = strip->getSunriseTime();

    // System statistics
    stats[F("chip_ResetReason")] = ESP.getResetInfo();
    
    uint32_t free = 0;
    uint16_t max = 0;
    uint8_t frag = 0;
    ESP.getHeapStats(&free, &max, &frag);
    stats[F("chip_FreeHeap")] = free;
    stats[F("chip_MaxHeap")] = max;
    stats[F("chip_HeapFrag")] = frag;
    
    ESP.resetFreeContStack();
    stats[F("chip_FreeStack")] = ESP.getFreeContStack();
    stats[F("chip_ID")] = ESP.getChipId();
    
    // Network statistics
    if (instance && instance->networkManager) {
        const auto& wifiStats = instance->networkManager->getWiFiStats();
        stats[F("wifi_IP")] = WiFi.localIP().toString();
        stats[F("wifi_CONNECT_ERR_COUNT")] = wifiStats.disconnectCounter;
        stats[F("wifi_SIGNAL")] = WiFi.RSSI();
        stats[F("wifi_CHAN")] = WiFi.channel();
        stats[F("wifi_GATEWAY")] = wifiStats.gatewayIP.toString();
        stats[F("wifi_BSSID")] = WiFi.BSSIDstr();
        stats[F("wifi_BSSIDCRC")] = strip->calc_CRC16(0x5555, (unsigned char*)WiFi.BSSID(), 6);
    }
    
    static uint8_t statusCounter = 0;
    stats[F("statsCounter")] = sin8(statusCounter++);
    stats[F("fps")] = FastLED.getFPS();

    response->setLength();
    request->send(response);
}

void WebHandlers::handleSet(AsyncWebServerRequest* request) {
    if (!strip) {
        sendJsonError(request, F("LED strip not initialized"));
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    JsonObject& currentState = root.createNestedObject(F("currentState"));

    bool parametersSet = false;
    uint32_t color = ColorConstants::COLOR_BLACK;

    // Process all parameters
    for (uint8_t i = 0; i < request->params(); i++) {
        AsyncWebParameter* param = request->getParam(i);
        String paramName = param->name();
        String paramValue = param->value();

        if (isField(paramName.c_str())) {
            Field field = getField(paramName.c_str());
            
            if (field.type == ColorFieldType) {
                // Handle color parameters
                if (paramValue == "solidColor") {
                    uint8_t r = 0, g = 0, b = 0;
                    if (request->hasParam("r")) r = constrain(request->getParam("r")->value().toInt(), 0, 255);
                    if (request->hasParam("g")) g = constrain(request->getParam("g")->value().toInt(), 0, 255);
                    if (request->hasParam("b")) b = constrain(request->getParam("b")->value().toInt(), 0, 255);
                    color = (((r << 16) | (g << 8) | (b << 0)) & 0xffffff);
                } else {
                    color = parseColorParameter(paramValue);
                }
                
                strip->setSolidColor(color);
                strip->setColor(color);
                currentState[field.name] = color;
                parametersSet = true;
            } else {
                // Handle normal parameters
                uint16_t value = constrain(paramValue.toInt(), field.min, field.max);
                setFieldValue(paramName.c_str(), value);
                currentState[paramName] = getFieldValue(paramName.c_str());
                parametersSet = true;
            }
        }
    }

    // Handle special pixel operations
    if (request->hasParam(F("pi"))) {
        // Single pixel
        uint16_t pixel = constrain(request->getParam(F("pi"))->value().toInt(), 0, LED_COUNT_TOT - 1);
        strip->getSegment()->mode = FX_MODE_VOID;
        strip->setPower(true);
        pLeds[pixel] = CRGB(color);
        
        currentState[F("pixel")] = pixel;
        currentState[F("color")] = String(color, HEX);
        currentState[F("effect")] = strip->getModeName(FX_MODE_VOID);
        currentState[F("power")] = F("on");
        parametersSet = true;
    }
    else if (request->hasParam(F("rnS")) && request->hasParam(F("rnE"))) {
        // Pixel range
        uint16_t start = constrain(request->getParam(F("rnS"))->value().toInt(), 0, LED_COUNT_TOT - 1);
        uint16_t end = constrain(request->getParam(F("rnE"))->value().toInt(), start, LED_COUNT_TOT - 1);
        
        strip->getSegment()->mode = FX_MODE_VOID;
        strip->setPower(true);
        for (uint16_t i = start; i <= end; i++) {
            pLeds[i] = CRGB(color);
        }
        
        currentState[F("rnS")] = start;
        currentState[F("rnE")] = end;
        currentState[F("color")] = String(color, HEX);
        currentState[F("effect")] = strip->getModeName(FX_MODE_VOID);
        currentState[F("power")] = F("on");
        parametersSet = true;
    }
    else if (request->hasParam(F("rgb"))) {
        // Solid color for entire strip
        strip->setColor(color);
        strip->setPower(true);
        strip->setMode(FX_MODE_STATIC);
        
        currentState[F("color")] = String(color, HEX);
        currentState[F("effect")] = strip->getModeName(FX_MODE_STATIC);
        currentState[F("power")] = F("on");
        parametersSet = true;
    }

    if (!parametersSet) {
        currentState[F("message")] = F("No valid parameters provided");
    }

    // Mark configuration for saving
    if (instance && instance->configManager && parametersSet) {
        instance->configManager->markForSave();
    }

    response->setLength();
    request->send(response);
}

void WebHandlers::handleGetModes(AsyncWebServerRequest* request) {
    if (!strip) {
        sendJsonError(request, F("LED strip not initialized"));
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    JsonObject& modeInfo = root.createNestedObject(F("modeinfo"));
    
    modeInfo[F("count")] = strip->getModeCount();
    JsonObject& modes = modeInfo.createNestedObject(F("modes"));
    
    for (uint8_t i = 0; i < strip->getModeCount(); i++) {
        modes[strip->getModeName(i)] = i;
    }
    
    response->setLength();
    request->send(response);
}

void WebHandlers::handleGetPalettes(AsyncWebServerRequest* request) {
    if (!strip) {
        sendJsonError(request, F("LED strip not initialized"));
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    JsonObject& palInfo = root.createNestedObject(F("palinfo"));
    
    palInfo[F("count")] = strip->getPalCount();
    JsonObject& palettes = palInfo.createNestedObject(F("pals"));
    
    for (uint8_t i = 0; i < strip->getPalCount(); i++) {
        palettes[strip->getPalName(i)] = i;
    }
    
    response->setLength();
    request->send(response);
}

void WebHandlers::handleReset(AsyncWebServerRequest* request) {
    if (!instance || !instance->configManager) {
        sendJsonError(request, F("Configuration manager not available"));
        return;
    }

    if (request->hasParam(F("rst"))) {
        String resetType = request->getParam(F("rst"))->value();
        
        if (resetType == F("Restart")) {
            request->redirect("/");
            instance->configManager->requestReset(ConfigManager::SAVE_AND_RESTART);
        }
        else if (resetType == F("FactoryReset")) {
            request->redirect("/");
            instance->configManager->requestReset(ConfigManager::FACTORY_RESET);
        }
        else if (resetType == F("Defaults")) {
            request->redirect("/");
            instance->configManager->requestReset(ConfigManager::RESET_DEFAULTS);
        }
        else {
            request->send(400, F("text/plain"), F("Invalid reset type"));
        }
    } else {
        request->send(200, F("text/plain"), 
            F("/reset needs a Parameter:\n"
              "\t - Restart (Will save EEPROM and restart the ESP)\n"
              "\t - Defaults (Will reset to default values and restart the ESP)\n"
              "\t - FactoryReset (Will reset everything to Factory values)"));
    }
}

void WebHandlers::handleAllValues(AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    JsonArray& values = root.createNestedArray(F("values"));
    
    if (!getAllValuesJSONArray(values)) {
        JsonObject& errorObj = values.createNestedObject();
        errorObj[F("ValueError")] = F("Did not read any values!");
    }
    
    response->setLength();
    request->send(response);
}

void WebHandlers::handleFieldValue(AsyncWebServerRequest* request) {
    if (!request->hasParam(F("name"))) {
        request->send(400, F("text/plain"), F("Missing 'name' parameter"));
        return;
    }
    
    String name = request->getParam(F("name"))->value();
    String value = getFieldValue(name.c_str());
    
    request->send(200, F("text/plain"), value);
}

void WebHandlers::handlePing(AsyncWebServerRequest* request) {
    request->send(200, F("text/plain"), F("Pong"));
}

void WebHandlers::handleNotFound(AsyncWebServerRequest* request) {
    request->redirect("/");
}

void WebHandlers::sendJsonError(AsyncWebServerRequest* request, const String& error) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    root[F("error")] = error;
    response->setLength();
    request->send(response);
}

uint32_t WebHandlers::parseColorParameter(const String& value) {
    return constrain(strtoul(value.c_str(), NULL, 16), 0, 0xffffff);
}