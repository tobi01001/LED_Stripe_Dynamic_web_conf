#include "MoveBarCubeEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t MoveBarCubeEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay();
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
    
    // Calculate speed mapping using helper
    const uint16_t speed = EffectHelper::safeMapuint16_t(seg->beat88 > (20000 / seg->segments) ? (20000 / seg->segments) : seg->beat88, 0, (20000 / seg->segments), 0, 65535);
    
    
    // Apply background fade effect using helper
    const uint8_t fadeAmount = EffectHelper::safeMapuint16_t(speed, 0, 65535, 64, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
    
    // Calculate the cubic wave position for smooth movement
    uint16_t position = calculateCubicPosition(speed, width);
    
    // Calculate color increment for smooth palette distribution
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    strip->drawFractionalBar(position, width, *strip->getCurrentPalette(), 
                            runtime->baseHue, 255, false, colorIncrement);
    
    return strip->getStripMinDelay();
}

uint16_t MoveBarCubeEffect::calculateCubicPosition(uint16_t speed, uint16_t width) {
    // Use cubic easing for dramatic acceleration/deceleration
    uint16_t triangleWave = EffectHelper::triwave16(beat88(speed / 2, _timebase));
    uint16_t easedWave = EffectHelper::ease16InOutCubic(triangleWave);
    return EffectHelper::safeMapuint16_t(easedWave, (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(width * 16));
}


const __FlashStringHelper* MoveBarCubeEffect::getName() const {
    return F("1/2 Bar3");
}

uint8_t MoveBarCubeEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_CUBE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_CUBE, MoveBarCubeEffect)