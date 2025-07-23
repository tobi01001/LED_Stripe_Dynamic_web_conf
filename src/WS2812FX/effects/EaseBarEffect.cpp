#include "EaseBarEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool EaseBarEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    runtime->modeinit = false;
    
    // Calculate minimum bar size based on strip length using helper
    state.minLeds = EffectHelper::calculateProportionalWidth(strip, 4, 10);  // 1/4 with min 10
    
    // Calculate beat frequencies from speed setting
    calculateBeatFrequencies(seg->beat88, state.beatFreq1, state.beatFreq2);
    
    // Initialize timing and counters
    state.counter = 0;
    state.lastUpdate = millis();
    
    // Initialize phase offsets for varied animation patterns
    state.phaseOffset1 = 0;
    state.phaseOffset2 = 0;
    initialized = true;  // Set initialized flag to true
    return true;
}

uint16_t EaseBarEffect::update(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    // Ensure effect is properly initialized
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return minimum delay if init failed
        }
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Generate primary and secondary beat patterns using calculated frequencies
    uint16_t beat1 = beatsin16(state.beatFreq1, 0, BEAT88_MAX + BEAT88_MAX / 5);
    uint16_t beat2 = beatsin16(state.beatFreq2, 0, BEAT88_MAX + BEAT88_MAX / 6);
    
    // Update counter when primary beat cycles complete (creates phase shifts)
    if (beat1 == 0) {
        state.counter++;
    }
    
    // Calculate triangular wave offset from counter for smooth transitions
    uint8_t triangularOffset = triwave8(state.counter);
    
    // Calculate brightness modulation using beat differential
    uint8_t brightnessMod = 255 - beatsin8(max(state.beatFreq2 - state.beatFreq1, 1));
    
    // Calculate dynamic starting position for the bar
    uint16_t startLed = beatsin16(state.beatFreq1 / 2, 
                                  runtime->start, 
                                  runtime->stop - state.minLeds, 
                                  0, 
                                  beat1 + triangularOffset);
    
    // Calculate dynamic bar size
    uint16_t numLeds = beatsin16(state.beatFreq2 / 2, 
                                 state.minLeds, 
                                 runtime->length - (startLed - runtime->start), 
                                 0, 
                                 beat2);
    
    // Ensure bar doesn't exceed strip boundaries
    numLeds = min(numLeds, (uint16_t)(runtime->length - (startLed - runtime->start)));
    
    // Apply fade-out effect using helper
    EffectHelper::applyFadeEffect(strip, 128);
    
    // Calculate palette increment and starting index using helper logic
    uint8_t paletteIncrement = calculatePaletteIncrement(runtime->length, seg->paletteDistribution);
    uint8_t startPaletteIndex = EffectHelper::safeMapuint16_t(startLed, runtime->start, runtime->stop, 0, 255) + runtime->baseHue;
    
    // Render the main bar using palette colors
    fill_palette(&strip->leds[startLed], numLeds, startPaletteIndex, paletteIncrement, 
                 *strip->getCurrentPalette(), 255, seg->blendType);
    
    // Apply brightness modulation across the entire strip
    for (uint16_t i = runtime->start; i < runtime->stop; i++) {
        strip->leds[i].nscale8(beatsin8(max((state.beatFreq2 - state.beatFreq1) / 2, 1), 
                                        128, 255, 0, brightnessMod + i));
    }
    
    return strip->getStripMinDelay();
}

uint16_t EaseBarEffect::calculateMinLeds(uint16_t stripLength) {
    // Calculate minimum bar size ensuring visibility on any strip length
    // Uses 1/4 of strip length with a reasonable minimum of 10 LEDs
    return max((uint16_t)(stripLength / 4), (uint16_t)10);
}

void EaseBarEffect::calculateBeatFrequencies(uint16_t speed, uint8_t& freq1, uint8_t& freq2) {
    // Convert speed parameter to beat frequencies
    // Map speed from BEAT88 range to reasonable frequency ranges for smooth animation
    // Primary frequency controls position, secondary controls size
    freq1 = map(speed, (uint16_t)BEAT88_MIN, (uint16_t)(BEAT88_MAX / 2), 
                (uint16_t)2, (uint16_t)63);
    freq2 = map(speed, (uint16_t)BEAT88_MIN, (uint16_t)(BEAT88_MAX / 2), 
                (uint16_t)3, (uint16_t)111);
}

uint8_t EaseBarEffect::calculatePaletteIncrement(uint16_t stripLength, uint8_t paletteDistribution) {
    // Calculate palette increment for smooth color distribution
    // Ensures good color variety across the strip length while respecting palette distribution setting
    return max(1, (256 * 100 / (stripLength * paletteDistribution)));
}

const __FlashStringHelper* EaseBarEffect::getName() const {
    return F("Ease Bar");
}

uint8_t EaseBarEffect::getModeId() const {
    return FX_MODE_EASE_BAR;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_EASE_BAR, EaseBarEffect)