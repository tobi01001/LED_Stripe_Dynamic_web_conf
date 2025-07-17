/*************************************************************
 * NetworkManager.cpp - WiFi and web server management implementation
 * 
 * Extracted from httpledstripe_esp.cpp to improve code organization
 *************************************************************/

#include "NetworkManager.h"
#include "WebHandlers.h"
#include "../config/ConfigManager.h"
#include "../defaults.h"
#include <LittleFS.h>

// Static instance for callback handling
static NetworkManager* g_networkManagerInstance = nullptr;

NetworkManager::NetworkManager() 
    : server(nullptr)
    , webSocketServer(nullptr)
    , webHandlers(nullptr)
    , deviceName(nullptr)
    , otaIsRunning(false)
    , wifiDisabled(false)
    , lastWiFiCheck(0)
{
    g_networkManagerInstance = this;
    memset(pingPongs, 0, sizeof(pingPongs));
}

NetworkManager::~NetworkManager() {
    shutdown();
    g_networkManagerInstance = nullptr;
}

bool NetworkManager::initialize(const char* name, uint16_t timeout) {
    deviceName = name;
    
    if (wifiDisabled) {
        return true; // Skip network initialization if disabled
    }
    
    bool success = true;
    success &= setupWiFi(timeout);
    success &= setupWebServer();
    setupWebHandlers();
    initOTA();
    
    return success;
}

bool NetworkManager::setupWiFi(uint16_t timeout) {
    if (wifiDisabled) {
        return true;
    }

    // Set hostname and WiFi mode
    WiFi.hostname(deviceName);
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.persistent(true);

    DNSServer dns;
    AsyncWiFiManager wifiManager(server, &dns);
    
    wifiManager.setConfigPortalTimeout(timeout);
    wifiManager.setBreakAfterConfig(true);

    if (!wifiManager.autoConnect(deviceName)) {
        #ifdef HAS_KNOB_CONTROL
        wifiStats.isConnected = false;
        return false;
        #else
        // Without knob control, WiFi is mandatory
        // System should restart in original code
        return false;
        #endif
    }

    // Reset disconnect counter on successful connection
    wifiStats.disconnectCounter = 0;
    wifiStats.errorCounter = 0;
    
    if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(true);
    wifiStats.gatewayIP = WiFi.gatewayIP();
    wifiStats.isConnected = true;
    
    return true;
}

bool NetworkManager::setupWebServer() {
    if (!server) {
        server = new AsyncWebServer(80);
    }

    setupRequestHandlers();

    // Setup CORS headers
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
    DefaultHeaders::Instance().addHeader(F("Access-Control-Max-Age"), "1");

    // Setup WebSocket if not disabled
    #ifndef DO_NOT_USE_WEBSOCKET
    if (!webSocketServer) {
        webSocketServer = new AsyncWebSocket("/ws");
    }
    #endif

    if (webSocketServer) {
        webSocketServer->enable(true);
        webSocketServer->onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                                   AwsEventType type, void* arg, uint8_t* data, size_t len) {
            if (g_networkManagerInstance) {
                g_networkManagerInstance->handleWebSocketEvent(server, client, type, arg, data, len);
            }
        });
        server->addHandler(webSocketServer);
    }

    // Serve static files from LittleFS
    server->serveStatic("/", LittleFS, "/").setCacheControl("max-age=60");
    
    server->begin();

    // Setup mDNS
    if (MDNS.begin(deviceName)) {
        MDNS.addService("http", "tcp", 80);
    }

    return true;
}

void NetworkManager::setupRequestHandlers() {
    // Basic endpoints only - main handlers are in WebHandlers
    server->on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, F("text/plain"), F("Pong"));
    });

    server->onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
}

void NetworkManager::setupWebHandlers() {
    // This should be called after ConfigManager is available
    // For now, we'll set it up later when needed
}

void NetworkManager::initializeWebHandlers(ConfigManager* configManager) {
    if (webHandlers) {
        delete webHandlers;
    }
    
    webHandlers = new WebHandlers(this, configManager);
    if (webHandlers && server) {
        webHandlers->setupHandlers(server);
    }
}

void NetworkManager::initOTA() {
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.setHostname(deviceName);

    ArduinoOTA.onStart([this]() {
        otaIsRunning = true;
        if (server) {
            server->end();
        }
        if (webSocketServer) {
            webSocketServer->textAll("OTA started!");
            webSocketServer->closeAll();
            webSocketServer->enable(false);
        }
        LittleFS.end();
    });

    ArduinoOTA.onEnd([this]() {
        otaIsRunning = false;
        LittleFS.begin();
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        LittleFS.begin();
        // TODO: Handle OTA errors appropriately
    });

    ArduinoOTA.begin();
}

void NetworkManager::handleLoop() {
    if (otaIsRunning) {
        return;
    }

    checkWiFiConnection();
    
    if (wifiStats.isConnected && !wifiDisabled) {
        ArduinoOTA.handle();
        MDNS.update();
        cleanupClients();
    }
}

