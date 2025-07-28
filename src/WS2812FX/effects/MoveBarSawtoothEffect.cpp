#include "MoveBarSawtoothEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t MoveBarSawtoothEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the bar width using helper function
    const uint16_t width = EffectHelper::calculateProportionalWidth(strip, 2, 1);  // 1/2 with min 1
    
    // Calculate speed mapping using helper
    const uint16_t speed = EffectHelper::safeMapuint16_t(seg->beat88 > (20000 / seg->segments) ? (20000 / seg->segments) : seg->beat88, 0, (20000 / seg->segments), 0, 65535);
    
    
    // Apply background fade effect using helper
    const uint8_t fadeAmount = EffectHelper::safeMapuint16_t(speed, 0, 65535, 64, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
    
    // Calculate triangle position for sawtooth movement using EffectHelper

    uint16_t trianglePosition = EffectHelper::triwave16(beat88(speed / 2, _timebase));
    trianglePosition = EffectHelper::safeMapuint16_t(trianglePosition, 0, 65535, 0, width * 16);
    
    // Calculate color increment for smooth palette distribution
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    strip->drawFractionalBar(trianglePosition, width, *strip->getCurrentPalette(), 
                            runtime->baseHue, 255, false, colorIncrement);
    
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* MoveBarSawtoothEffect::getName() const {
    return F("1/2 Bar");
}

uint8_t MoveBarSawtoothEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_SAWTOOTH;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_SAWTOOTH, MoveBarSawtoothEffect)