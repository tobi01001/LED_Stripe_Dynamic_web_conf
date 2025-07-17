/*************************************************************
 * httpledstripe_esp_refactored.cpp - Refactored main application
 * 
 * This is a cleaner, more maintainable version of the original
 * httpledstripe_esp.cpp with improved object-oriented design,
 * better separation of concerns, and reduced complexity.
 *************************************************************/

#ifndef DEBUG
# define FASTLED_INTERNAL
#endif

#include <LittleFS.h>
#include <Arduino.h>

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

// Application modules
#include "Constants.h"
#include "defaults.h"
#include "LED_strip/led_strip.h"
#include "network/NetworkManager.h"
#include "config/ConfigManager.h"
#include "display/DisplayManager.h"
#include "utils/PerformanceMonitor.h"

extern "C" {
  #include "user_interface.h"
}

// Global instances of our managers
NetworkManager* networkManager = nullptr;
ConfigManager* configManager = nullptr;
DisplayManager* displayManager = nullptr;

// LED strip data
CRGB pLeds[LED_COUNT_TOT + 1]; // Extra buffer for safety
CRGB eLeds[LED_COUNT];

// Runtime tracking
struct RuntimeInfo {
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
    uint16_t days = 0;
} runtimeInfo;

// System state
String currentResetReason;
String lastResetReason;
uint8_t statusCounter = 0;

// Build version information
#ifdef DEBUG
  #ifdef HAS_KNOB_CONTROL
    const char* buildVersion PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH "_KNOB";
  #else
    const char* buildVersion PROGMEM = BUILD_VERSION "_DBG_" PIO_SRC_BRANCH;
  #endif
#else
  #ifdef HAS_KNOB_CONTROL
    const char* buildVersion PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH "_KNOB";
  #else
    const char* buildVersion PROGMEM = BUILD_VERSION "_" PIO_SRC_BRANCH;
  #endif
#endif

const char* gitRevision PROGMEM = BUILD_GITREV;

// Function declarations
void initializeSystem();
void initializeLEDs();
void handleSystemLoop();
void updateRuntime();
void handleResetRequests();
void addSystemEntropy();
String getResetReasonString(uint8_t reason);
void showInitStatus(uint32_t color);

/**
 * Arduino setup function - Initialize all system components
 */
void setup() {
    // Sanity delay for system stabilization
    delay(SystemConstants::INIT_DELAY_MS);

    #ifdef DEBUG
    Serial.begin(115200);
    Serial.println(F("LED Controller starting..."));
    #endif

    // Initialize entropy for random number generation
    pinMode(A0, INPUT);
    addSystemEntropy();

    // Initialize basic system state
    currentResetReason = ESP.getResetInfo();
    
    // Initialize file system
    if (!LittleFS.begin()) {
        #ifdef DEBUG
        Serial.println(F("Failed to initialize LittleFS"));
        #endif
    }

    // Read last reset reason from file system
    File resetFile = LittleFS.open(F("/lastReset.txt"), "r");
    if (resetFile) {
        lastResetReason = resetFile.readStringUntil('\r');
        resetFile.close();
    } else {
        lastResetReason = F("First boot or FS error");
    }

    // Write current reset reason
    resetFile = LittleFS.open(F("/lastReset.txt"), "w");
    if (resetFile) {
        resetFile.println(currentResetReason + " " + String(random(65536)));
        resetFile.close();
    }

    // Initialize system components
    initializeSystem();

    #ifdef DEBUG
    Serial.println(F("System initialization complete"));
    #endif
}

/**
 * Arduino main loop
 */
void loop() {
    PERF_BEGIN_LOOP();
    
    uint32_t now = millis();

    // Handle manager loops
    if (networkManager && !networkManager->isOTARunning()) {
        PERF_START_TIMING("Network");
        networkManager->handleLoop();
        PERF_END_TIMING("Network");
    }

    if (displayManager) {
        PERF_START_TIMING("Display");
        displayManager->handleLoop();
        PERF_END_TIMING("Display");
    }

    // Handle LED strip updates
    if (strip) {
        PERF_START_TIMING("LEDStrip");
        strip->service();
        PERF_END_TIMING("LEDStrip");
    }

    // Periodic system tasks
    static uint32_t lastSystemCheck = 0;
    if (now - lastSystemCheck >= SystemConstants::SYSTEM_CHECK_INTERVAL_MS) {
        PERF_START_TIMING("SystemTasks");
        
        if (configManager) {
            configManager->periodicSave();
        }
        handleResetRequests();
        PERF_UPDATE_MEMORY();
        
        PERF_END_TIMING("SystemTasks");
        lastSystemCheck = now;
    }

    // Update runtime counter
    static uint32_t lastRuntimeUpdate = 0;
    if (now - lastRuntimeUpdate >= SystemConstants::RUNTIME_UPDATE_INTERVAL_MS) {
        updateRuntime();
        lastRuntimeUpdate = now;
    }
    
    PERF_END_LOOP();
}

/**
 * Initialize all system components
 */
