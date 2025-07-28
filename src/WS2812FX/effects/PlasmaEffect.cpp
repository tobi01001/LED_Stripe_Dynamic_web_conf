#include "PlasmaEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t PlasmaEffect::update(WS2812FX* strip) {
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
    
    // Calculate phase values for the two wave components
    // Primary phase - standard beat-based oscillation
    uint8_t primaryPhase = beatsin88(seg->beat88, 0, 255, _timebase);
    
    // Secondary phase - slightly faster oscillation (11/10 * beat88) for complex interaction
    uint8_t secondaryPhase = beatsin88((seg->beat88 * 11) / 10, 0, 255, _timebase);
    
    // Additional phase for brightness modulation - even faster (12/10 * beat88)
    uint8_t brightnessModulator = beatsin88((seg->beat88 * 12) / 10, 0, 128, _timebase);

    // Process each LED in the segment
    for (int k = runtime->start; k < runtime->stop; k++) {
        // Calculate position within the segment for wave calculations
        int relativePosition = k - runtime->start;
        
        // Create complex color index by combining two wave functions:
        // 1. Cubic wave with position-dependent phase shift
        uint8_t cubicWaveValue = cubicwave8((relativePosition * CUBIC_WAVE_FREQUENCY) + primaryPhase) / 2;
        
        // 2. Cosine wave with different frequency and secondary phase  
        uint8_t cosWaveValue = cos8((relativePosition * COS_WAVE_FREQUENCY) + secondaryPhase) / 2;
        
        // Combine waves and add base hue for color cycling
        uint8_t colorIndex = cubicWaveValue + cosWaveValue + runtime->baseHue;
        
        // Calculate brightness with dynamic modulation
        // Use qsuba to create dark spaces when brightness modulator is high
        // This creates the characteristic "plasma" holes and valleys
        uint8_t brightness = qsuba(colorIndex, brightnessModulator);
        
        // Get color from palette with calculated index and modulated brightness
        CRGB newColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            colorIndex, 
            brightness, 
            seg->blendType);
        
        // Blend new color with existing LED color for smooth transitions
        // Using fixed blend amount for consistent plasma flow
        strip->leds[k] = nblend(strip->leds[k], newColor, BLEND_AMOUNT);
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* PlasmaEffect::getName() const {
    return F("Plasma");
}

uint8_t PlasmaEffect::getModeId() const {
    return FX_MODE_PLASMA;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_PLASMA, PlasmaEffect)