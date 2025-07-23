#include "ColorWipeSawtoothEffect.h"
#include "../WS2812FX_FastLed.h"

uint16_t ColorWipeSawtoothEffect::calculateWipePosition(WS2812FX* strip, uint32_t timebase) {
    auto seg = strip->getSegment();
    if (!seg) return 0;
    
    // Create sawtooth wave using beat88 directly (mode 3 in original)
    // Multiply by 4 for faster sawtooth pattern
    return beat88((seg->beat88 * 4) % 65535, timebase);
}

const __FlashStringHelper* ColorWipeSawtoothEffect::getName() const {
    return F("Wipe Sawtooth");
}

uint8_t ColorWipeSawtoothEffect::getModeId() const {
    return FX_MODE_COLOR_WIPE_SAWTOOTH;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_COLOR_WIPE_SAWTOOTH, ColorWipeSawtoothEffect)