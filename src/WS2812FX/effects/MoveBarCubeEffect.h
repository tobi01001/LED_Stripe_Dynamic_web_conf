#ifndef MOVE_BAR_CUBE_EFFECT_H
#define MOVE_BAR_CUBE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Move Bar Cube effect - displays a moving bar using cubic wave motion
 * 
 * This effect creates a moving bar (1/2 of the strip length) that oscillates
 * using a cubic easing pattern. The bar provides even more dramatic acceleration
 * and deceleration compared to quadratic, creating a strong bouncing effect.
 * 
 * Key features:
 * - Bar width is half the strip length
 * - Movement follows EffectHelper::ease16InOutCubic(EffectHelper::triwave16()) for cubic easing
 * - Background fades based on speed setting
 * - Uses current palette for coloring
 * - Fractional positioning for smooth movement
 * - Most dramatic acceleration/deceleration of the move bar effects
 */
class MoveBarCubeEffect : public Effect {
public:
    MoveBarCubeEffect() = default;
    virtual ~MoveBarCubeEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Calculate the cubic wave position for the moving bar
     * @param speed The calculated speed value for the animation
     * @param width The width of the bar (half strip length)
     * @return 16-bit fractional position for smooth movement
     */
    uint16_t calculateCubicPosition(uint16_t speed, uint16_t width);
    
    
   
};

#endif // MOVE_BAR_CUBE_EFFECT_H