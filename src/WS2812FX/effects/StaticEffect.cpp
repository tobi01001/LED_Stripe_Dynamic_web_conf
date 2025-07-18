#include "StaticEffect.h"
#include "../WS2812FX_FastLed.h"

bool StaticEffect::init(WS2812FX* strip) {
    // Static effect needs no special initialization
    // Access the segment runtime directly through the strip
    strip->_segment_runtime.modeinit = false;
    return true;
}

uint16_t StaticEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip
    auto& seg = strip->_segment;
    auto& runtime = strip->_segment_runtime;
    
    // Fill the segment with colors from the current palette
    // Distribute the palette over the display length based on paletteDistribution setting
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    uint8_t paletteDistribution = seg.paletteDistribution;
    uint8_t deltaHue = max(1, (256 * 100 / (runtime.length * paletteDistribution)));
    
    fill_palette(&strip->leds[runtime.start], runtime.length, 
                 runtime.baseHue, deltaHue, *currentPalette, 
                 255, seg.blendType);
    
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