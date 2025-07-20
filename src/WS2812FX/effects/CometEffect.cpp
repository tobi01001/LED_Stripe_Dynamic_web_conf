#include "CometEffect.h"
#include "../WS2812FX_FastLed.h"

// Include FastLED lib8tion for beat88 function
#include "lib8tion.h"

bool CometEffect::init(WS2812FX* strip) {
    // Initialize the timebase for consistent animation timing
    // This ensures the comet starts its journey smoothly from the beginning
    timebase = millis();
    
    // Mark initialization as complete - this is used by the base system
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t CometEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip's public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate comet width proportional to strip length (1/15th of total length, minimum 1)
    // This ensures the comet scales appropriately for different strip lengths
    const uint16_t width = max(1, runtime->length / 15);
    
    // Apply fade out effect to create the comet's trailing tail - intensity 96 provides smooth fading
    // This creates the characteristic diminishing brightness behind the comet head
    strip->fade_out(96);
    
    // Generate linear movement across the strip using beat88 function
    // The beat88 function provides consistent timing based on the effect speed setting
    // Multiplying by 4 increases the movement speed for more dynamic motion
    // Output range 0-65535 is mapped to the full strip length in 16-bit fixed point
    uint16_t pos = map(beat88(seg->beat88 * 4, timebase), 
                       (uint16_t)0, (uint16_t)65535, 
                       (uint16_t)0, (uint16_t)(runtime->length * 16));
    
    // Calculate the absolute position by adding the relative position to the strip start
    // This accounts for segmented strips where the effect may not start at LED 0
    uint16_t absolutePos = runtime->start * 16 + pos;
    
    // Calculate color index that progresses along the strip
    // This creates a rainbow-like color progression as the comet moves
    // The color is mapped from the current position across the full strip range
    uint8_t colorIndex = map(absolutePos, 
                            (uint16_t)(runtime->start * 16), 
                            (uint16_t)(runtime->stop * 16), 
                            (uint16_t)0, (uint16_t)255) + runtime->baseHue;
    
    // Draw the fractional comet using the drawFractionalBar function
    // - absolutePos: position in 16-bit fixed point for sub-pixel accuracy
    // - width: comet width in pixels
    // - currentPalette: use current color palette for consistent theming
    // - colorIndex: dynamic color that changes based on position
    // - brightness: maximum brightness (255) for vibrant comet head
    // - mixColor: true to blend colors smoothly across the comet width
    // - incindex: 1 for gradual color progression across the comet
    strip->drawFractionalBar(absolutePos,
                            width,
                            *strip->getCurrentPalette(),
                            colorIndex, 
                            255, true, 1);
    
    // Return minimum delay for smooth animation
    // The strip system will handle timing constraints automatically
    return strip->getStripMinDelay();
}

const __FlashStringHelper* CometEffect::getName() const {
    return F("Comet");
}

uint8_t CometEffect::getModeId() const {
    return FX_MODE_COMET;
}

// Register this effect with the factory system
// This allows the effect to be created automatically when the mode is selected
REGISTER_EFFECT(FX_MODE_COMET, CometEffect)