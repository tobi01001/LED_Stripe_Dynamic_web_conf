#include "RainbowCycleEffect.h"
#include "../WS2812FX_FastLed.h"

bool RainbowCycleEffect::init(WS2812FX* strip) {
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

uint16_t RainbowCycleEffect::update(WS2812FX* strip) {
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Get current palette for color generation
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    // Calculate current position in rainbow cycle using beat88 function
    // beat88 provides smooth, continuous timing based on speed parameter
    // The result is mapped from 16-bit beat range to 8-bit palette index range
    uint16_t beatValue = beat88(seg->beat88, timebase);
    uint8_t startingPaletteIndex = map(beatValue,
                                      (uint16_t)0, (uint16_t)65535,    // Input range: full 16-bit beat88 range
                                      (uint16_t)0, (uint16_t)255);     // Output range: 8-bit palette index
    
    // Calculate color spacing (delta) based on segment length and palette distribution
    // The palette distribution setting controls how stretched or compressed the rainbow appears
    // Higher values create more compressed rainbows, lower values create more stretched rainbows
    // The calculation ensures at least 1 step between colors to avoid solid color appearance
    uint8_t deltaHue = max(1, (256 * 100 / (runtime->length * seg->paletteDistribution)));
    
    // Fill the segment with rainbow colors using FastLED's fill_palette function
    // This function automatically distributes colors from the palette across the strip
    // Parameters:
    // - &strip->leds[runtime->start]: Starting LED position in the strip
    // - runtime->length: Number of LEDs to fill
    // - startingPaletteIndex: Starting position in palette (cycles over time)
    // - deltaHue: Step size between adjacent LEDs in palette space
    // - *currentPalette: The color palette to use
    // - 255: Brightness (full brightness, actual brightness controlled by strip)
    // - seg->blendType: Blending mode for smooth color transitions
    fill_palette(&strip->leds[runtime->start],
                runtime->length,
                startingPaletteIndex,           // Starting palette position (cycles for animation)
                deltaHue,                       // Color step between adjacent LEDs
                *currentPalette,
                255,                            // Full internal brightness
                seg->blendType);                // Use configured blend mode
    
    // Return minimum delay for smooth animation
    // The timing is controlled by the beat88 function and timebase
    return strip->getStripMinDelay();
}

const __FlashStringHelper* RainbowCycleEffect::getName() const {
    return F("Rainbow Cycle");
}

uint8_t RainbowCycleEffect::getModeId() const {
    return FX_MODE_RAINBOW_CYCLE;
}

void RainbowCycleEffect::cleanup() {
    // Reset state for clean transition to next effect
    initialized = false;
    timebase = 0;
}

// Register this effect with the factory system
// This enables automatic creation when FX_MODE_RAINBOW_CYCLE is requested
REGISTER_EFFECT(FX_MODE_RAINBOW_CYCLE, RainbowCycleEffect)