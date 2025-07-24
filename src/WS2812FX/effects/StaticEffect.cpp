#include "StaticEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t StaticEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment data for palette distribution calculation
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate hue delta based on palette distribution setting
    uint8_t paletteDistribution = seg->paletteDistribution;
    uint8_t deltaHue = max(1, (256 * 100 / (runtime->length * paletteDistribution)));
    
    // Use helper to fill palette with full brightness
    EffectHelper::fillPaletteWithBrightness(strip, 255, deltaHue);
    
    // Static effect doesn't need frequent updates, return minimum delay
    return strip->getStripMinDelay();
}

const __FlashStringHelper* StaticEffect::getName() const {
    return F("Static");
}

uint8_t StaticEffect::getModeId() const {
    return FX_MODE_STATIC;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_STATIC, StaticEffect)