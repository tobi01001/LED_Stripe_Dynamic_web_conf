#include "CometEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

// Include FastLED lib8tion for beat88 function
#include "lib8tion.h"

bool CometEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false; // Local since CometEffect doesn't maintain persistent state
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t CometEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment and runtime data through the strip's public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate comet width proportional to strip length using helper
    const uint16_t width = EffectHelper::calculateProportionalWidth(strip, 15, 1);
    
    // Apply fade out effect to create the comet's trailing tail using helper
    EffectHelper::applyFadeEffect(strip, EffectHelper::MEDIUM_FADE);
    
    // Generate linear movement across the strip using helper with increased speed
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, timebase, EffectHelper::FAST_SPEED);
    uint16_t pos = EffectHelper::mapPositionToStrip(strip, beatPosition);
    
    // Calculate the absolute position by adding the relative position to the strip start
    uint16_t absolutePos = runtime->start * 16 + pos;
    
    // Calculate color index that progresses along the strip using helper
    uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, absolutePos);
    
    // Draw the fractional comet using the drawFractionalBar function
    strip->drawFractionalBar(absolutePos,
                            width,
                            *strip->getCurrentPalette(),
                            colorIndex, 
                            255, true, 1);
    
    // Return minimum delay for smooth animation
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