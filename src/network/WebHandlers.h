/*************************************************************
 * WebHandlers.h - HTTP request handlers for the LED controller
 * 
 * This module provides all the HTTP endpoint handlers that were
 * previously embedded in the main application file.
 *************************************************************/

#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// Forward declarations
class NetworkManager;
class ConfigManager;

/**
 * Manages all HTTP request handlers for the LED controller web interface
 */
class WebHandlers {
public:
    WebHandlers(NetworkManager* network, ConfigManager* config);
    ~WebHandlers();

    // Setup all request handlers on the server
    void setupHandlers(AsyncWebServer* server);

    // Individual handler methods
    static void handleStatus(AsyncWebServerRequest* request);
    static void handleSet(AsyncWebServerRequest* request);
    static void handleGetModes(AsyncWebServerRequest* request);
    static void handleGetPalettes(AsyncWebServerRequest* request);
    static void handleReset(AsyncWebServerRequest* request);
    static void handleAllConfig(AsyncWebServerRequest* request);
    static void handleAllValues(AsyncWebServerRequest* request);
    static void handleFieldValue(AsyncWebServerRequest* request);
    static void handlePing(AsyncWebServerRequest* request);
    static void handleNotFound(AsyncWebServerRequest* request);

private:
    NetworkManager* networkManager;
    ConfigManager* configManager;

    // Static instance for callback access
    static WebHandlers* instance;

    // Helper methods
    static void sendJsonError(AsyncWebServerRequest* request, const String& error);
    static bool validateParameters(AsyncWebServerRequest* request);
    static uint32_t parseColorParameter(const String& value);
    
    // Non-copyable
    WebHandlers(const WebHandlers&) = delete;
    WebHandlers& operator=(const WebHandlers&) = delete;
};

#endif // WEB_HANDLERS_H