void NetworkManager::checkWiFiConnection() {
    uint32_t now = millis();
    
    if (wifiDisabled || now < lastWiFiCheck + WIFI_TIMEOUT) {
        return;
    }

    bool connected = (WiFi.status() == WL_CONNECTED);
    
    if (!connected) {
        if (server) {
            server->end();
        }
        if (webSocketServer) {
            webSocketServer->enable(false);
        }
        
        wifiStats.errorCounter += 2;
        wifiStats.disconnectCounter += 50;
        wifiStats.isConnected = false;
    } else {
        if (server) {
            server->begin();
        }
        if (webSocketServer) {
            webSocketServer->enable(true);
        }
        
        if (wifiStats.errorCounter > 0) {
            wifiStats.errorCounter--;
        }
        
        static uint8_t resetCnt = 0;
        resetCnt = (resetCnt + 1) % 6;
        if (wifiStats.disconnectCounter > 0 && !resetCnt) {
            wifiStats.disconnectCounter--;
        }
        
        wifiStats.isConnected = true;
    }

    // Handle extended disconnection
    if (wifiStats.errorCounter > 20) {
        WiFi.mode(WIFI_OFF);
        delay(2000);
        setupWiFi();
    }

    lastWiFiCheck = now;
}

void NetworkManager::cleanupClients() {
    if (!webSocketServer) {
        return;
    }

    static uint32_t lastCleanup = 0;
    uint32_t now = millis();
    
    if (now - lastCleanup > 2000) { // Every 2 seconds
        for (const auto& client : webSocketServer->getClients()) {
            uint8_t i = getClient(client->id());
            if (i < 10) { // DEFAULT_MAX_WS_CLIENTS
                client->text("{\"Client\": " + String(client->id()) + "}");
                pingPongs[i].ping = random(256);
                client->ping(&pingPongs[i].ping, sizeof(uint8_t));
            }
        }
        webSocketServer->cleanupClients();
        lastCleanup = now;
    }
}

void NetworkManager::broadcastInt(const __FlashStringHelper* name, uint16_t value) {
    if (!webSocketServer || wifiDisabled || !wifiStats.isConnected) {
        return;
    }

    DynamicJsonBuffer jsonBuffer;
    JsonObject& obj = jsonBuffer.createObject();
    obj[F("name")] = name;
    obj[F("value")] = value;

    size_t len = obj.measureLength();
    AsyncWebSocketMessageBuffer* buffer = webSocketServer->makeBuffer(len);
    if (buffer) {
        obj.printTo((char*)buffer->get(), len + 1);
        webSocketServer->textAll(buffer);
    }
}

void NetworkManager::broadcastColor(const __FlashStringHelper* name, uint32_t color) {
    if (!webSocketServer || wifiDisabled || !wifiStats.isConnected) {
        return;
    }

    DynamicJsonBuffer jsonBuffer;
    JsonObject& obj = jsonBuffer.createObject();
    obj[F("name")] = name;
    obj[F("value")] = color & 0xffffff;

    size_t len = obj.measureLength();
    AsyncWebSocketMessageBuffer* buffer = webSocketServer->makeBuffer(len);
    if (buffer) {
        obj.printTo((char*)buffer->get(), len + 1);
        webSocketServer->textAll(buffer);
    }
}

uint8_t NetworkManager::addClient(uint32_t clientId) {
    if (!webSocketServer || wifiDisabled) {
        return 10; // DEFAULT_MAX_WS_CLIENTS
    }

    if (webSocketServer->count() >= 10) {
        return 10;
    }

    // Check if client already exists
    for (uint8_t i = 0; i < 10; i++) {
        if (pingPongs[i].iD == clientId) {
            return i;
        }
    }

    // Find empty slot
    for (uint8_t i = 0; i < 10; i++) {
        if (pingPongs[i].iD == 0) {
            pingPongs[i].iD = clientId;
            pingPongs[i].ping = 0;
            pingPongs[i].pong = 0;
            return i;
        }
    }

    return 10;
}

uint8_t NetworkManager::getClient(uint32_t clientId) const {
    for (uint8_t i = 0; i < 10; i++) {
        if (pingPongs[i].iD == clientId) {
            return i;
        }
    }
    return 10;
}

void NetworkManager::removeClient(uint32_t clientId) {
    for (uint8_t i = 0; i < 10; i++) {
        if (pingPongs[i].iD == clientId) {
            pingPongs[i].iD = 0;
            pingPongs[i].ping = 0;
            pingPongs[i].pong = 0;
            break;
        }
    }
}

void NetworkManager::handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                         AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (wifiDisabled) {
        return;
    }

    switch (type) {
        case WS_EVT_CONNECT:
            client->printf("{\"Client\":%u}", client->id());
            {
                uint8_t i = addClient(client->id());
                if (i < 10) {
                    pingPongs[i].ping = random(256);
                    client->ping(&pingPongs[i].ping, sizeof(uint8_t));
                }
            }
            break;

        case WS_EVT_DISCONNECT:
            removeClient(client->id());
            break;

        case WS_EVT_PONG:
            {
                uint8_t i = getClient(client->id());
                if (i < 10) {
                    pingPongs[i].pong = (len) ? *data : 0;
                }
            }
            break;

        case WS_EVT_ERROR:
            // TODO: Handle WebSocket errors
            break;

        case WS_EVT_DATA:
            // TODO: Handle WebSocket data if needed
            break;
    }
}

void NetworkManager::shutdown() {
    if (webHandlers) {
        delete webHandlers;
        webHandlers = nullptr;
    }

    if (webSocketServer) {
        webSocketServer->enable(false);
        delete webSocketServer;
        webSocketServer = nullptr;
    }

    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }

    WiFi.mode(WIFI_OFF);
    wifiStats.isConnected = false;
}