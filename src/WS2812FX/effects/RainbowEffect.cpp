#include "RainbowEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool RainbowEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool tempInit = false; // Use local temp variable instead of member
    bool result = EffectHelper::standardInit(strip, timebase, tempInit);
    setInitialized(result); // Set base class initialization state
    return result;
}

uint16_t RainbowEffect::update(WS2812FX* strip) {
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
    
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Get current palette for color generation
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    // Calculate current position in rainbow cycle using helper
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, timebase);
    uint8_t paletteIndex = map(beatPosition, 
                              (uint16_t)0, (uint16_t)65535,
                              (uint16_t)0, (uint16_t)255);
    
    // Generate single color from palette at calculated position
    CRGB rainbowColor = strip->ColorFromPaletteWithDistribution(
        *currentPalette,
        paletteIndex,
        seg->brightness,        // Apply current brightness setting
        seg->blendType          // Use current blend mode for smooth transitions
    );
    
    // Fill entire segment with the calculated rainbow color
    fill_solid(&strip->leds[runtime->start], runtime->length, rainbowColor);
    
    // Return minimum delay for smooth color transitions
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
    setInitialized(false);
    timebase = 0;
}

// Register this effect with the factory system
// This enables automatic creation when FX_MODE_RAINBOW is requested
REGISTER_EFFECT(FX_MODE_RAINBOW, RainbowEffect)