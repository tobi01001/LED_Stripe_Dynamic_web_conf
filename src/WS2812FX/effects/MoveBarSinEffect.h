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

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Calculate the sine wave position for the moving bar
     * @param speed The calculated speed value for the animation
     * @param width The width of the bar (half strip length)
     * @return 16-bit fractional position for smooth movement
     */
    uint16_t calculateSinePosition(uint16_t speed, uint16_t width);
    
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

#endif // MOVE_BAR_SIN_EFFECT_H