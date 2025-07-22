#include "BreathEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool BreathEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from EffectHelper
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t BreathEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate breathing brightness using beatsin88 for smooth sine wave modulation
    // Double the speed (beat88 * 2) for more responsive breathing effect
    // Range from 10 (minimum visible) to 255 (maximum brightness)
    uint8_t breathingBrightness = beatsin88(seg->beat88 * 2, 10, 255, timebase);
    
    // Fill the entire strip segment with palette colors
    // Using fill_palette for efficient rendering with the breathing brightness
    // Parameters:
    // - LED array starting at segment start
    // - Segment length 
    // - Starting hue (baseHue for color cycling)
    // - Hue increment of 5 for gentle color distribution
    // - Current palette for colors
    // - Breathing brightness for pulsing effect
    // - Blend type for smooth color transitions
    fill_palette(&strip->leds[runtime->start], 
                 runtime->length, 
                 0 + runtime->baseHue,  // Starting hue with base offset
                 5,                     // Hue increment for color distribution
                 *strip->getCurrentPalette(), 
                 breathingBrightness,   // Modulated brightness for breathing effect
                 seg->blendType);
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* BreathEffect::getName() const {
    return F("Breath");
}

uint8_t BreathEffect::getModeId() const {
    return FX_MODE_BREATH;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_BREATH, BreathEffect)