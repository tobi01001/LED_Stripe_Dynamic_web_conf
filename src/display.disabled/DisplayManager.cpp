/*************************************************************
 * DisplayManager.cpp - OLED display and knob control implementation
 *************************************************************/

#include "DisplayManager.h"
#include "../Constants.h"

#ifdef HAS_KNOB_CONTROL

DisplayManager::DisplayManager()
    : encoder(HardwarePins::KNOB_ENCODER_A, HardwarePins::KNOB_ENCODER_B)
    , display(HardwarePins::DISPLAY_I2C_ADDR, HardwarePins::DISPLAY_SDA, HardwarePins::DISPLAY_SCL)
    , currentState(DISPLAY_OFF)
    , currentField(0)
    , lastControlOperation(0)
    , lastButtonPress(0)
    , displayWasOff(false)
    , inSubmenu(false)
    , newFieldSelected(false)
    , setNewValue(false)
    , encoderMaxVal(255)
    , encoderMinVal(0)
    , encoderCurVal(0)
    , encoderSteps(1)
    , oldEncoderVal(0)
    , timeoutBar(0)
{
}

DisplayManager::~DisplayManager() {
    shutdown();
}

bool DisplayManager::initialize() {
    setupDisplay();
    currentState = DISPLAY_SHOW_INFO;
    lastControlOperation = millis();
    
    // Show boot information
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, F("Booting... Please wait"));
    display.display();
    
    return true;
}

void DisplayManager::handleLoop() {
    uint32_t now = millis();
    
    handleButtonInput();
    handleEncoderInput();
    updateDisplay();
    
    // Handle display timeout
    if (now > lastControlOperation + DISPLAY_TIMEOUT) {
        currentState = DISPLAY_OFF;
        displayWasOff = true;
    } else if (now > lastControlOperation + OPERATION_TIMEOUT) {
        timeoutBar = map((uint16_t)(now - lastControlOperation), 
                        (uint16_t)0, (uint16_t)DISPLAY_TIMEOUT, 
                        (uint16_t)127, (uint16_t)0);
        currentField = getNextField(0, true);
        inSubmenu = true;
        currentState = DISPLAY_SHOW_INFO;
    } else {
        timeoutBar = map((uint16_t)(now - lastControlOperation), 
                        (uint16_t)0, (uint16_t)DISPLAY_TIMEOUT, 
                        (uint16_t)127, (uint16_t)0);
        if (!inSubmenu) {
            currentState = DISPLAY_SHOW_MENU;
        } else {
            // Set state based on field type
            currentState = DISPLAY_SHOW_INFO; // Simplified for now
        }
    }
}

void DisplayManager::shutdown() {
    display.displayOff();
}

void DisplayManager::setupDisplay() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void DisplayManager::handleButtonInput() {
    uint32_t now = millis();
    
    if (digitalRead(HardwarePins::KNOB_BUTTON) == LOW && 
        now > lastButtonPress + BUTTON_DEBOUNCE) {
        
        lastButtonPress = now;
        
        if (displayWasOff) {
            displayWasOff = false;
            lastControlOperation = now - OPERATION_TIMEOUT - 100;
            currentState = DISPLAY_SHOW_INFO;
            return;
        }
        
        lastControlOperation = now;
        inSubmenu = !inSubmenu;
        
        if (!inSubmenu) {
            encoderMaxVal = getFieldCount() - 1;
            encoderMinVal = 0;
            encoderSteps = 1;
            encoderCurVal = currentField;
            oldEncoderVal = currentField;
        } else {
            oldEncoderVal = setEncoderValues(currentField);
        }
    }
}

