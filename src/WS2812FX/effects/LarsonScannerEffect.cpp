#include "LarsonScannerEffect.h"
#include "../WS2812FX_FastLed.h"

// Include FastLED lib8tion for beat88 function
#include "lib8tion.h"

bool LarsonScannerEffect::init(WS2812FX* strip) {
    // Initialize the timebase for consistent animation timing
    // This ensures the effect starts smoothly regardless of when it's activated
    timebase = millis();
    
    // Mark initialization as complete - this is used by the base system
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t LarsonScannerEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip's public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate bar width proportional to strip length (1/15th of total length, minimum 1)
    // This ensures the effect scales appropriately for different strip lengths
    const uint16_t width = max(1, runtime->length / 15);
    
    // Apply fade out effect to create trailing - intensity 96 provides smooth trailing
    // This creates the characteristic "comet tail" effect as the bar moves
    strip->fade_out(96);
    
    // Generate smooth bouncing motion using triangular wave function
    // The beat88 function provides consistent timing based on the effect speed setting
    // Multiplying by 4 increases the movement speed for more dynamic action
    uint16_t pos = triwave16(beat88(seg->beat88 * 4, timebase));
    
    // Map the triangular wave output (0-65535) to the actual LED strip positions
    // The mapping accounts for the bar width to ensure it doesn't exceed strip boundaries
    // Position values are in 16-bit fixed point for smooth sub-pixel positioning
    pos = map(pos, 
              (uint16_t)0, (uint16_t)65535, 
              (uint16_t)(runtime->start * 16), 
              (uint16_t)(runtime->stop * 16 - width * 16));
    
    // Draw the fractional bar with smooth positioning and color cycling
    // - pos: position in 16-bit fixed point for sub-pixel accuracy
    // - width: bar width in pixels  
    // - currentPalette: use current color palette for consistent theming
    // - color index: cycles based on position and base hue for dynamic coloring
    // - brightness: maximum brightness (255) for vibrant colors
    // - mixColor: true to blend colors smoothly
    // - incindex: 1 for gradual color progression across the bar
    strip->drawFractionalBar(pos,
                            width,
                            *strip->getCurrentPalette(),
                            runtime->baseHue + map(pos, 
                                                  (uint16_t)(runtime->start * 16), 
                                                  (uint16_t)(runtime->stop * 16 - width * 16), 
                                                  (uint16_t)0, (uint16_t)255), 
                            255, true, 1);
    
    // Return minimum delay for smooth animation
    // The strip system will handle timing constraints automatically
    return strip->getStripMinDelay();
}

const __FlashStringHelper* LarsonScannerEffect::getName() const {
    return F("Larson Scanner");
}

uint8_t LarsonScannerEffect::getModeId() const {
    return FX_MODE_LARSON_SCANNER;
}

// Register this effect with the factory system
// This allows the effect to be created automatically when the mode is selected
REGISTER_EFFECT(FX_MODE_LARSON_SCANNER, LarsonScannerEffect)