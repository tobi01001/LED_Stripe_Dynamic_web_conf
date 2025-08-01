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
    
    // Calculate triangle position for sawtooth movement using EffectHelper
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, millis(), speed / 32768);
    uint16_t trianglePosition = EffectHelper::generateTriangleWave(beatPosition, 0, width * 16);
    
    // Draw the moving bar using EffectHelper
    EffectHelper::drawBar(strip, trianglePosition, width, runtime->baseHue, 255);
    
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