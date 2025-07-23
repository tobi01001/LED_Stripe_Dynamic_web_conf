#include "PrideEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool PrideEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Initialize all internal state variables to clean defaults
    sPseudotime = 0;      // Reset accumulated pseudo-time
    sLastMillis = 0;      // Will be set properly on first update
    sHue16 = 0;           // Start with red hue position
    
    // Clear the mode initialization flag
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t PrideEffect::update(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    CRGB* leds = strip->leds;
    
    // Calculate dynamic parameters based on strip speed (beat88)
    uint8_t brightdepth = calculateBrightDepth(seg->beat88);
    uint16_t brightnessthetainc16 = calculateBrightnessInc(seg->beat88);
    uint8_t msmultiplier = calculateTimeMultiplier(seg->beat88);
    uint16_t hueinc16 = calculateHueInc(seg->beat88);
    
    // Initialize hue starting position for this frame
    uint16_t hue16 = sHue16;
    
    // Update timing - calculate milliseconds elapsed since last frame
    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis;
    sLastMillis = ms;
    
    // Update internal state based on elapsed time
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88((seg->beat88 / 5) * 2 + 1, 5, 9);
    
    // Initialize brightness wave position for this frame
    uint16_t brightnesstheta16 = sPseudotime;
    
    // Render each LED in the segment with calculated colors
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Advance hue for each LED position - creates horizontal color spread
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;  // Convert from 16-bit to 8-bit hue
        
        // Advance brightness wave position for each LED
        brightnesstheta16 += brightnessthetainc16;
        
        // Calculate brightness using sine wave for smooth transitions
        uint16_t b16 = sin16(brightnesstheta16) + 32768;  // Convert from ±32767 to 0-65535
        
        // Apply brightness depth and scaling - creates wave-like brightness variation
        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;  // Square for more contrast
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;   // Apply depth scaling
        bri8 += (255 - brightdepth);  // Add base brightness to prevent full black
        
        // Create final color using palette for consistency
        CRGB newcolor = strip->ColorFromPaletteWithDistribution(*strip->getCurrentPalette(), 
                                                                hue8, bri8, seg->blendType);
        
        // Apply color to LED position (with reverse indexing like original)
        uint16_t pixelnumber = runtime->stop - i;
        
        // Blend with existing color for smooth transitions
        nblend(leds[pixelnumber], newcolor, 64);
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* PrideEffect::getName() const {
    return F("Pride");
}

uint8_t PrideEffect::getModeId() const {
    return FX_MODE_PRIDE;
}

// Helper method implementations
uint8_t PrideEffect::calculateBrightDepth(uint16_t beat88) const {
    // Brightness depth controls how deep the brightness waves go
    // Higher beat88 = faster, more dramatic brightness variations
    return beatsin88(beat88 / 3 + 1, 96, 224);
}

uint16_t PrideEffect::calculateBrightnessInc(uint16_t beat88) const {
    // Controls the frequency of brightness waves across the strip
    // Higher values = more waves, more detailed brightness patterns
    return beatsin88(beat88 / 5 + 1, (25 * 256), (40 * 256));
}

uint8_t PrideEffect::calculateTimeMultiplier(uint16_t beat88) const {
    // Controls how fast the waves move over time
    // Higher values = faster wave movement
    return beatsin88(beat88 / 7 + 1, 23, 60);
}

uint16_t PrideEffect::calculateHueInc(uint16_t beat88) const {
    // Controls color transition speed across the strip
    // Higher values = more rainbow colors visible simultaneously
    return beatsin88(beat88 / 9 + 1, 1, 3000);
}

// Register this effect with the factory so it can be created by mode ID
REGISTER_EFFECT(FX_MODE_PRIDE, PrideEffect)