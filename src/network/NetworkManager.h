/*************************************************************
 * NetworkManager.h - WiFi and web server management
 * 
 * Extracted from httpledstripe_esp.cpp to improve code organization
 * and separation of concerns.
 * 
 * This class encapsulates all network-related functionality:
 * - WiFi connection management
 * - Web server setup and handling
 * - WebSocket management
 * - OTA updates
 *************************************************************/

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

// Forward declarations
class LEDController;

/**
 * Manages all network-related functionality for the LED controller
 */
class NetworkManager {
public:
    struct PingPong {
        uint32_t iD = 0;
        uint8_t pong = 0;
        uint8_t ping = 0;
    };

    struct WiFiStats {
        uint8_t errorCounter = 0;
        uint16_t disconnectCounter = 0;
        IPAddress gatewayIP;
        bool isConnected = false;
    };

    NetworkManager();
    ~NetworkManager();

    // Core network management
    bool initialize(const char* deviceName, uint16_t timeout = 240);
    void handleLoop();
    void shutdown();

    // WiFi management
    bool setupWiFi(uint16_t timeout = 240);
    bool isWiFiConnected() const { return wifiStats.isConnected; }
    const WiFiStats& getWiFiStats() const { return wifiStats; }

    // Web server management
    bool setupWebServer();
    void broadcastToClients(const String& message);
    void broadcastInt(const __FlashStringHelper* name, uint16_t value);
    void broadcastColor(const __FlashStringHelper* name, uint32_t color);

    // OTA management
    void initOTA();
    bool isOTARunning() const { return otaIsRunning; }

    // WebSocket client management
    uint8_t addClient(uint32_t clientId);
    uint8_t getClient(uint32_t clientId) const;
    void removeClient(uint32_t clientId);

    // Configuration
    void setWiFiDisabled(bool disabled) { wifiDisabled = disabled; }
    bool isWiFiDisabled() const { return wifiDisabled; }

private:
    // Internal state
    AsyncWebServer* server;
    AsyncWebSocket* webSocketServer;
    WiFiStats wifiStats;
    PingPong pingPongs[10]; // DEFAULT_MAX_WS_CLIENTS
    
    const char* deviceName;
    bool otaIsRunning;
    bool wifiDisabled;
    uint32_t lastWiFiCheck;

    // Internal methods
    void setupRequestHandlers();
    void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                             AwsEventType type, void* arg, uint8_t* data, size_t len);
    void checkWiFiConnection();
    void cleanupClients();
    
    // Static callback wrappers
    static void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                               AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    // Non-copyable
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
};

#endif // NETWORK_MANAGER_H