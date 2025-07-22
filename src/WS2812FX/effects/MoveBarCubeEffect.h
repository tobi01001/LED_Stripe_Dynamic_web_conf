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

    bool init(WS2812FX* strip) override;
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
    
    /**
     * @brief Apply background fade based on speed
     * @param strip Pointer to the WS2812FX instance
     * @param speed The calculated speed value
     */
    void applyBackgroundFade(WS2812FX* strip, uint16_t speed);
    
    /**
     * @brief Draw the moving bar at the specified position
     * @param strip Pointer to the WS2812FX instance
     * @param position 16-bit fractional position of the bar
     * @param width Width of the bar to draw
     */
    void drawMovingBar(WS2812FX* strip, uint16_t position, uint16_t width);

private:
    // Internal timebase for beat calculations
    uint32_t timebase;
    
    // Initialization tracking
    bool initialized = false;
};

#endif // MOVE_BAR_CUBE_EFFECT_H