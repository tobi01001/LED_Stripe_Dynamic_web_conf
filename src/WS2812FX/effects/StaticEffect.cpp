#include "StaticEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool StaticEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false; // Local variable since StaticEffect has no persistent state
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t StaticEffect::update(WS2812FX* strip) {
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