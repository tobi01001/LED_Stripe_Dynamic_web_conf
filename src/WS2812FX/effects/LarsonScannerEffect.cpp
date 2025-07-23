#include "LarsonScannerEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

// Include FastLED lib8tion for beat88 function
#include "lib8tion.h"

bool LarsonScannerEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false;
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t LarsonScannerEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    // Ensure effect is properly initialized
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return minimum delay if init failed
        }
    }
    // Access segment and runtime data through the strip's public interface
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate bar width proportional to strip length using helper
    const uint16_t width = EffectHelper::calculateProportionalWidth(strip, 15, 1);
    
    // Apply fade out effect using helper
    EffectHelper::applyFadeEffect(strip, EffectHelper::MEDIUM_FADE);
    
    // Generate smooth bouncing motion using helper with increased speed
    uint16_t triangularPosition = EffectHelper::calculateTrianglePosition(strip, timebase, EffectHelper::FAST_SPEED);
    
    // Map the triangular wave to the strip position using helper
    uint16_t pos = EffectHelper::mapPositionToStrip(strip, triangularPosition);
    
    // Calculate color index using helper
    uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, pos, runtime->baseHue);
    
    // Draw the fractional bar using helper
    EffectHelper::drawBar(strip, pos, width, colorIndex);
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* LarsonScannerEffect::getName() const {
    return F("Larson Scanner");
}

uint8_t LarsonScannerEffect::getModeId() const {
    return FX_MODE_LARSON_SCANNER;
}

// Register this effect with the factory system
// This allows the effect to be created automatically when the mode is selected
REGISTER_EFFECT(FX_MODE_LARSON_SCANNER, LarsonScannerEffect)