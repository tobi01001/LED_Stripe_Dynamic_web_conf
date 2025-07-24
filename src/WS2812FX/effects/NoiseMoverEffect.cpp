#include "NoiseMoverEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool NoiseMoverEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize noise distance parameter for unique starting position
    noiseDist = 1234;
    
    return true;
}

uint16_t NoiseMoverEffect::update(WS2812FX* strip) {
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
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate noise-based position
    // Use segment length as X-scale for noise to create position-dependent variation
    uint16_t xScale = runtime->length;
    
    // Generate noise value for smooth, organic movement
    uint8_t noiseValue = inoise8(xScale, noiseDist + NOISE_Y_SCALE);
    
    // Map noise value to pixel position in 16-bit fractional format for sub-pixel accuracy
    // This creates smooth movement across the strip with the bar width accounted for
    uint16_t pixelPosition = map((uint16_t)noiseValue, 
                                (uint16_t)0, (uint16_t)255,
                                (uint16_t)(runtime->start * 16), 
                                (uint16_t)(runtime->stop * 16 - BAR_WIDTH * 16));
    
    // Calculate color index based on base hue
    // This could be enhanced with additional color movement, but keeping simple for robustness
    uint8_t colorIndex = runtime->baseHue;
    
    // Apply fade effect to create trailing pattern
    // Use EffectHelper's fade function for consistent behavior
    EffectHelper::applyFadeEffect(strip, FADE_AMOUNT);
    
    // Draw the moving bar using fractional positioning
    // Parameters:
    // - pixelPosition: 16-bit fractional position for smooth movement
    // - BAR_WIDTH: width of the bar in pixels
    // - palette: current palette for colors
    // - colorIndex: color based on position and base hue
    // - brightness: full brightness (255)
    // - mixColor: true for color mixing
    // - increment: color increment (1 for gradual change)
    uint8_t finalColorIndex = (uint8_t)((uint8_t)(pixelPosition / 64) + colorIndex);
    strip->drawFractionalBar(pixelPosition, BAR_WIDTH, *strip->getCurrentPalette(), 
                           finalColorIndex, 255, true, 1);
    
    // Update noise distance for next frame using beat-based movement
    // This creates variable speed movement that responds to the beat88 setting
    uint16_t distanceIncrement = beatsin88(seg->beat88, 1, 12, timebase);
    noiseDist += distanceIncrement;
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* NoiseMoverEffect::getName() const {
    return F("iNoise8");
}

uint8_t NoiseMoverEffect::getModeId() const {
    return FX_MODE_NOISEMOVER;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_NOISEMOVER, NoiseMoverEffect)