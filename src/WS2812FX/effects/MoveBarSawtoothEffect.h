#ifndef MOVE_BAR_SAWTOOTH_EFFECT_H
#define MOVE_BAR_SAWTOOTH_EFFECT_H

#include "../Effect.h"

/**
 * @brief Move Bar Sawtooth effect - displays a moving bar using sawtooth wave motion
 * 
 * This effect creates a moving bar (1/2 of the strip length) that oscillates
 * using a linear sawtooth pattern. The bar provides consistent speed movement
 * with sharp direction changes, creating a mechanical, linear motion effect.
 * 
 * Key features:
 * - Bar width is half the strip length
 * - Movement follows triwave16() for linear sawtooth motion
 * - Background fades based on speed setting
 * - Uses current palette for coloring
 * - Fractional positioning for smooth movement
 * - Linear movement with sharp direction changes
 */
class MoveBarSawtoothEffect : public Effect {
public:
    MoveBarSawtoothEffect() = default;
    virtual ~MoveBarSawtoothEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Calculate the sawtooth wave position for the moving bar
     * @param speed The calculated speed value for the animation
     * @param width The width of the bar (half strip length)
     * @return 16-bit fractional position for smooth movement
     */
    uint16_t calculateSawtoothPosition(uint16_t speed, uint16_t width);
    
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
};

#endif // MOVE_BAR_SAWTOOTH_EFFECT_H