void initializeSystem() {
    // Handle critical reset conditions first
    uint8_t resetReason = ESP.getResetInfoPtr()->reason;
    if (resetReason == REASON_WDT_RST || 
        resetReason == REASON_EXCEPTION_RST || 
        resetReason == REASON_SOFT_WDT_RST) {
        
        #ifdef DEBUG
        Serial.println(F("Critical reset detected, clearing config"));
        #endif
        
        // Clear configuration and restart
        File resetFile = LittleFS.open(F("/lastReset.txt"), "w");
        if (resetFile) {
            resetFile.println(currentResetReason + " - Auto Recovery");
            resetFile.close();
        }
        
        delay(2000);
        ESP.restart();
    }

    // Initialize LEDs first
    initializeLEDs();

    // Create and initialize configuration manager
    configManager = new ConfigManager();
    if (configManager) {
        configManager->initialize(strip->getSegmentSize());
        
        // Try to load saved configuration
        if (!configManager->loadConfiguration()) {
            #ifdef DEBUG
            Serial.println(F("Using default configuration"));
            #endif
        }
    }

    // Initialize display manager (if knob control is enabled)
    displayManager = new DisplayManager();
    if (displayManager) {
        if (!displayManager->initialize()) {
            #ifdef DEBUG
            Serial.println(F("Display initialization failed"));
            #endif
        }
    }

    // Initialize network manager (unless WiFi is disabled)
    bool wifiDisabled = false;
    #ifdef HAS_KNOB_CONTROL
    if (strip) {
        wifiDisabled = strip->getWiFiDisabled();
    }
    #endif

    if (!wifiDisabled) {
        networkManager = new NetworkManager();
        if (networkManager) {
            if (!networkManager->initialize(LED_NAME)) {
                #ifdef DEBUG
                Serial.println(F("Network initialization failed"));
                #endif
                
                #ifndef HAS_KNOB_CONTROL
                // Without knob control, network is required
                // In original code, this would restart the system
                delay(3000);
                ESP.restart();
                #endif
            } else {
                // Initialize web handlers now that config manager is ready
                networkManager->initializeWebHandlers(configManager);
            }
        }
    }

    showInitStatus(ColorConstants::COLOR_GREEN);
    delay(SystemConstants::INIT_DELAY_MS);
    showInitStatus(ColorConstants::COLOR_BLACK);
}

/**
 * Initialize LED strip and related components
 */
void initializeLEDs() {
    showInitStatus(ColorConstants::COLOR_BLUE);
    
    // Initialize FastLED
    FastLED.addLeds<WS2812, LED_PIN, GRB>(pLeds, LED_COUNT_TOT);
    FastLED.setBrightness(255);

    // Initialize LED strip controller
    stripe_setup(pLeds, eLeds);

    showInitStatus(ColorConstants::COLOR_YELLOW);
}

/**
 * Update system runtime counters
 */
void updateRuntime() {
    if (++runtimeInfo.seconds >= 60) {
        runtimeInfo.seconds = 0;
        if (++runtimeInfo.minutes >= 60) {
            runtimeInfo.minutes = 0;
            if (++runtimeInfo.hours >= 24) {
                runtimeInfo.hours = 0;
                runtimeInfo.days++;
            }
        }
    }
}

/**
 * Handle pending reset requests
 */
void handleResetRequests() {
    if (!configManager) {
        return;
    }

    ConfigManager::ResetType resetType = configManager->getPendingReset();
    
    switch (resetType) {
        case ConfigManager::SAVE_AND_RESTART:
            configManager->saveConfiguration();
            delay(100);
            ESP.restart();
            break;

        case ConfigManager::RESET_TO_DEFAULTS:
            if (strip) {
                strip->resetDefaults();
            }
            configManager->deleteConfigFile();
            configManager->saveConfiguration();
            configManager->writeLastResetReason(SystemConstants::RESET_REASON_DEFAULT_RESET);
            delay(100);
            ESP.restart();
            break;

        case ConfigManager::FACTORY_RESET:
            // Visual indication of factory reset
            if (strip) {
                for (uint16_t i = 0; i < strip->getStripLength(); i++) {
                    strip->leds[i] = ColorConstants::COLOR_RED;
                    strip->show();
                    delay(5);
                }
            }
            
            configManager->factoryReset();
            
            // Reset WiFi settings
            if (networkManager) {
                delete networkManager;
                networkManager = nullptr;
            }
            
            WiFi.persistent(false);
            delay(100);
            ESP.restart();
            break;

        case ConfigManager::NO_RESET:
        default:
            // No action needed
            break;
    }
    
    configManager->clearPendingReset();
}

/**
 * Add entropy to random number generator
 */
void addSystemEntropy() {
    random16_add_entropy(ESP.getChipId());
    random16_add_entropy(esp_get_cycle_count());
    random16_add_entropy(ESP.getFreeHeap());
    random16_add_entropy(analogRead(A0));
}

/**
 * Show initialization status on LEDs (for debugging without knob control)
 */
void showInitStatus(uint32_t color) {
    #ifdef DEBUG
    if (strip && strip->leds) {
        // Dim the color for power safety during debug
        color &= ColorConstants::DEBUG_COLOR_MASK;
        
        for (uint8_t i = 0; i < LEDConstants::NUM_INFO_LEDS; i++) {
            strip->leds[i] = CRGB(color);
        }
        strip->show();
    }
    #endif
}