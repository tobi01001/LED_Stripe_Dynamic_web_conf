#include "DualScanEffect.h"
#include "../WS2812FX_FastLed.h"

bool DualScanEffect::init(WS2812FX* strip) {
    // Initialize the timebase for consistent beat calculations
    // Using current millis ensures smooth continuation if effect is restarted
    timebase = millis();
    
    // Clear the mode initialization flag
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t DualScanEffect::update(WS2812FX* strip) {
    // Get access to runtime and LED array
    auto runtime = strip->getSegmentRuntime();
    CRGB* leds = strip->leds;
    
    // Calculate the current positions of both scanning bars
    uint16_t forwardBarOffset = calculateForwardBarPosition(strip);
    uint16_t reverseBarOffset = calculateReverseBarPosition(strip);
    
    // Clear the entire segment to black before drawing the new bar positions
    // This creates the clean scanning effect with only the two bars visible
    fill_solid(&(leds[runtime->start]), runtime->length, CRGB::Black);
    
    // Calculate color indices for both bars
    uint8_t forwardColorIndex = calculateForwardColorIndex(forwardBarOffset, runtime->baseHue);
    uint8_t reverseColorIndex = calculateReverseColorIndex(reverseBarOffset, runtime->baseHue);
    
    // Draw the forward-moving bar (moves from start to end)
    // Position is relative to segment start
    strip->drawFractionalBar(runtime->start * 16 + forwardBarOffset, BAR_WIDTH,
                           *strip->getCurrentPalette(), forwardColorIndex, 255, true, 1);
    
    // Draw the reverse-moving bar (moves from end to start)  
    // Position calculation accounts for the mirrored movement
    strip->drawFractionalBar(runtime->stop * 16 - reverseBarOffset, BAR_WIDTH,
                           *strip->getCurrentPalette(), reverseColorIndex, 255, true, 1);
    
    // Return minimum delay for smooth animation
    // Dual scanning effects benefit from high frame rates for smooth motion
    return strip->getStripMinDelay();
}

const __FlashStringHelper* DualScanEffect::getName() const {
    return F("Dual Scan");
}

uint8_t DualScanEffect::getModeId() const {
    return FX_MODE_DUAL_SCAN;
}

// Helper method implementations
uint16_t DualScanEffect::calculateForwardBarPosition(WS2812FX* strip) const {
    // Get the current beat value using our timebase for consistent timing
    // beat88() returns a value that cycles from 0 to 65535 based on the speed setting
    uint16_t beatValue = beat88(strip->getSegment()->beat88, timebase);
    
    // Apply triwave function to create smooth back-and-forth motion
    // triwave16() converts the linear beat progression into a triangular wave
    uint16_t triangularBeat = triwave16(beatValue);
    
    // Get runtime to access segment boundaries
    auto runtime = strip->getSegmentRuntime();
    
    // Map the triangular wave to the segment boundaries for the forward bar
    // We map to 16-bit fractional positions for sub-pixel accuracy
    // The range is adjusted to account for the bar width to prevent overflow
    uint16_t segmentLength = (runtime->stop - runtime->start) * 16;
    uint16_t maxPosition = segmentLength - BAR_WIDTH * 16;
    
    // Map from the full triangle wave range (0-65535) to our segment range
    uint16_t position = map(triangularBeat, 
                           (uint16_t)0, (uint16_t)65535,    // Input range
                           (uint16_t)0, maxPosition);       // Output range
    
    return position;
}

uint16_t DualScanEffect::calculateReverseBarPosition(WS2812FX* strip) const {
    // Use the same beat calculation as the forward bar for synchronized movement
    uint16_t beatValue = beat88(strip->getSegment()->beat88, timebase);
    uint16_t triangularBeat = triwave16(beatValue);
    
    // Get runtime to access segment boundaries
    auto runtime = strip->getSegmentRuntime();
    
    // For the reverse bar, we use the same position calculation
    // The actual reversal happens in the drawing position calculation
    uint16_t segmentLength = (runtime->stop - runtime->start) * 16;
    uint16_t maxPosition = segmentLength - BAR_WIDTH * 16;
    
    uint16_t position = map(triangularBeat, 
                           (uint16_t)0, (uint16_t)65535,    // Input range
                           (uint16_t)0, maxPosition);       // Output range
    
    return position;
}

uint8_t DualScanEffect::calculateForwardColorIndex(uint16_t position_16bit, uint8_t baseHue) const {
    // Convert 16-bit fractional position back to pixel position for color calculation
    uint16_t pixelPosition = position_16bit / 16;
    
    // Create a color index that changes based on the bar's position
    // This makes the bar color shift as it moves across the strip
    uint8_t colorIndex = pixelPosition + baseHue;
    
    return colorIndex;
}

uint8_t DualScanEffect::calculateReverseColorIndex(uint16_t position_16bit, uint8_t baseHue) const {
    // Convert 16-bit fractional position back to pixel position for color calculation
    uint16_t pixelPosition = position_16bit / 16;
    
    // Create a complementary color index for the reverse bar
    // This provides visual distinction between the two bars
    // Using 255 - pixelPosition creates a color that moves in opposite direction
    uint8_t colorIndex = 255 - pixelPosition + baseHue;
    
    return colorIndex;
}

// Register this effect with the factory so it can be created by mode ID
REGISTER_EFFECT(FX_MODE_DUAL_SCAN, DualScanEffect)