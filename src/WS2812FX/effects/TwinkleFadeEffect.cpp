#include "TwinkleFadeEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool TwinkleFadeEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize timing variables
    _lastFadeTime = millis();
    
    return true;
}

uint16_t TwinkleFadeEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment and runtime data for effect parameters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    uint32_t currentTime = millis();
    
    // Perform fade operation at controlled intervals (similar to EVERY_N_MILLISECONDS)
    // Use strip minimum delay as the fade interval for consistent timing
    if (currentTime - _lastFadeTime >= strip->getStripMinDelay()) {
        // Calculate fade amount based on speed setting
        // Higher beat88 values result in faster fading
        uint8_t fadeAmount = qadd8(seg->beat88 >> 8, 12);
        
        // Apply fade effect using helper
        EffectHelper::applyFadeEffect(strip, fadeAmount);
        
        _lastFadeTime = currentTime;
    }
    
    // Count currently active (lit) LEDs to determine current spark density
    uint16_t numSparks = 0;
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Check if LED has any color (is not completely black)
        if (strip->leds[runtime->start + i]) {
            numSparks++;
        }
    }
    
    // Calculate target number of active LEDs based on twinkle density setting using helper
    uint16_t maxSparks = EffectHelper::safeMapuint16_t((uint16_t)seg->twinkleDensity, 
                                              (uint16_t)0, (uint16_t)8, 
                                              (uint16_t)0, (uint16_t)runtime->length);
    
    // Decide whether to add new twinkles or dim existing ones
    if (numSparks < maxSparks) {
        // Below target density - add a new twinkle
        
        // Select a random LED position within the segment
        uint16_t ledIndex = random16(runtime->length);
        uint16_t absoluteIndex = runtime->start + ledIndex;
        
        if (!strip->leds[absoluteIndex]) {
            // LED is currently off - light it up with a new twinkle
            
            // Generate random color from current palette
            uint8_t randomColorIndex = random8();
            
            // Generate random brightness (128-255 for good visibility)
            uint8_t randomBrightness = random8(128, 255);
            
            // Set the LED color using palette with distribution
            strip->leds[absoluteIndex] = strip->ColorFromPaletteWithDistribution(
                *(strip->getCurrentPalette()),  // Current color palette
                randomColorIndex,               // Random color index
                randomBrightness,              // Random brightness
                seg->blendType                 // Blend type from segment settings
            );
        } else {
            // do not add twinkle if LED is already lit
            // This prevents overwriting existing twinkles
        }
    }
    else
    {
        // do nothing if we are at or above target density
    }
    // If numSparks >= maxSparks, we're at or above target density
    // Just let existing twinkles fade naturally without adding new ones
    
    // Return 0 for immediate next update (matches original implementation)
    // This allows for smooth, responsive twinkling
    return 0;
}

const __FlashStringHelper* TwinkleFadeEffect::getName() const {
    return F("Twinkle Fade");
}

uint8_t TwinkleFadeEffect::getModeId() const {
    return FX_MODE_TWINKLE_FADE;
}

// Register this effect with the factory system
// This allows the effect to be created dynamically when the mode is selected
REGISTER_EFFECT(FX_MODE_TWINKLE_FADE, TwinkleFadeEffect)