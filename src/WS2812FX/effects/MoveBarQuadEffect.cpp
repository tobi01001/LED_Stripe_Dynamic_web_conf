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
    
    // Access segment and runtime data through the strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the bar width using EffectHelper
    const uint16_t width = EffectHelper::calculateProportionalWidth(strip, 2, 1);
    
    // Calculate speed mapping from beat88 setting
    uint16_t constrainedSpeed = seg->beat88 > (20000 / seg->segments) ? 
                                (20000 / seg->segments) : seg->beat88;
    const uint16_t speed = EffectHelper::safeMapuint16_t(constrainedSpeed, 0, (20000 / seg->segments), 0, 65535);
    
    // Apply background fade effect using EffectHelper
    uint8_t fadeAmount = EffectHelper::safeMapuint16_t(speed, 0, 65535, 64, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
    
    // Calculate quadratic movement position
    uint16_t position = calculateQuadPosition(strip,speed, width);
    
    // Draw the moving bar using EffectHelper
    EffectHelper::drawBar(strip, position, width, runtime->baseHue, 255);
    
    return strip->getStripMinDelay();
}

uint16_t MoveBarQuadEffect::calculateQuadPosition(WS2812FX* strip, uint16_t speed, uint16_t width) {
    // Use triangle wave for smooth movement with quadratic easing
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, millis(), speed / 32768);
    uint16_t triangleWave = EffectHelper::generateTriangleWave(beatPosition, 0, 255);
    
    // Apply quadratic easing for dramatic acceleration/deceleration
    uint16_t easedWave = EffectHelper::ease16InOutQuad(triangleWave << 8); // Scale to 16-bit for easing
    
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