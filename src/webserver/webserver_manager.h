#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

// Forward declarations
extern AsyncWebServer server;
extern AsyncWebSocket *webSocketsServer;

struct pingPong {
  uint32_t iD = 0;
  uint8_t pong = 0;
  uint8_t ping = 0;
};

extern pingPong my_pingPongs[];

/**
 * @brief Initialize web server with all routes and handlers
 */
void setupWebServer(void);

/**
 * @brief Handle HTTP /set requests for LED control
 * @param request The incoming HTTP request
 */
void handleSet(AsyncWebServerRequest *request);

/**
 * @brief Handle HTTP requests for unknown/not found resources
 * @param request The incoming HTTP request
 */
void handleNotFound(AsyncWebServerRequest *request);

/**
 * @brief Handle HTTP /getmodes requests to return available effects
 * @param request The incoming HTTP request
 */
void handleGetModes(AsyncWebServerRequest *request);

/**
 * @brief Handle HTTP /getpals requests to return available color palettes
 * @param request The incoming HTTP request
 */
void handleGetPals(AsyncWebServerRequest *request);

/**
 * @brief Handle HTTP /status requests to return current LED status
 * @param request The incoming HTTP request
 */
void handleStatus(AsyncWebServerRequest *request);

/**
 * @brief Handle HTTP /reset requests for system reset operations
 * @param request The incoming HTTP request
 */
void handleResetRequest(AsyncWebServerRequest *request);

/**
 * @brief Handle WebSocket events (connect, disconnect, messages)
 * @param server WebSocket server instance
 * @param client WebSocket client instance
 * @param type Event type
 * @param arg Event arguments
 * @param data Event data
 * @param len Data length
 */
void webSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

/**
 * @brief Add a WebSocket client to the ping-pong list
 * @param iD Client ID
 * @return Client index in the list
 */
uint8_t addClient(uint32_t iD);

/**
 * @brief Get client index by ID
 * @param iD Client ID
 * @return Client index in the list
 */
uint8_t getClient(uint32_t iD);

/**
 * @brief Remove a WebSocket client from the ping-pong list
 * @param iD Client ID
 */
void removeClient(uint32_t iD);

/**
 * @brief Handle periodic WebSocket client maintenance
 */
void handleWebSocketMaintenance(void);

#endif // WEBSERVER_MANAGER_H