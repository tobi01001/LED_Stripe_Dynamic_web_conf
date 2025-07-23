#include "RainbowCycleEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool RainbowCycleEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t RainbowCycleEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    // Ensure effect is properly initialized
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return minimum delay if init failed
        }
    }
    // Get segment and runtime data through the strip public interface
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate current position in rainbow cycle using helper
    uint16_t beatValue = EffectHelper::calculateBeatPosition(strip, timebase);
    uint8_t startingPaletteIndex = EffectHelper::safeMapuint16_t(beatValue, 0, 65535, 0, 255);
    
    // Calculate color spacing (delta) based on segment length and palette distribution
    uint8_t deltaHue = max(1, (256 * 100 / (runtime->length * seg->paletteDistribution)));
    
    // Fill the segment with rainbow colors using FastLED's fill_palette function
    fill_palette(&strip->leds[runtime->start],
                runtime->length,
                startingPaletteIndex,
                deltaHue,
                *strip->getCurrentPalette(),
                255,                            // Full internal brightness
                seg->blendType);
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* RainbowCycleEffect::getName() const {
    return F("Rainbow Cycle");
}

uint8_t RainbowCycleEffect::getModeId() const {
    return FX_MODE_RAINBOW_CYCLE;
}

void RainbowCycleEffect::cleanup() {
    // Reset state for clean transition to next effect
    initialized = false;
    timebase = 0;
}

// Register this effect with the factory system
// This enables automatic creation when FX_MODE_RAINBOW_CYCLE is requested
REGISTER_EFFECT(FX_MODE_RAINBOW_CYCLE, RainbowCycleEffect)