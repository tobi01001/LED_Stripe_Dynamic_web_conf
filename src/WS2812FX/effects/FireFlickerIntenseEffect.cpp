#include "FireFlickerIntenseEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool FireFlickerIntenseEffect::init(WS2812FX* strip) {
    // Fire flicker effect is stateless - use minimal initialization
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    return true;
}

uint16_t FireFlickerIntenseEffect::update(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Maximum flicker intensity for the "intense" variant
    const uint8_t flickerIntensity = 255;  // Intense variant uses maximum variation
    
    // Process each LED in the segment individually
    for (uint16_t i = runtime->start; i <= runtime->stop; i++) {
        // Calculate color index for this LED position using helper
        uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, i - runtime->start, runtime->baseHue);
        
        // Set the base color from current palette
        strip->leds[i] = strip->ColorFromPaletteWithDistribution(*strip->getCurrentPalette(), 
                                                                colorIndex, 
                                                                seg->brightness, 
                                                                seg->blendType);
        
        // Apply random flicker effect with 2/3 probability for organic appearance
        if (random8(3)) {
            applyFlicker(strip->leds[i], flickerIntensity);
        }
    }
    
    return strip->getStripMinDelay();
}

void FireFlickerIntenseEffect::applyFlicker(CRGB& pixel, uint8_t flickerIntensity) {
    // Generate random flicker amount up to the specified intensity
    int flicker = random8(0, flickerIntensity);
    
    // Create a temporary color that will be darkened by random amounts
    CRGB temp = pixel;
    
    // Subtract random amounts from each color component independently
    // This creates more realistic fire-like color variations
    // Each component (R, G, B) can flicker independently for organic appearance
    temp -= CRGB(random8(flicker), random8(flicker), random8(flicker));
    
    // Blend the flickered color with the original using nblend
    // Blend factor of 96 provides smooth but noticeable flickering
    // This prevents harsh jumps while maintaining visible flicker effect
    nblend(pixel, temp, 96);
}

const __FlashStringHelper* FireFlickerIntenseEffect::getName() const {
    return F("Fire Flicker");
}

uint8_t FireFlickerIntenseEffect::getModeId() const {
    return FX_MODE_FIRE_FLICKER_INTENSE;
}

// Register this effect with the factory system
// This allows the effect to be created automatically when the mode is selected
REGISTER_EFFECT(FX_MODE_FIRE_FLICKER_INTENSE, FireFlickerIntenseEffect)