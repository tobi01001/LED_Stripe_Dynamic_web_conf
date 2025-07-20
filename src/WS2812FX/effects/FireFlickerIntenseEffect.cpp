#include "FireFlickerIntenseEffect.h"
#include "../WS2812FX_FastLed.h"

bool FireFlickerIntenseEffect::init(WS2812FX* strip) {
    // Fire flicker effect needs no special initialization as it's stateless
    // Each frame is generated independently using random values
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t FireFlickerIntenseEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip's public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the maximum flicker intensity for the "intense" variant
    // Lower values = more intense flicker (255/1 = 255, maximum variation)
    const uint8_t flickerIntensity = 255 / 1;  // Intense variant uses divisor of 1
    
    // Process each LED in the segment individually
    for (uint16_t i = runtime->start; i <= runtime->stop; i++) {
        // Calculate color index for this LED position
        // This distributes the palette colors evenly across the strip length
        // and adds the base hue offset for color cycling capability
        uint8_t colorIndex = map(i, runtime->start, runtime->stop, 
                                (uint16_t)0, (uint16_t)255) + runtime->baseHue;
        
        // Set the base color from the current palette using distribution-aware function
        // This respects the palette distribution setting for consistent color theming
        strip->leds[i] = strip->ColorFromPaletteWithDistribution(*strip->getCurrentPalette(), 
                                                                colorIndex, 
                                                                seg->brightness, 
                                                                seg->blendType);
        
        // Apply random flicker effect with 2/3 probability (random8(3) returns 0,1,2)
        // This creates organic, non-uniform flickering that looks more realistic
        if (random8(3)) {
            applyFlicker(strip->leds[i], flickerIntensity);
        }
    }
    
    // Return minimum delay for smooth, rapid flickering animation
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