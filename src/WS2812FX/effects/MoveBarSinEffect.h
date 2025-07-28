#ifndef MOVE_BAR_SIN_EFFECT_H
#define MOVE_BAR_SIN_EFFECT_H

#include "../Effect.h"

/**
 * @brief Move Bar Sine effect - displays a moving bar using sine wave motion
 * 
 * This effect creates a moving bar (1/2 of the strip length) that oscillates
 * using a sine wave pattern. The bar fades the background and draws a colored
 * bar that moves smoothly across the strip following a sinusoidal path.
 * 
 * Key features:
 * - Bar width is half the strip length
 * - Movement follows beatsin16() for smooth sine wave motion
 * - Background fades based on speed setting
 * - Uses current palette for coloring
 * - Fractional positioning for smooth movement
 */
class MoveBarSinEffect : public Effect {
public:
    MoveBarSinEffect() = default;
    virtual ~MoveBarSinEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:


};

#endif // MOVE_BAR_SIN_EFFECT_H