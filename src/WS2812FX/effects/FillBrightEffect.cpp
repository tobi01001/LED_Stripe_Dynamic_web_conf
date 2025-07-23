#include "FillBrightEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool FillBrightEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from EffectHelper
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t FillBrightEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate hue movement speed with minimum value
    uint16_t hueSpeed = max((uint16_t)(seg->beat88 / HUE_SPEED_DIVISOR), 
                           (uint16_t)MIN_HUE_SPEED);
    
    // Calculate hue position using beat88 for continuous movement
    uint8_t huePosition = beat88(hueSpeed, timebase);
    
    // Calculate hue increment based on segment length and palette distribution
    // This creates appropriate color distribution across the strip
    uint16_t hueIncrement = max((uint16_t)(255 * 100 / (runtime->length * seg->paletteDistribution) + 1), 
                               (uint16_t)1);
    
    // Calculate brightness wave speed with minimum value
    uint16_t brightnessSpeed = max((uint16_t)(seg->beat88 / BRIGHTNESS_SPEED_DIVISOR), 
                                  (uint16_t)MIN_BRIGHTNESS_SPEED);
    
    // Calculate brightness using beatsin88 for smooth wave motion
    uint8_t brightness = beatsin88(brightnessSpeed, MIN_BRIGHTNESS, MAX_BRIGHTNESS, timebase);
    
    // Fill the entire strip with palette colors
    // Parameters:
    // - LED array starting at segment start
    // - Segment length
    // - Hue position (moving over time)
    // - Hue increment (for color distribution)
    // - Current palette
    // - Brightness (waving over time)
    // - Blend type for smooth transitions
    fill_palette(&strip->leds[runtime->start], 
                 runtime->length, 
                 huePosition,                  // Moving hue position
                 (uint8_t)hueIncrement,        // Hue increment for distribution
                 *strip->getCurrentPalette(), 
                 brightness,                   // Waving brightness
                 seg->blendType);
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FillBrightEffect::getName() const {
    return F("Wave Bright");
}

uint8_t FillBrightEffect::getModeId() const {
    return FX_MODE_FILL_BRIGHT;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FILL_BRIGHT, FillBrightEffect)