#include "ColorWipeTriwaveEffect.h"
#include "../WS2812FX_FastLed.h"

uint16_t ColorWipeTriwaveEffect::calculateWipePosition(WS2812FX* strip, uint32_t timebase) {
    auto seg = strip->getSegment();
    if (!seg) return 0;
    
    // Create triangular wave using triwave16 with beat88 (mode 1 in original)
    // The original code used mode 2 for triwave functionality. In this implementation, 
    // we ensure mode uniqueness by assigning a dedicated mode for this effect. This 
    // avoids conflicts and ensures consistent behavior.
    // Double the speed for more responsive movement
    uint16_t beatPosition = beat88((seg->beat88 * 2) % 65535, timebase);
    return strip->triwave16(beatPosition);
}

const __FlashStringHelper* ColorWipeTriwaveEffect::getName() const {
    return F("Wipe Triwave");
}

uint8_t ColorWipeTriwaveEffect::getModeId() const {
    return FX_MODE_COLOR_WIPE_TRIWAVE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_COLOR_WIPE_TRIWAVE, ColorWipeTriwaveEffect)