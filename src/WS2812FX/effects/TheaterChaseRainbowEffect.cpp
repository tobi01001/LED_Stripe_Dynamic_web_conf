#include "TheaterChaseRainbowEffect.h"
#include "../WS2812FX_FastLed.h"

bool TheaterChaseRainbowEffect::init(WS2812FX* strip) {
    // Initialize effect state variables
    _colorCounter = 0;
    _timebase = millis();  // Record current time for pattern timing
    
    // Access the segment runtime to mark initialization as complete
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t TheaterChaseRainbowEffect::update(WS2812FX* strip) {
    // Access segment and runtime data for effect parameters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Increment color counter for rainbow cycling, mask to 8 bits to prevent overflow
    _colorCounter = (_colorCounter + 1) & 0xFF;
    
    // Get the current palette color based on the counter
    // This creates the rainbow cycling effect as the counter increments
    CRGB chaseColor = strip->ColorFromPaletteWithDistribution(
        *(strip->getCurrentPalette()),  // Current color palette
        _colorCounter,                   // Color index (cycles 0-255)
        255,                            // Full brightness
        seg->blendType                  // Blend type from segment settings
    );
    
    // Calculate the offset for the theater chase pattern
    // Uses beat88() to create smooth, speed-controlled movement
    // The beat88 function creates a sawtooth wave from 0 to 65535 based on BPM and time
    uint16_t beatValue = beat88(seg->beat88 / 2, _timebase);
    
    // Map the beat value to 0-255 range and take modulo 3 for theater chase pattern
    // This creates the shifting pattern where every 3rd LED is lit
    uint16_t offset = map(beatValue, (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255) % 3;
    
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
    
    // Return the minimum delay for smooth animation
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