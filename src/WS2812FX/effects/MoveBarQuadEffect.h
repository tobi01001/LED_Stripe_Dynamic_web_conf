#ifndef MOVE_BAR_QUAD_EFFECT_H
#define MOVE_BAR_QUAD_EFFECT_H

#include "../Effect.h"

/**
 * @brief Move Bar Quad effect - displays a moving bar using quadratic wave motion
 * 
 * This effect creates a moving bar (1/2 of the strip length) that oscillates
 * using a quadratic easing pattern. The bar provides a more dramatic acceleration
 * and deceleration compared to the sine wave, creating a bouncing effect.
 * 
 * Key features:
 * - Bar width is half the strip length
 * - Movement follows ease16InOutQuad(triwave16()) for quadratic easing
 * - Background fades based on speed setting
 * - Uses current palette for coloring
 * - Fractional positioning for smooth movement
 * - More dramatic acceleration/deceleration than sine wave
 */
class MoveBarQuadEffect : public Effect {
public:
    MoveBarQuadEffect() = default;
    virtual ~MoveBarQuadEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Calculate the quadratic wave position for the moving bar
     * @param speed The calculated speed value for the animation
     * @param width The width of the bar (half strip length)
     * @return 16-bit fractional position for smooth movement
     */
    uint16_t calculateQuadPosition(uint16_t speed, uint16_t width);
    
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
     * @brief 16-bit quadratic easing function for dramatic acceleration/deceleration
     * @param i Input value (0-65535)
     * @return Eased output with quadratic curve
     */
    static inline uint16_t ease16InOutQuad(uint16_t i) {
        uint16_t j = i;
        if (j & 0x8000) {
            j = 65535 - j;
        }
        uint16_t jj = scale16(j, j);
        uint16_t jj2 = jj << 1;
        if (i & 0x8000) {
            jj2 = 65535 - jj2;
        }
        return jj2;
    }
};

#endif // MOVE_BAR_QUAD_EFFECT_H