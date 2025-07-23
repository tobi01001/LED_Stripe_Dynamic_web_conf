#ifndef COLOR_WIPE_SAWTOOTH_EFFECT_H
#define COLOR_WIPE_SAWTOOTH_EFFECT_H

#include "ColorWipeBaseEffect.h"

/**
 * @brief Color Wipe Sawtooth effect - creates sawtooth wave color wipe pattern
 * 
 * This effect creates a color wipe using a sawtooth wave pattern for position.
 * The sawtooth creates a linear ramp that resets sharply, creating a fast 
 * wipe in one direction followed by a quick reset.
 */
class ColorWipeSawtoothEffect : public ColorWipeBaseEffect {
public:
    ColorWipeSawtoothEffect() = default;
    virtual ~ColorWipeSawtoothEffect() = default;

    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

protected:
    uint16_t calculateWipePosition(WS2812FX* strip, uint32_t timebase) override;
};

#endif // COLOR_WIPE_SAWTOOTH_EFFECT_H