#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

// Forward declarations
extern AsyncWebServer server;
extern AsyncWebSocket *webSocketsServer;
extern bool OTAisRunning;
extern uint8_t wifi_err_counter;
extern uint16_t wifi_disconnect_counter;
extern IPAddress gateway_ip;

#ifdef HAS_KNOB_CONTROL
extern bool WiFiConnected;
#endif

/**
 * @brief Initialize WiFi connection with WiFi Manager
 * @param timeout Timeout in seconds for captive portal (default 240)
 */
void setupWiFi(uint16_t timeout = 240);

/**
 * @brief Initialize Over-The-Air update functionality
 */
void initOverTheAirUpdate(void);

/**
 * @brief Check WiFi connection status and handle reconnection
 * @param now Current timestamp in milliseconds
 * @return true if WiFi is connected, false otherwise
 */
bool checkWiFiConnection(uint32_t now);

/**
 * @brief Handle OTA update process
 */
void handleOTA(void);

/**
 * @brief Handle MDNS updates
 */
void handleMDNS(void);

#endif // NETWORK_MANAGER_H