#include "EaseEffect.h"
#include "../WS2812FX_FastLed.h"

bool EaseEffect::init(WS2812FX* strip, segment* seg, segment_runtime* runtime) {
    if (!isFirstFrame(runtime)) {
        return true; // Already initialized
    }
    
    mode_variables& mv = getModeVars(runtime);
    
    // Initialize ease-specific variables
    mv.ease.trigger = false;
    mv.ease.beat = seg->beat88;
    mv.ease.oldbeat = seg->beat88;
    mv.ease.p_lerp = 0;
    mv.ease.timebase = millis();
    
    setInitialized(runtime);
    return true;
}

uint16_t EaseEffect::update(WS2812FX* strip, segment* seg, segment_runtime* runtime) {
    mode_variables& mv = getModeVars(runtime);
    
    // Get current color (stays at baseHue instead of moving around palette)
    uint8_t colorMove = runtime->baseHue;
    
    // Fade out tail based on speed
    strip->fade_out(seg->beat88 >> 5);
    
    // Calculate sine curve for LED position (factor 16 for fractional bar)
    uint16_t lerpVal = beatsin88(mv.ease.beat, 
                                runtime->start * 16, 
                                runtime->stop * 16 - (WIDTH * 16), 
                                mv.ease.timebase);
    
    // Check if we're in the middle to modify speed
    if (lerpVal == ((runtime->length * 16) / 2)) {
        // Trigger is used because we may be in middle for multiple frames
        if (mv.ease.trigger) {
            // If external speed changed, refresh values
            if (mv.ease.oldbeat != seg->beat88) {
                mv.ease.beat = seg->beat88;
                mv.ease.oldbeat = seg->beat88;
            }
            
            // Reset trigger and timebase for smooth animation
            mv.ease.trigger = false;
            mv.ease.timebase = millis();
            
            // Randomly adjust beat speed
            if (mv.ease.beat < 255) {
                mv.ease.beat += 2 * random8();
            } else {
                mv.ease.beat += 2 * (128 - random8());
            }
        }
    } else {
        // Activate trigger if position changed
        if (lerpVal != mv.ease.p_lerp) {
            mv.ease.trigger = true;
        }
    }
    
    mv.ease.p_lerp = lerpVal;
    
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