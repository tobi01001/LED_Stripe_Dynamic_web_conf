#include "TheaterChaseDualPaletteEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t TheaterChaseDualPaletteEffect::update(WS2812FX* strip) {
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
    
    // Calculate current chase position using helper
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, _timebase, SPEED_DIVISOR);
    uint16_t chaseOffset = map(beatPosition, 0, 65535, 0, BEAT_RANGE_MAX) % CHASE_PATTERN_SIZE;
    
    // Apply the dual palette chase pattern to each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        uint16_t ledIndex = runtime->start + i;
        
        // Calculate base palette index using helper function
        uint8_t basePaletteIndex = EffectHelper::calculateColorIndexPosition(strip, i, runtime->baseHue);
        
        // Determine if this LED should be foreground (active chase) or background
        if ((i % CHASE_PATTERN_SIZE) == chaseOffset) {
            // Foreground (active chase) - full brightness palette color
            CRGB ledColor = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),
                basePaletteIndex,
                FOREGROUND_BRIGHTNESS,
                seg->blendType);
            
            strip->leds[ledIndex] = ledColor;
        } else {
            // Background - offset palette color with reduced brightness
            uint8_t backgroundPaletteIndex = basePaletteIndex + PALETTE_OFFSET;
            
            CRGB ledColor = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),
                backgroundPaletteIndex,
                BACKGROUND_BRIGHTNESS,
                seg->blendType);
            
            strip->leds[ledIndex] = ledColor;
        }
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* TheaterChaseDualPaletteEffect::getName() const {
    return F("Theater Chase Dual palette");
}

uint8_t TheaterChaseDualPaletteEffect::getModeId() const {
    return FX_MODE_THEATER_CHASE_DUAL_P;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_THEATER_CHASE_DUAL_P, TheaterChaseDualPaletteEffect)