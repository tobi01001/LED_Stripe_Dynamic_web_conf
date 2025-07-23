#ifndef COLOR_WIPE_SINE_EFFECT_H
#define COLOR_WIPE_SINE_EFFECT_H

#include "ColorWipeBaseEffect.h"

/**
 * @brief Color Wipe Sine effect - creates sine wave color wipe pattern
 * 
 * This effect creates a color wipe using a sine wave pattern for position.
 * The sine wave creates smooth back-and-forth motion with gradual acceleration
 * and deceleration at the ends.
 */
class ColorWipeSineEffect : public ColorWipeBaseEffect {
public:
    ColorWipeSineEffect() = default;
    virtual ~ColorWipeSineEffect() = default;

    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

protected:
    uint16_t calculateWipePosition(WS2812FX* strip, uint32_t timebase) override;
};

#endif // COLOR_WIPE_SINE_EFFECT_H