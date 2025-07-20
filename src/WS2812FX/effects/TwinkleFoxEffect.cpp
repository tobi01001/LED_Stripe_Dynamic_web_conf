#include "TwinkleFoxEffect.h"
#include "../WS2812FX_FastLed.h"

bool TwinkleFoxEffect::init(WS2812FX* strip) {
    // TwinkleFox requires no persistent state initialization
    // All state is computed fresh each frame for deterministic behavior
    
    // Access the segment runtime to mark initialization as complete
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint8_t TwinkleFoxEffect::attackDecayWave8(uint8_t phase) {
    /**
     * Create a natural-looking brightness curve with sharp attack and slow decay.
     * This mimics how real fireflies and stars appear to twinkle.
     * 
     * Phase 0-85: Attack phase - rapid brightness increase (linear ramp up)
     * Phase 86-255: Decay phase - slower brightness decrease (non-linear decay)
     */
    if (phase < 86) {
        // Attack phase: linear ramp from 0 to 255 over first 86 steps
        return phase * 3;  // 86 * 3 = 258, clamped to 255
    } else {
        // Decay phase: slower non-linear decay from 255 to 0
        phase -= 86;  // Normalize to 0-169 range
        return 255 - (phase + (phase / 2));  // Non-linear decay
    }
}

CRGB TwinkleFoxEffect::computeOneTwinkle(uint32_t* timeMs, uint8_t* salt, WS2812FX* strip) {
    /**
     * Compute the color and brightness for one LED's twinkle cycle.
     * This implements the core TwinkleFox algorithm for deterministic,
     * organic-looking twinkles.
     */
    
    // Access segment settings for twinkle parameters
    auto seg = strip->getSegment();
    
    // Extract timing information from the time value
    // The twinkleSpeed controls how fast the twinkling cycles occur
    uint16_t ticks = *timeMs >> (8 - seg->twinkleSpeed);
    
    // Split timing into fast and slow components
    uint8_t fastCycle8 = ticks;  // Low 8 bits for brightness wave
    uint16_t slowCycle16 = (ticks >> 8) + *salt;  // High bits + salt for color/density
    
    // Add some non-linearity to the slow cycle for more organic behavior
    slowCycle16 += sin8(slowCycle16);
    slowCycle16 = (slowCycle16 * 2053) + 1384;  // PRNG formula
    uint8_t slowCycle8 = (slowCycle16 & 0xFF) + (slowCycle16 >> 8);
    
    // Determine if this pixel should be lit during this cycle
    // Based on twinkleDensity setting (0=none lit, 8=all lit)
    uint8_t bright = 0;
    if (((slowCycle8 & 0x0E) / 2) < seg->twinkleDensity) {
        // This pixel is active - compute its brightness using attack/decay wave
        bright = attackDecayWave8(fastCycle8);
    }
    
    // Generate color for this pixel
    CRGB color;
    if (bright > 0) {
        // Pixel is lit - get color from palette
        uint8_t hue = slowCycle8 - *salt;  // Hue based on slow cycle and salt
        color = strip->ColorFromPaletteWithDistribution(
            *(strip->getCurrentPalette()),  // Current color palette
            hue,                           // Color index
            bright,                        // Brightness value
            seg->blendType                 // Blend type from segment settings
        );
        
        // Optional: Add incandescent cooling effect (currently disabled)
        // This would make colors "cooler" (more blue) when dimmer
        // if (COOL_LIKE_INCANDESCENT) {
        //     coolLikeIncandescent(color, fastCycle8);
        // }
    } else {
        // Pixel is not lit this cycle
        color = CRGB::Black;
    }
    
    return color;
}

uint16_t TwinkleFoxEffect::update(WS2812FX* strip) {
    // Access runtime data
    auto runtime = strip->getSegmentRuntime();
    
    /**
     * Initialize the pseudorandom number generator (PRNG) with a fixed seed.
     * This MUST be reset to the same value each frame to ensure that each
     * LED's twinkling pattern remains stable and repeatable. This creates
     * the illusion that each LED has its own unique "personality."
     */
    uint16_t prng16 = 11337;  // Fixed seed for deterministic behavior
    
    // Get current time as the base for all animations
    uint32_t currentTime = millis();
    
    /**
     * Set up background color. In this implementation, we use black background
     * for maximum contrast. The original algorithm supports auto-selecting
     * background colors based on palette, but black works well for most cases.
     */
    CRGB backgroundColor = CRGB::Black;
    uint8_t backgroundBrightness = backgroundColor.getAverageLight();
    
    // Process each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        /**
         * Generate unique parameters for this LED using the PRNG.
         * Each LED gets different timing offsets and speed multipliers,
         * creating the organic, varied twinkling effect.
         */
        
        // Advance PRNG and use result as time offset for this pixel
        prng16 = (uint16_t)(prng16 * 2053) + 1384;
        uint16_t timeOffset = prng16;
        
        // Advance PRNG again for speed multiplier
        prng16 = (uint16_t)(prng16 * 2053) + 1384;
        
        // Calculate speed multiplier (0.5x to 1.4375x normal speed)
        // This gives each LED its own unique twinkling rate
        uint8_t speedMultiplierQ5_3 = ((((prng16 & 0xFF) >> 4) + (prng16 & 0x0F)) & 0x0F) + 0x08;
        
        // Compute adjusted time for this specific pixel
        uint32_t adjustedTime = (uint32_t)((currentTime * speedMultiplierQ5_3) >> 3) + timeOffset;
        
        // Extract salt value for this pixel (affects color and behavior)
        uint8_t pixelSalt = prng16 >> 8;
        
        // Compute the color for this pixel at this time
        CRGB pixelColor = computeOneTwinkle(&adjustedTime, &pixelSalt, strip);
        
        /**
         * Blend the computed color with the background based on brightness difference.
         * This creates smooth transitions and prevents harsh on/off behavior.
         */
        uint8_t pixelBrightness = pixelColor.getAverageLight();
        int16_t brightnessDelta = pixelBrightness - backgroundBrightness;
        
        uint16_t ledIndex = runtime->start + i;
        
        if (brightnessDelta >= 32 || (!backgroundColor)) {
            // Pixel is significantly brighter than background - use computed color
            strip->leds[ledIndex] = pixelColor;
        } else if (brightnessDelta > 0) {
            // Pixel is slightly brighter - blend with background
            // Note: Index bug was fixed in the calculation of ledIndex on line 147 (was using +SEG_RT.start instead of runtime->start + i)
            strip->leds[ledIndex] = blend(backgroundColor, pixelColor, brightnessDelta * 8);
        } else {
            // Pixel is not brighter than background - use background color
            strip->leds[ledIndex] = backgroundColor;
        }
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* TwinkleFoxEffect::getName() const {
    return F("Twinkle Fox");
}

uint8_t TwinkleFoxEffect::getModeId() const {
    return FX_MODE_TWINKLE_FOX;
}

// Register this effect with the factory system
REGISTER_EFFECT(FX_MODE_TWINKLE_FOX, TwinkleFoxEffect)