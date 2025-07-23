#include "ColorWipeSineEffect.h"
#include "../WS2812FX_FastLed.h"

uint16_t ColorWipeSineEffect::calculateWipePosition(WS2812FX* strip, uint32_t timebase) {
    auto seg = strip->getSegment();
    if (!seg) return 0;
    
    // Create sine wave using beatsin16 (mode 0 in original)
    // Double the speed for more responsive movement
    return beatsin16((seg->beat88 * 2) % 65535, 0, 65535, timebase);
}

const __FlashStringHelper* ColorWipeSineEffect::getName() const {
    return F("Wipe Sine");
}

uint8_t ColorWipeSineEffect::getModeId() const {
    return FX_MODE_COLOR_WIPE_SINE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_COLOR_WIPE_SINE, ColorWipeSineEffect)