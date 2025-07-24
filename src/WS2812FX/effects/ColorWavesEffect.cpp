#include "ColorWavesEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool ColorWavesEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    auto seg = strip->getSegment();
    
    // Initialize hue and timing state
    state.hue16 = 0;
    state.pseudotime = 0;
    state.lastMillis = millis() - strip->getStripMinDelay();
    
    // Calculate initial wave parameters based on current speed setting
    uint8_t brightDepth;
    uint16_t brightThetaInc;
    uint8_t msMultiplier;
    uint16_t hueInc;
    
    calculateWaveParameters(seg->beat88, brightDepth, brightThetaInc, 
                           msMultiplier, hueInc);
    
    // Store calculated parameters in state
    state.brightnessDepth = brightDepth;
    state.multiplier = msMultiplier;
    state.hueIncrement = hueInc;
    state.brightnessTheta = state.pseudotime;
    
    return true;
}

uint16_t ColorWavesEffect::update(WS2812FX* strip) {
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
    
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate time delta for consistent animation timing
    uint32_t currentMillis = millis();
    uint16_t deltaMillis = currentMillis - state.lastMillis;
    state.lastMillis = currentMillis;
    
    // Calculate current wave parameters based on speed setting
    uint8_t brightDepth;
    uint16_t brightThetaInc;
    uint8_t msMultiplier;
    uint16_t hueInc;
    
    calculateWaveParameters(seg->beat88, brightDepth, brightThetaInc, 
                           msMultiplier, hueInc);
    
    // Update internal timing accumulators
    state.pseudotime += deltaMillis * msMultiplier;
    state.hue16 += deltaMillis * beatsin88(max((seg->beat88 * 4) / 10, 8), 5, 9);
    
    // Initialize brightness theta for this frame
    uint16_t brightnessTheta = state.pseudotime;
    
    // Render color waves across the strip
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate hue for this pixel position
        uint16_t pixelHue16 = state.hue16 + (i * hueInc);
        
        // Apply sine wave shaping to hue for more organic color transitions
        uint8_t shapedHue = shapeHue(pixelHue16);
        
        // Calculate brightness using sine wave modulation
        uint8_t brightness = calculateBrightness(brightnessTheta, brightDepth);
        
        // Convert shaped hue to palette index with scaling
        uint8_t paletteIndex = scale8(shapedHue, 240);
        
        // Get color from current palette using calculated index and brightness
        CRGB newColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            paletteIndex, 
            brightness, 
            seg->blendType
        );
        
        // Calculate pixel position (reverse order for wave direction)
        uint16_t pixelNumber = (runtime->length - 1) - i;
        
        // Blend new color with existing pixel color for smooth transitions using helper
        nblend(strip->leds[runtime->start + pixelNumber], newColor, 128);
        
        // Advance brightness theta for next pixel
        brightnessTheta += brightThetaInc;
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

void ColorWavesEffect::calculateWaveParameters(uint16_t speed, 
                                               uint8_t& brightDepth,
                                               uint16_t& brightThetaInc,
                                               uint8_t& msMultiplier,
                                               uint16_t& hueInc) {
    // Calculate brightness modulation depth based on speed
    // Creates breathing/pulsing effect that scales with animation speed
    brightDepth = beatsin88(max(speed / 3 + 5, 7), 96, 224);
    
    // Calculate brightness wave increment (frequency)
    // Controls how quickly brightness waves move across the strip
    brightThetaInc = beatsin88(max(speed / 4 + 2, 5), (25 * 256), (40 * 256));
    
    // Calculate time multiplier for overall animation speed
    // Affects how quickly the waves progress over time
    msMultiplier = beatsin88(max(speed / 6 - 9, 3), 23, 60);
    
    // Calculate hue increment for color progression along strip
    hueInc = calculateHueIncrement(speed);
}

uint16_t ColorWavesEffect::calculateHueIncrement(uint16_t speed) {
    // Calculate hue increment based on speed for smooth color flow
    // Higher speeds create faster color transitions
    return beatsin88(max(speed / 8 - 12, 2), 300, 1500);
}

uint8_t ColorWavesEffect::shapeHue(uint16_t hue16) {
    // Convert 16-bit hue to 8-bit with sine wave shaping
    // This creates more organic color transitions instead of linear progression
    
    uint8_t baseHue = hue16 / 256;
    uint16_t hue16_128 = hue16 >> 7;
    
    // Apply sine wave shaping to create non-linear hue progression
    if (hue16_128 & 0x100) {
        // In the second half of the cycle, invert the progression
        return 255 - (hue16_128 >> 1);
    } else {
        // In the first half, use normal progression
        return hue16_128 >> 1;
    }
}

uint8_t ColorWavesEffect::calculateBrightness(uint16_t theta, uint8_t brightDepth) {
    // Calculate brightness using sine wave modulation
    // Provides smooth brightness waves that flow across the strip
    
    // Generate sine wave value (-32768 to +32767) and shift to positive range
    uint16_t sineValue = sin16(theta) + 32768;
    
    // Square the sine value for more pronounced brightness variations
    uint16_t squaredSine = (uint32_t)((uint32_t)sineValue * (uint32_t)sineValue) / 65536;
    
    // Apply brightness depth modulation
    uint8_t modulatedBrightness = (uint32_t)(((uint32_t)squaredSine) * brightDepth) / 65536;
    
    // Add base brightness to ensure minimum visibility
    return modulatedBrightness + (255 - brightDepth);
}

const __FlashStringHelper* ColorWavesEffect::getName() const {
    return F("Color Waves");
}

uint8_t ColorWavesEffect::getModeId() const {
    return FX_MODE_COLOR_WAVES;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_COLOR_WAVES, ColorWavesEffect)