void DisplayManager::handleEncoderInput() {
    // Simplified encoder handling
    int8_t direction = encoder.direction();
    if (direction != 0) {
        uint32_t now = millis();
        
        if (displayWasOff) {
            displayWasOff = false;
            lastControlOperation = now - OPERATION_TIMEOUT - 100;
            currentState = DISPLAY_SHOW_INFO;
            return;
        }
        
        lastControlOperation = now;
        
        // Handle encoder rotation
        if (direction < 0 && (encoderCurVal <= (encoderMinVal + encoderSteps))) {
            encoderCurVal = encoderMinVal;
        } else if (direction > 0 && encoderCurVal >= (encoderMaxVal - encoderSteps)) {
            encoderCurVal = encoderMaxVal;
        } else {
            encoderCurVal = encoderCurVal + direction * encoderSteps;
        }
        
        if (oldEncoderVal != encoderCurVal) {
            if (!inSubmenu) {
                // Navigate fields
                if (encoderCurVal > currentField) {
                    currentField = getNextField(currentField, true);
                } else {
                    currentField = getNextField(currentField, false);
                }
                newFieldSelected = true;
            } else {
                // Set field value
                if (newFieldSelected) {
                    oldEncoderVal = setEncoderValues(currentField);
                    newFieldSelected = false;
                }
                setNewValue = true;
            }
            oldEncoderVal = encoderCurVal;
        }
        
        encoderCurVal = oldEncoderVal;
        
        // Apply field value changes
        if (setNewValue) {
            setNewValue = false;
            const Field* fields = getFields();
            if (fields && currentField < getFieldCount() && fields[currentField].setValue) {
                fields[currentField].setValue(encoderCurVal);
            }
        }
    }
}

void DisplayManager::updateDisplay() {
    static uint32_t lastUpdate = 0;
    static bool blinkState = false;
    
    uint32_t now = millis();
    
    // Update at display FPS
    if (now - lastUpdate < (1000 / DISPLAY_FPS)) {
        return;
    }
    lastUpdate = now;
    
    // Handle cursor blink
    if (now % CURSOR_BLINK_RATE_MS < (CURSOR_BLINK_RATE_MS / 2)) {
        blinkState = !blinkState;
    }
    
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    
    if (currentState == DISPLAY_OFF) {
        display.displayOff();
        return;
    }
    
    display.displayOn();
    
    switch (currentState) {
        case DISPLAY_SHOW_INFO:
            drawInfoScreen();
            break;
        case DISPLAY_SHOW_MENU:
            drawMenuScreen();
            break;
        case DISPLAY_SHOW_BOOL_MENU:
            drawBooleanMenu();
            break;
        case DISPLAY_SHOW_NUMBER_MENU:
            drawNumberMenu();
            break;
        case DISPLAY_SHOW_SELECT_MENU:
            drawSelectMenu();
            break;
        default:
            display.drawString(0, 0, F("Unknown state"));
            break;
    }
    
    // Draw WiFi indicator
    drawWiFiIndicator();
    
    // Draw timeout bar
    display.drawHorizontalLine(64 - (timeoutBar / 2), 63, timeoutBar);
    
    display.display();
}

void DisplayManager::drawInfoScreen() {
    display.drawString(0, 0, LED_NAME);
    display.drawString(0, 10, F("Brightness:"));
    display.drawString(0, 20, F("Speed:"));
    display.drawString(0, 30, F("Mode:"));
    display.drawString(0, 40, F("Palette:"));
    display.drawString(0, 50, F("Power:"));
    
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    
    if (strip) {
        display.drawString(127, 10, String(strip->getTargetBrightness()));
        display.drawString(127, 20, String(strip->getBeat88()));
        display.drawString(127, 30, strip->getPower() ? strip->getModeName(strip->getMode()) : F("Off"));
        display.drawString(127, 40, strip->getPalName(strip->getTargetPaletteNumber()));
        display.drawString(127, 50, String(strip->getCurrentPower() / 5));
    }
}

void DisplayManager::drawMenuScreen() {
    display.drawString(0, 0, F("Menu:"));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    
    const Field* fields = getFields();
    if (fields && currentField < getFieldCount()) {
        display.drawString(64, 30, fields[currentField].label);
    }
}

