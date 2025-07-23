#ifndef COLOR_WIPE_QUAD_EFFECT_H
#define COLOR_WIPE_QUAD_EFFECT_H

#include "ColorWipeBaseEffect.h"

/**
 * @brief Color Wipe Quad effect - creates quadratic wave color wipe pattern
 * 
 * This effect creates a color wipe using a quadratic wave pattern for position.
 * The quadratic wave creates smooth ease-in/ease-out motion with more time
 * spent at the extremes and faster movement through the middle.
 */
class ColorWipeQuadEffect : public ColorWipeBaseEffect {
public:
    ColorWipeQuadEffect() = default;
    virtual ~ColorWipeQuadEffect() = default;

    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

protected:
    uint16_t calculateWipePosition(WS2812FX* strip, uint32_t timebase) override;
};

#endif // COLOR_WIPE_QUAD_EFFECT_H