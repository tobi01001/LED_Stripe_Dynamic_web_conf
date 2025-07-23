#include "RunningLightsEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool RunningLightsEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from EffectHelper
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t RunningLightsEffect::update(WS2812FX* strip) {
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
    
    // Calculate the moving wave offset based on beat88 timing
    // Map beat position to extended range (length * 10) for smoother movement
    uint16_t waveOffset = map(beat88(seg->beat88, timebase), 
                             (uint16_t)0, (uint16_t)65535, 
                             (uint16_t)0, (uint16_t)(runtime->length * 10));
    
    // Process each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate sine wave brightness for this LED position
        // Map LED position to full sine wave range (0-255)
        uint8_t sinePosition = map(i, (uint16_t)0, (uint16_t)(runtime->length - 1), 
                                  (uint16_t)0, (uint16_t)255);
        
        // Generate sine wave brightness, subtract small amount to create darker valleys
        uint8_t luminosity = qsub8(sin8_C(sinePosition), 2);
        
        // Calculate the final LED position with wave offset
        // Add current LED index to wave offset and wrap around segment length
        uint16_t finalOffset = (waveOffset + i) % runtime->length;
        
        // Calculate color index based on the offset position
        // Map position back to color range and add base hue for color cycling
        uint8_t colorIndex = map(finalOffset, (uint16_t)0, (uint16_t)(runtime->length - 1), 
                                (uint16_t)0, (uint16_t)255) + runtime->baseHue;
        
        // Get color from palette with calculated brightness and distribution
        CRGB newColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            colorIndex, 
            luminosity, 
            seg->blendType);
        
        // Blend new color with existing LED color for smooth transitions
        // Use speed-based blend amount (beat88 >> 8) plus minimum blend value
        uint8_t blendAmount = qadd8(seg->beat88 >> 8, 16);
        // Ensure the calculated index is within bounds
        if (runtime->start + finalOffset < runtime->stop) {
            nblend(strip->leds[runtime->start + finalOffset], newColor, blendAmount);
        }
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* RunningLightsEffect::getName() const {
    return F("Running Lights");
}

uint8_t RunningLightsEffect::getModeId() const {
    return FX_MODE_RUNNING_LIGHTS;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_RUNNING_LIGHTS, RunningLightsEffect)