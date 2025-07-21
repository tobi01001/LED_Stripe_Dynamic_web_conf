#include "TheaterChaseRainbowEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool TheaterChaseRainbowEffect::init(WS2812FX* strip) {
    // Validate strip pointer and use standardized initialization
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Initialize effect state variables
    _colorCounter = 0;
    _timebase = millis();  // Record current time for pattern timing
    
    // Mark initialization as complete
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t TheaterChaseRainbowEffect::update(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Increment color counter for rainbow cycling, mask to 8 bits to prevent overflow
    _colorCounter = (_colorCounter + 1) & 0xFF;
    
    // Get the current palette color based on the counter using helper logic
    CRGB chaseColor = strip->ColorFromPaletteWithDistribution(
        *(strip->getCurrentPalette()),  // Current color palette
        _colorCounter,                   // Color index (cycles 0-255)
        255,                            // Full brightness
        seg->blendType                  // Blend type from segment settings
    );
    
    // Calculate the offset for the theater chase pattern using beat position
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, _timebase, EffectHelper::SLOW_SPEED);
    uint16_t offset = map(beatPosition, 0, 65535, 0, 255) % 3;
    
    // Apply the theater chase pattern to all LEDs in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Check if this LED position should be lit based on the offset
        if ((i % 3) == offset) {
            // Light this LED with the current rainbow color
            strip->leds[runtime->start + i] = chaseColor;
        } else {
            // Turn off LEDs that are not part of the current chase pattern
            strip->leds[runtime->start + i] = CRGB::Black;
        }
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* TheaterChaseRainbowEffect::getName() const {
    return F("Theater Chase Rainbow");
}

uint8_t TheaterChaseRainbowEffect::getModeId() const {
    return FX_MODE_THEATER_CHASE_RAINBOW;
}

// Register this effect with the factory system
// This allows the effect to be created dynamically when the mode is selected
REGISTER_EFFECT(FX_MODE_THEATER_CHASE_RAINBOW, TheaterChaseRainbowEffect)