void DisplayManager::drawBooleanMenu() {
    // Simplified boolean menu
    display.drawString(0, 0, F("Boolean Setting"));
}

void DisplayManager::drawNumberMenu() {
    // Simplified number menu
    display.drawString(0, 0, F("Number Setting"));
}

void DisplayManager::drawSelectMenu() {
    // Simplified select menu
    display.drawString(0, 0, F("Select Setting"));
}

void DisplayManager::drawWiFiIndicator() {
    // Draw WiFi signal strength indicator
    if (WiFi.status() == WL_CONNECTED) {
        int32_t rssi = WiFi.RSSI();
        uint8_t bars = 0;
        
        if (rssi >= -55) bars = 5;
        else if (rssi >= -70) bars = 4;
        else if (rssi >= -85) bars = 3;
        else if (rssi >= -100) bars = 2;
        else if (rssi >= -110) bars = 1;
        
        for (uint8_t i = 0; i <= bars; i++) {
            display.drawVerticalLine(115 + 2*i, 10 - i*2, i*2);
        }
    } else {
        // No WiFi - draw X
        display.drawLine(117, 9, 123, 3);
        display.drawLine(123, 9, 117, 3);
    }
}

uint8_t DisplayManager::getNextField(uint8_t currentField, bool up) const {
    const uint8_t fieldCount = getFieldCount();
    const Field* fields = getFields();
    
    if (!fields || fieldCount == 0) {
        return 0;
    }
    
    uint8_t result = currentField;
    uint8_t firstField = 0;
    uint8_t lastField = fieldCount - 1;
    
    // Find first and last valid fields
    for (uint8_t i = 0; i < fieldCount; i++) {
        if (fields[i].type <= SelectFieldType) {
            lastField = i;
        }
        if (fields[fieldCount - 1 - i].type <= SelectFieldType) {
            firstField = fieldCount - 1 - i;
        }
    }
    
    uint8_t sanity = 0;
    while (result == currentField || (fields[result].type > SelectFieldType)) {
        if (sanity++ > fieldCount) break;
        
        if (up) {
            result++;
            if (result >= lastField) {
                result = lastField;
            }
        } else {
            if (result > firstField) {
                result--;
            } else {
                result = firstField;
            }
        }
    }
    
    return result;
}

uint16_t DisplayManager::setEncoderValues(uint8_t field) {
    const Field* fields = getFields();
    if (!fields || field >= getFieldCount()) {
        return 0;
    }
    
    uint16_t currentValue = 0;
    
    switch (fields[field].type) {
        case NumberFieldType:
            currentValue = (uint16_t)fields[field].getValue();
            encoderSteps = (fields[field].max - fields[field].min) / 100;
            if (encoderSteps == 0) encoderSteps = 1;
            encoderMaxVal = fields[field].max;
            encoderMinVal = fields[field].min;
            encoderCurVal = currentValue;
            break;
            
        case BooleanFieldType:
            currentValue = (uint16_t)fields[field].getValue();
            encoderSteps = 1;
            encoderMaxVal = 1;
            encoderMinVal = 0;
            encoderCurVal = currentValue;
            break;
            
        case SelectFieldType:
            currentValue = (uint16_t)fields[field].getValue();
            encoderSteps = 1;
            encoderMaxVal = fields[field].max;
            encoderMinVal = fields[field].min;
            encoderCurVal = currentValue;
            break;
            
        default:
            break;
    }
    
    return currentValue;
}

uint8_t DisplayManager::drawTextLine(uint8_t y, uint8_t fontHeight, const String& text) {
    if (text == "") return y;
    
    uint8_t textLines = 1;
    textLines = (((display.getStringWidth(text) - 2) / DisplayConstants::DISPLAY_WIDTH) + 1) * fontHeight;
    display.drawStringMaxWidth(0, y, DisplayConstants::DISPLAY_WIDTH, text);
    y += textLines;
    return y;
}

#endif // HAS_KNOB_CONTROL