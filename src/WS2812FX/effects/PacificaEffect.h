#ifndef PACIFICA_EFFECT_H
#define PACIFICA_EFFECT_H

#include "../Effect.h"

/**
 * @brief Pacifica effect - creates realistic ocean wave animations using specific color palettes
 * 
 * This effect simulates the movement and colors of ocean waves by:
 * - Layering multiple wave patterns with different speeds and scales
 * - Using specialized ocean-themed color palettes (blues, greens, teals)
 * - Creating dynamic wave layer interactions that vary over time
 * - Adding "whitecap" effects where wave patterns align
 * - Deepening colors to enhance the oceanic appearance
 * 
 * The effect uses four distinct wave layers, each with its own:
 * - Color palette (pacifica_palette_p1, p2, p3)
 * - Speed and timing characteristics
 * - Scale and amplitude parameters
 * - Phase relationships that create realistic wave interference patterns
 * 
 * Speed control affects all wave layers proportionally while maintaining
 * their relative timing relationships for natural-looking motion.
 * 
 * Based on: FastLED Pacifica example by Mark Kriegsman
 */
class PacificaEffect : public Effect {
public:
    PacificaEffect() = default;
    virtual ~PacificaEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Internal state structure for wave layer animation
     * 
     * Contains all timing and phase information needed for the four wave layers.
     * This eliminates dependencies on external shared state variables.
     */
    struct {
        uint16_t colorIndexStart1;  ///< Phase offset for wave layer 1
        uint16_t colorIndexStart2;  ///< Phase offset for wave layer 2
        uint16_t colorIndexStart3;  ///< Phase offset for wave layer 3
        uint16_t colorIndexStart4;  ///< Phase offset for wave layer 4
        uint32_t lastMillis;        ///< Timestamp of last update for delta calculations
        uint16_t effectiveSpeed;    ///< Calculated effective speed for wave timing
    } state;

    /**
     * @brief Calculate effective speed from beat88 parameter
     * @param beat88 Speed parameter from segment configuration
     * @return Effective speed value ensuring minimum rate
     */
    uint16_t calculateEffectiveSpeed(uint16_t beat88);
};

#endif // PACIFICA_EFFECT_H