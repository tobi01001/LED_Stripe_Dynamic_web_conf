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
 * - Movement follows ease16InOutCubic(triwave16()) for cubic easing
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
    
    /**
     * @brief 16-bit triangle wave function for smooth back-and-forth motion
     * @param in Input value (0-65535)
     * @return Triangle wave output (0-65534, with smooth transitions)
     */
    static inline uint16_t triwave16(uint16_t in) {
        if (in & 0x8000) {
            in = 65535 - in;
        }
        return in << 1;
    }
    
    /**
     * @brief 16-bit cubic easing function for the most dramatic acceleration/deceleration
     * @param i Input value (0-65535)
     * @return Eased output with cubic curve
     */
    static inline uint16_t ease16InOutCubic(uint16_t i) {
        uint16_t ii = scale16(i, i);
        uint16_t iii = scale16(ii, i);
        uint32_t r1 = (3 * (uint16_t)(ii)) - (2 * (uint16_t)(iii));
        uint16_t result = r1;
        // if we got "65536", return 65535:
        if (r1 & 0x10000) {
            result = 65535;
        }
        return result;
    }
};

#endif // MOVE_BAR_CUBE_EFFECT_H