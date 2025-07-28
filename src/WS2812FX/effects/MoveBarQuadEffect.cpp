#include "MoveBarQuadEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t MoveBarQuadEffect::update(WS2812FX* strip) {
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
    
    // Calculate quadratic movement position
    uint16_t position = calculateQuadPosition(strip,speed, width);
    
    // Calculate color increment for smooth palette distribution
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    strip->drawFractionalBar(position, width, *strip->getCurrentPalette(), 
                            runtime->baseHue, 255, false, colorIncrement);

    return strip->getStripMinDelay();
}

uint16_t MoveBarQuadEffect::calculateQuadPosition(WS2812FX* strip, uint16_t speed, uint16_t width) {
    uint16_t triangleWave = EffectHelper::triwave16(beat88(speed / 2, _timebase));
       
    // Apply quadratic easing for dramatic acceleration/deceleration
    uint16_t easedWave = EffectHelper::ease16InOutQuad(triangleWave); // Scale to 16-bit for easing
    
    // Map to bar movement range
    return EffectHelper::safeMapuint16_t(easedWave, 0, 65535, 0, width * 16);
}

const __FlashStringHelper* MoveBarQuadEffect::getName() const {
    return F("1/2 Bar2");
}

uint8_t MoveBarQuadEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_QUAD;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_QUAD, MoveBarQuadEffect)