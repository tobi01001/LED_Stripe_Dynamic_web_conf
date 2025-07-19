#include "ScanEffect.h"
#include "../WS2812FX_FastLed.h"

bool ScanEffect::init(WS2812FX* strip) {
    // Initialize the timebase for consistent beat calculations
    // Using current millis ensures smooth continuation if effect is restarted
    timebase = millis();
    
    // Clear the mode initialization flag
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t ScanEffect::update(WS2812FX* strip) {
    // Get access to runtime and LED array
    auto runtime = strip->getSegmentRuntime();
    CRGB* leds = strip->leds;
    
    // Calculate the current position of the scanning bar
    // Uses triwave for smooth back-and-forth motion
    uint16_t led_offset = calculateBarPosition(strip);
    
    // Clear the entire segment to black before drawing the new bar position
    // This creates the clean scanning effect with only one bar visible
    fill_solid(&(leds[runtime->start]), runtime->length, CRGB::Black);
    
    // Calculate color index based on position for dynamic color changing
    uint8_t colorIndex = calculateColorIndex(led_offset, runtime->baseHue);
    
    // Draw the scanning bar using fractional positioning for smooth movement
    // The bar starts at the calculated offset position and has BAR_WIDTH pixels
    // Parameters:
    // - led_offset: 16-bit fractional position (actual_pos * 16)
    // - BAR_WIDTH: width of the bar in pixels
    // - current palette: for color selection
    // - colorIndex: index into the palette
    // - 255: full brightness
    // - true: mix colors for smooth blending
    // - 1: color increment for slight variation across bar width
    strip->drawFractionalBar(runtime->start * 16 + led_offset, BAR_WIDTH, 
                           *strip->getCurrentPalette(), colorIndex, 255, true, 1);
    
    // Return minimum delay for smooth animation
    // Scanning effects benefit from high frame rates for smooth motion
    return strip->getStripMinDelay();
}

const __FlashStringHelper* ScanEffect::getName() const {
    return F("Scan");
}

uint8_t ScanEffect::getModeId() const {
    return FX_MODE_SCAN;
}

// Helper method implementations
uint16_t ScanEffect::calculateBarPosition(WS2812FX* strip) const {
    // Get the current beat value using our timebase for consistent timing
    // beat88() returns a value that cycles from 0 to 65535 based on the speed setting
    uint16_t beatValue = beat88(strip->getSegment()->beat88, timebase);
    
    // Apply triwave function to create smooth back-and-forth motion
    // triwave16() converts the linear beat progression into a triangular wave
    // that smoothly goes from 0 to max and back to 0
    uint16_t triangularBeat = triwave16(beatValue);
    
    // Get runtime to access segment boundaries
    auto runtime = strip->getSegmentRuntime();
    
    // Map the triangular wave to the segment boundaries
    // We map to 16-bit fractional positions for sub-pixel accuracy
    // The range is adjusted to account for the bar width to prevent overflow
    uint16_t startPos = runtime->start * 16;  // Convert start to 16-bit fractional
    uint16_t endPos = runtime->stop * 16 - BAR_WIDTH * 16;  // Account for bar width
    
    // Map from the full triangle wave range (0-65535) to our segment range
    uint16_t position = map(triangularBeat, 
                           (uint16_t)0, (uint16_t)65535,      // Input range
                           (uint16_t)0, endPos - startPos);   // Output range (relative to segment)
    
    return position;
}

uint8_t ScanEffect::calculateColorIndex(uint16_t position_16bit, uint8_t baseHue) const {
    // Convert 16-bit fractional position back to pixel position for color calculation
    uint16_t pixelPosition = position_16bit / 16;
    
    // Create a color index that changes based on the bar's position
    // This makes the bar color shift as it moves across the strip
    // The baseHue provides the base color offset, while the position creates variation
    uint8_t colorIndex = pixelPosition + baseHue;
    
    return colorIndex;
}

// Register this effect with the factory so it can be created by mode ID
REGISTER_EFFECT(FX_MODE_SCAN, ScanEffect)