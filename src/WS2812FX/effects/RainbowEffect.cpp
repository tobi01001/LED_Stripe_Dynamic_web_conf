#include "RainbowEffect.h"
#include "../WS2812FX_FastLed.h"

bool RainbowEffect::init(WS2812FX* strip) {
    if (initialized) {
        return true; // Already initialized
    }
    
    // Initialize private state variables
    timebase = millis();    // Set time reference for consistent beat calculations
    
    // Get segment runtime data through the strip
    auto runtime = strip->getSegmentRuntime();
    
    // Reset the runtime modeinit flag to indicate proper initialization
    runtime->modeinit = false;
    
    initialized = true;
    return true;
}

uint16_t RainbowEffect::update(WS2812FX* strip) {
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Get current palette for color generation
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    // Calculate current position in rainbow cycle using beat88 function
    // beat88 provides smooth, continuous timing based on speed parameter
    // The result is mapped from 16-bit beat range to 8-bit palette index range
    uint16_t beatValue = beat88(seg->beat88, timebase);
    uint8_t paletteIndex = map(beatValue, 
                              (uint16_t)0, (uint16_t)65535,    // Input range: full 16-bit beat88 range
                              (uint16_t)0, (uint16_t)255);     // Output range: 8-bit palette index
    
    // Generate single color from palette at calculated position
    // ColorFromPaletteWithDistribution applies palette distribution settings
    // and ensures consistent color appearance across different palettes
    CRGB rainbowColor = strip->ColorFromPaletteWithDistribution(
        *currentPalette,
        paletteIndex,
        seg->brightness,        // Apply current brightness setting
        seg->blendType          // Use current blend mode for smooth transitions
    );
    
    // Fill entire segment with the calculated rainbow color
    // fill_solid ensures all LEDs in the segment display the same color
    // This creates the characteristic "all LEDs same color" rainbow effect
    fill_solid(&strip->leds[runtime->start], runtime->length, rainbowColor);
    
    // Return minimum delay for smooth color transitions
    // The timing is controlled by the beat88 function and timebase
    return strip->getStripMinDelay();
}

const __FlashStringHelper* RainbowEffect::getName() const {
    return F("Rainbow");
}

uint8_t RainbowEffect::getModeId() const {
    return FX_MODE_RAINBOW;
}

void RainbowEffect::cleanup() {
    // Reset state for clean transition to next effect
    initialized = false;
    timebase = 0;
}

// Register this effect with the factory system
// This enables automatic creation when FX_MODE_RAINBOW is requested
REGISTER_EFFECT(FX_MODE_RAINBOW, RainbowEffect)