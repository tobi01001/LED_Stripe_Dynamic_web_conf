#include "MoveBarSinEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool MoveBarSinEffect::init(WS2812FX* strip) {
    // Use standardized initialization
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t MoveBarSinEffect::update(WS2812FX* strip) {
    // Validate strip pointer and initialization
    if (!EffectHelper::validateStripPointer(strip) || !initialized) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    
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
    uint16_t position = calculateSinePosition(speed, width);
    
    // Draw the moving bar at the calculated position
    drawMovingBar(strip, position, width);
    
    return strip->getStripMinDelay();
}

uint16_t MoveBarSinEffect::calculateSinePosition(uint16_t speed, uint16_t width) {
    // Use beatsin16 for smooth sine wave movement
    // The bar oscillates from position 0 to (width * 16) for fractional positioning
    return beatsin16(speed / 2, 0, width * 16, timebase);
}

void MoveBarSinEffect::applyBackgroundFade(WS2812FX* strip, uint16_t speed) {
    // Background fade is now handled in update() using EffectHelper
    // This method is kept for compatibility but no longer used
}

void MoveBarSinEffect::drawMovingBar(WS2812FX* strip, uint16_t position, uint16_t width) {
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate color increment for smooth palette distribution
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    strip->drawFractionalBar(position, width, *strip->getCurrentPalette(), 
                            runtime->baseHue, 255, false, colorIncrement);
}

const __FlashStringHelper* MoveBarSinEffect::getName() const {
    return F("1/2 Bar Sine");
}

uint8_t MoveBarSinEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_SIN;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_SIN, MoveBarSinEffect)