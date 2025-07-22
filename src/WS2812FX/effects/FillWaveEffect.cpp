#include "FillWaveEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool FillWaveEffect::init(WS2812FX* strip) {
    // Use standardized initialization pattern
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t FillWaveEffect::update(WS2812FX* strip) {
    // Validate strip pointer and ensure initialization
    if (!EffectHelper::validateStripPointer(strip) || !initialized) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate hue offset using beat position for smooth wave motion
    // Multiplying speed by 2 creates faster hue cycling for more dynamic waves
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, timebase, 2);
    uint8_t hueOffset = runtime->baseHue + map(beatPosition, 0, 65535, 0, 255);
    
    // Calculate palette distribution delta hue for color spacing
    uint8_t paletteDistribution = seg->paletteDistribution;
    uint8_t deltaHue = max(255 * 100 / (runtime->length * paletteDistribution) + 1, 1);
    
    // Calculate brightness using triangle wave for pulsing wave effect
    // Triangle wave creates smooth up-down brightness transitions
    uint8_t brightness = EffectHelper::generateTriangleWave(
        beatPosition, 
        seg->targetBrightness / 10,  // Minimum brightness (10% of target)
        255                          // Maximum brightness
    );
    
    // Fill strip with palette colors using calculated parameters
    EffectHelper::fillPaletteWithBrightness(strip, brightness, deltaHue);
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FillWaveEffect::getName() const {
    return F("FILL Wave");
}

uint8_t FillWaveEffect::getModeId() const {
    return FX_MODE_FILL_WAVE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FILL_WAVE, FillWaveEffect)