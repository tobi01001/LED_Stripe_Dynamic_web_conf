#ifndef COLOR_WIPE_TRIWAVE_EFFECT_H
#define COLOR_WIPE_TRIWAVE_EFFECT_H

#include "ColorWipeBaseEffect.h"

/**
 * @brief Color Wipe Triwave effect - creates triangular wave color wipe pattern
 * 
 * This effect creates a color wipe using a triangular wave pattern for position.
 * The triangular wave creates linear motion in both directions with sharp
 * reversals at the extremes, similar to sawtooth but bidirectional.
 */
class ColorWipeTriwaveEffect : public ColorWipeBaseEffect {
public:
    ColorWipeTriwaveEffect() = default;
    virtual ~ColorWipeTriwaveEffect() = default;

    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

protected:
    uint16_t calculateWipePosition(WS2812FX* strip, uint32_t timebase) override;
};

#endif // COLOR_WIPE_TRIWAVE_EFFECT_H