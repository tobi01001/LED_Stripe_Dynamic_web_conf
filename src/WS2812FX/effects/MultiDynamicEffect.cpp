#include "MultiDynamicEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool MultiDynamicEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize private state variables
    nextUpdate = 0;         // Reset timer to trigger immediate update
    lastColorIndex = 0;     // Start with first palette index
    
    return true;
}

uint16_t MultiDynamicEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Get current system time
    uint32_t currentTime = millis();
    
    // Check if it's time to update colors based on speed setting
    if (currentTime > nextUpdate) {
        
        // Update all LEDs in the segment with new random colors
        for (uint16_t i = runtime->start; i <= runtime->stop; i++) {
            // Generate next random palette index using the strip's random function
            lastColorIndex = EffectHelper::get_random_wheel_index(lastColorIndex, 32);
            
            // Get current palette and apply color with distribution
            CRGBPalette16* currentPalette = strip->getCurrentPalette();
            
            // Apply color from palette with brightness and blending
            strip->leds[i] = strip->ColorFromPaletteWithDistribution(
                *currentPalette, 
                lastColorIndex, 
                seg->brightness,    // Use current brightness setting
                seg->blendType      // Use current blend mode
            );
        }
        
        // Calculate next update time using EffectHelper safe mapping
        uint32_t interval = EffectHelper::safeMapuint16_t(seg->beat88, 0, BEAT88_MAX, 255, 4) << 4;
        nextUpdate = currentTime + interval;
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* MultiDynamicEffect::getName() const {
    return F("Dynamic");
}

uint8_t MultiDynamicEffect::getModeId() const {
    return FX_MODE_MULTI_DYNAMIC;
}

void MultiDynamicEffect::cleanup() {
    // Reset state for clean transition to next effect
    initialized = false;
    nextUpdate = 0;
    lastColorIndex = 0;
}

// Register this effect with the factory system
// This enables automatic creation when FX_MODE_MULTI_DYNAMIC is requested
REGISTER_EFFECT(FX_MODE_MULTI_DYNAMIC, MultiDynamicEffect)