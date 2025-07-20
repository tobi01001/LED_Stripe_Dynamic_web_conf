#include "EaseBarEffect.h"
#include "../WS2812FX_FastLed.h"

bool EaseBarEffect::init(WS2812FX* strip) {
    // Initialize internal state for ease bar effect
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    runtime->modeinit = false;
    
    // Calculate minimum bar size based on strip length
    state.minLeds = calculateMinLeds(runtime->length);
    
    // Calculate beat frequencies from speed setting
    calculateBeatFrequencies(seg->beat88, state.beatFreq1, state.beatFreq2);
    
    // Initialize timing and counters
    state.counter = 0;
    state.lastUpdate = millis();
    
    // Initialize phase offsets for varied animation patterns
    state.phaseOffset1 = 0;
    state.phaseOffset2 = 0;
    
    return true;
}

uint16_t EaseBarEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Generate primary and secondary beat patterns using calculated frequencies
    // These create the core timing for position and size animations
    uint16_t beat1 = beatsin16(state.beatFreq1, 0, BEAT88_MAX + BEAT88_MAX / 5);
    uint16_t beat2 = beatsin16(state.beatFreq2, 0, BEAT88_MAX + BEAT88_MAX / 6);
    
    // Update counter when primary beat cycles complete (creates phase shifts)
    if (beat1 == 0) {
        state.counter++;
    }
    
    // Calculate triangular wave offset from counter for smooth transitions
    uint8_t triangularOffset = triwave8(state.counter);
    
    // Calculate brightness modulation using beat differential
    // This creates pulsing effects synchronized with the animation
    uint8_t brightnessMod = 255 - beatsin8(max(state.beatFreq2 - state.beatFreq1, 1));
    
    // Calculate dynamic starting position for the bar
    // Combines primary beat with offset to create smooth, organic movement
    uint16_t startLed = beatsin16(state.beatFreq1 / 2, 
                                  runtime->start, 
                                  runtime->stop - state.minLeds, 
                                  0, 
                                  beat1 + triangularOffset);
    
    // Calculate dynamic bar size
    // Uses secondary beat to vary size independently of position
    uint16_t numLeds = beatsin16(state.beatFreq2 / 2, 
                                 state.minLeds, 
                                 runtime->length - (startLed - runtime->start), 
                                 0, 
                                 beat2);
    
    // Ensure bar doesn't exceed strip boundaries
    numLeds = min(numLeds, (uint16_t)(runtime->length - (startLed - runtime->start)));
    
    // Calculate palette increment for smooth color progression across the strip
    uint8_t paletteIncrement = calculatePaletteIncrement(runtime->length, seg->paletteDistribution);
    
    // Calculate starting palette index based on bar position and base hue
    // This creates color that flows with the bar movement
    uint8_t startPaletteIndex = map(startLed, runtime->start, runtime->stop, 
                                    (uint16_t)0, (uint16_t)255) + runtime->baseHue;
    
    // Apply fade-out effect to create trailing visuals
    // Fade amount provides smooth background dimming
    fadeToBlackBy(&strip->leds[runtime->start], runtime->length, 128);
    
    // Render the main bar using palette colors
    // fill_palette provides smooth color transitions across the bar length
    fill_palette(&strip->leds[startLed], numLeds, startPaletteIndex, paletteIncrement, 
                 *strip->getCurrentPalette(), 255, seg->blendType);
    
    // Apply brightness modulation across the entire strip
    // This creates the synchronized pulsing effect that varies with animation
    for (uint16_t i = runtime->start; i < runtime->stop; i++) {
        // Individual pixel brightness scales with global brightness modulation
        // The modulation varies per pixel to create depth and movement
        strip->leds[i].nscale8(beatsin8(max((state.beatFreq2 - state.beatFreq1) / 2, 1), 
                                        128, 255, 0, brightnessMod + i));
    }
    
    // Return minimum delay for smooth animation
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