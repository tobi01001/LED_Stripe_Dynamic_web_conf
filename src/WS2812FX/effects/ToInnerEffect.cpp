#include "ToInnerEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t ToInnerEffect::update(WS2812FX* strip) {
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
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate the center point for the "to inner" effect
    uint16_t centerLength = (runtime->length / 2) + 1;
    uint16_t ledUpTo = centerLength + runtime->start;
    
    // Calculate fade amount based on beat88 speed
    // Use speed-based calculation with threshold to prevent overflow
    uint8_t fadeAmount;
    if (seg->beat88 * SPEED_MULTIPLIER <= FADE_THRESHOLD) {
        fadeAmount = (seg->beat88 * SPEED_MULTIPLIER) >> 6;
    } else {
        fadeAmount = 255;
    }
    
    // Apply background fade with minimum value
    uint8_t actualFade = max(fadeAmount, MIN_FADE);
    EffectHelper::applyFadeEffect(strip, actualFade);
    
    // Calculate beat speed with clamping
    uint16_t beatSpeed;
    if (seg->beat88 < SPEED_THRESHOLD) {
        beatSpeed = seg->beat88 * SPEED_MULTIPLIER;
    } else {
        beatSpeed = 65535; // Maximum value
    }
    
    // Calculate position using beatsin88 for smooth pulsing
    uint16_t pulseLength = beatsin88(beatSpeed, 0, ledUpTo, _timebase);
    
    // Fill the first half of the strip with palette colors
    // Start from segment beginning, fill up to calculated pulse length
    fill_palette(&strip->leds[runtime->start], 
                 pulseLength,
                 runtime->baseHue,          // Starting hue with base offset
                 HUE_INCREMENT,             // Hue increment for color distribution
                 *strip->getCurrentPalette(), 
                 255,                       // Full brightness
                 seg->blendType);
    
    // Mirror the first half to create symmetrical pattern
    // Copy from the beginning to the end in reverse order
    for (int16_t i = runtime->length - 1; i >= (runtime->length - ledUpTo); i--) {
        uint16_t mirrorIndex = (runtime->length - 1) - i + runtime->start;
        
        // Bounds checking to prevent array access violations
        if (mirrorIndex >= runtime->start && 
            mirrorIndex < runtime->stop && 
            i >= runtime->start && 
            i < runtime->stop) {
            strip->leds[i] = strip->leds[mirrorIndex];
        }
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* ToInnerEffect::getName() const {
    return F("Centering");
}

uint8_t ToInnerEffect::getModeId() const {
    return FX_MODE_TO_INNER;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_TO_INNER, ToInnerEffect)