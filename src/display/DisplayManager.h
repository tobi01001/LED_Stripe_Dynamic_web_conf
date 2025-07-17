/*************************************************************
 * DisplayManager.h - OLED display and knob control management
 * 
 * Extracted from httpledstripe_esp.cpp to improve code organization
 * This module handles the knob control interface and OLED display.
 *************************************************************/

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

#ifdef HAS_KNOB_CONTROL
#include "Encoder.h"
#include "SSD1306Brzo.h"
#include "../LED_strip/led_strip.h"

/**
 * Manages OLED display and rotary encoder interface
 */
class DisplayManager {
public:
    enum DisplayState {
        DISPLAY_OFF,
        DISPLAY_SHOW_INFO,
        DISPLAY_SHOW_MENU,
        DISPLAY_SHOW_SECTION_MENU,
        DISPLAY_SHOW_BOOL_MENU,
        DISPLAY_SHOW_NUMBER_MENU,
        DISPLAY_SHOW_SELECT_MENU
    };

    DisplayManager();
    ~DisplayManager();

    // Core display management
    bool initialize();
    void handleLoop();
    void shutdown();

    // Display control
    void setDisplayState(DisplayState state) { currentState = state; }
    DisplayState getDisplayState() const { return currentState; }
    
    // Field navigation
    void setCurrentField(uint8_t field) { currentField = field; }
    uint8_t getCurrentField() const { return currentField; }

    // Utility functions
    void forceDisplayUpdate() { lastControlOperation = millis() - displayTimeout - 100; }
    bool isDisplayActive() const { return currentState != DISPLAY_OFF; }

private:
    // Hardware components
    Encoder encoder;
    SSD1306Brzo display;
    
    // State management
    DisplayState currentState;
    uint8_t currentField;
    uint32_t lastControlOperation;
    uint32_t lastButtonPress;
    bool displayWasOff;
    bool inSubmenu;
    bool newFieldSelected;
    bool setNewValue;
    
    // Display configuration
    static const uint32_t DISPLAY_TIMEOUT = 30000;     // 30 seconds
    static const uint32_t OPERATION_TIMEOUT = 10000;   // 10 seconds
    static const uint32_t BUTTON_DEBOUNCE = 200;       // 200ms
    static const uint32_t ROTATION_DEBOUNCE = 50;      // 50ms
    static const uint8_t DISPLAY_FPS = 10;
    static const uint16_t CURSOR_BLINK_RATE = 500;     // 500ms
    
    // Encoder parameters
    uint16_t encoderMaxVal;
    uint16_t encoderMinVal;
    uint16_t encoderCurVal;
    uint16_t encoderSteps;
    uint16_t oldEncoderVal;
    
    // Display helpers
    uint8_t timeoutBar;
    
    // Internal methods
    void setupDisplay();
    void handleEncoderInput();
    void handleButtonInput();
    void updateDisplay();
    void drawInfoScreen();
    void drawMenuScreen();
    void drawBooleanMenu();
    void drawNumberMenu();
    void drawSelectMenu();
    void drawSectionMenu();
    
    // Navigation helpers
    uint8_t getNextField(uint8_t currentField, bool up) const;
    uint16_t setEncoderValues(uint8_t field);
    uint8_t drawTextLine(uint8_t y, uint8_t fontHeight, const String& text);
    
    // WiFi signal indicator
    void drawWiFiIndicator();
    
    // Non-copyable
    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;
};

#else // !HAS_KNOB_CONTROL

// Dummy class when knob control is not available
class DisplayManager {
public:
    bool initialize() { return true; }
    void handleLoop() {}
    void shutdown() {}
    void forceDisplayUpdate() {}
    bool isDisplayActive() const { return false; }
};

#endif // HAS_KNOB_CONTROL

#endif // DISPLAY_MANAGER_H