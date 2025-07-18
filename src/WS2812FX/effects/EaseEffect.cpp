#include "EaseEffect.h"
#include "../WS2812FX_FastLed.h"

bool EaseEffect::init(WS2812FX* strip) {
    if (initialized) {
        return true; // Already initialized
    }
    
    // Get segment and runtime data through the strip
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Initialize ease-specific variables
    trigger = false;
    beat = seg->beat88;
    oldbeat = seg->beat88;
    p_lerp = 0;
    timebase = millis();
    
    // Reset the runtime modeinit flag
    runtime->modeinit = false;
    
    initialized = true;
    return true;
}

uint16_t EaseEffect::update(WS2812FX* strip) {
    // Get segment and runtime data through the strip
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Get current color (stays at baseHue instead of moving around palette)
    uint8_t colorMove = runtime->baseHue;
    
    // Fade out tail based on speed
    strip->fade_out(seg->beat88 >> 5);
    
    // Calculate sine curve for LED position (factor 16 for fractional bar)
    uint16_t lerpVal = beatsin88(beat, 
                                runtime->start * 16, 
                                runtime->stop * 16 - (WIDTH * 16), 
                                timebase);
    
    // Check if we're in the middle to modify speed
    if (lerpVal == ((runtime->length * 16) / 2)) {
        // Trigger is used because we may be in middle for multiple frames
        if (trigger) {
            // If external speed changed, refresh values
            if (oldbeat != seg->beat88) {
                beat = seg->beat88;
                oldbeat = seg->beat88;
            }
            
            // Reset trigger and timebase for smooth animation
            trigger = false;
            timebase = millis();
            
            // Randomly adjust beat speed
            if (beat < 255) {
                beat += 2 * random8();
            } else {
                beat += 2 * (128 - random8());
            }
        }
    } else {
        // Activate trigger if position changed
        if (lerpVal != p_lerp) {
            trigger = true;
        }
    }
    
    p_lerp = lerpVal;
    
    // Draw two fractional bars - one normal, one mirrored
    CRGBPalette16* palette = strip->getCurrentPalette();
    uint8_t colorIndex = (uint8_t)((uint8_t)(lerpVal / 16 - runtime->start) + colorMove);
    
    strip->drawFractionalBar(lerpVal, WIDTH, *palette, colorIndex, 255, true, 1);
    strip->drawFractionalBar((runtime->stop * 16) - lerpVal, WIDTH, *palette, colorIndex, 255, true, 1);
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* EaseEffect::getName() const {
    return F("Ease");
}

uint8_t EaseEffect::getModeId() const {
    return FX_MODE_EASE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_EASE, EaseEffect)