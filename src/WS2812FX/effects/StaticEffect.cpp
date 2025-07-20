#include "StaticEffect.h"
#include "../WS2812FX_FastLed.h"

bool StaticEffect::init(WS2812FX* strip) {
    // Static effect needs no special initialization
    // Access the segment runtime through the public getter
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    return true;
}

uint16_t StaticEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Fill the segment with colors from the current palette
    // Distribute the palette over the display length based on paletteDistribution setting
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    uint8_t paletteDistribution = seg->paletteDistribution;
    uint8_t deltaHue = max(1, (256 * 100 / (runtime->length * paletteDistribution)));
    
    fill_palette(&strip->leds[runtime->start], runtime->length, 
                 runtime->baseHue, deltaHue, *currentPalette, 
                 255, seg->blendType);
    
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