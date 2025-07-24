#include "FillBeatEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

uint16_t FillBeatEffect::update(WS2812FX* strip) {
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
    
    // Calculate base parameters for the effect
    uint8_t reducedBeat = seg->beat88 >> 8; // Reduce beat for slower color changes
    
    // Process each LED in the segment
    for (uint16_t k = runtime->start; k < runtime->stop; k++) {
        // Calculate position-dependent brightness using beatsin88
        // Each LED has a phase offset (k * 2) to create a wave across the strip
        uint8_t brightness = beatsin88(seg->beat88, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 
                                     millis(), (k - runtime->start) * 2);
        
        // Calculate complex color index using multiple wave components:
        
        // 1. Base triangular wave from beat timing
        uint8_t baseTriWave = triwave8(beat8(reducedBeat));
        
        // 2. Additional oscillation with beatsin8 for variation
        uint8_t oscillation = beatsin8(reducedBeat, 0, 20);
        
        // 3. Position-dependent component mapped to LED position
        uint8_t positionComponent = map((uint16_t)k, 
                                       (uint16_t)runtime->start, 
                                       (uint16_t)runtime->stop, 
                                       (uint16_t)0, 
                                       (uint16_t)255);
        
        // Combine all components for final color index
        uint8_t colorIndex = baseTriWave + oscillation + positionComponent;
        
        // Get color from palette with calculated index and brightness
        CRGB newColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            colorIndex, 
            brightness, 
            seg->blendType);
        
        // Blend new color with existing LED color
        // Use speed-dependent blend amount for responsive transitions
        uint8_t blendAmount = qadd8(reducedBeat, BASE_BLEND_AMOUNT);
        strip->leds[k] = nblend(strip->leds[k], newColor, blendAmount);
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FillBeatEffect::getName() const {
    return F("Color Fill");
}

uint8_t FillBeatEffect::getModeId() const {
    return FX_MODE_FILL_BEAT;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FILL_BEAT, FillBeatEffect)