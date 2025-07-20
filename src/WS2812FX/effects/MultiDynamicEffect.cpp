#include "MultiDynamicEffect.h"
#include "../WS2812FX_FastLed.h"

bool MultiDynamicEffect::init(WS2812FX* strip) {
    if (initialized) {
        return true; // Already initialized
    }
    
    // Initialize private state variables
    nextUpdate = 0;         // Reset timer to trigger immediate update
    lastColorIndex = 0;     // Start with first palette index
    
    // Get segment runtime data through the strip
    auto runtime = strip->getSegmentRuntime();
    
    // Reset the runtime modeinit flag to indicate proper initialization
    runtime->modeinit = false;
    
    initialized = true;
    return true;
}

uint16_t MultiDynamicEffect::update(WS2812FX* strip) {
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Get current system time
    uint32_t currentTime = millis();
    
    // Check if it's time to update colors based on speed setting
    // The interval calculation uses BEAT88_MAX to invert speed (higher speed = shorter interval)
    // Right shift by 6 provides reasonable timing range (similar to original implementation)
    if (currentTime > nextUpdate) {
        
        // Update all LEDs in the segment with new random colors
        for (uint16_t i = runtime->start; i <= runtime->stop; i++) {
            // Generate next random palette index using the strip's random function
            // This maintains consistent random distribution patterns
            lastColorIndex = strip->get_random_wheel_index(lastColorIndex, 32);
            
            // Get current palette and apply color with distribution
            CRGBPalette16* currentPalette = strip->getCurrentPalette();
            
            // Apply color from palette with brightness and blending
            // ColorFromPaletteWithDistribution handles palette distribution settings
            strip->leds[i] = strip->ColorFromPaletteWithDistribution(
                *currentPalette, 
                lastColorIndex, 
                seg->brightness,    // Use current brightness setting
                seg->blendType      // Use current blend mode
            );
        }
        
        // Calculate next update time based on speed parameter
        // Higher beat88 values = faster changes (shorter intervals)
        // The bit shift provides a reasonable timing range for visual effect
        uint32_t interval = (BEAT88_MAX - seg->beat88) >> 6;
        nextUpdate = currentTime + interval;
    }
    
    // Return minimum delay for smooth operation
    // The actual timing is controlled by the internal nextUpdate mechanism
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