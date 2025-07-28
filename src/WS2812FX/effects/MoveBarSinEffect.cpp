#include "MoveBarSinEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t MoveBarSinEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer and initialization
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the bar width using helper function
    const uint16_t width = EffectHelper::calculateProportionalWidth(strip, 2, 1);  // 1/2 with min 1
    
    // Calculate speed mapping from beat88 setting using helper
    uint16_t constrainedSpeed = seg->beat88 > (20000 / seg->segments) ? 
                                (20000 / seg->segments) : seg->beat88;
    const uint16_t speed = EffectHelper::safeMapuint16_t(constrainedSpeed, 0, (20000 / seg->segments), 0, 65535);
    
    // Apply background fade effect using helper
    uint8_t fadeAmount = EffectHelper::safeMapuint16_t(speed, 0, 65535, 64, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
    
    // Calculate the sine wave position for smooth movement
    uint16_t position = beatsin16(speed / 2, 0, width * 16, _timebase);
    // Calculate color increment for smooth palette distribution
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    strip->drawFractionalBar(position, width, *strip->getCurrentPalette(),
                            runtime->baseHue, 255, false, colorIncrement);
    
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* MoveBarSinEffect::getName() const {
    return F("1/2 Bar Sine");
}

uint8_t MoveBarSinEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_SIN;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_SIN, MoveBarSinEffect)