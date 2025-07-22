#include "ColorWipeQuadEffect.h"
#include "../WS2812FX_FastLed.h"

uint16_t ColorWipeQuadEffect::calculateWipePosition(WS2812FX* strip, uint32_t timebase) {
    auto seg = strip->getSegment();
    if (!seg) return 0;
    
    // Create quadratic wave using quadwave16 with beat88 (mode 2 in original)
    // Double the speed for more responsive movement  
    uint16_t beatPosition = beat88((seg->beat88 * 2) % 65535, timebase);
    return EffectHelper::quadwave16(beatPosition);
}

const __FlashStringHelper* ColorWipeQuadEffect::getName() const {
    return F("Wipe Quad");
}

uint8_t ColorWipeQuadEffect::getModeId() const {
    return FX_MODE_COLOR_WIPE_QUAD;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_COLOR_WIPE_QUAD, ColorWipeQuadEffect)