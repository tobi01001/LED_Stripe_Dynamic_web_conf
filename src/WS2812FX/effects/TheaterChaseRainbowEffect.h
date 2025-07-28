#ifndef THEATER_CHASE_RAINBOW_EFFECT_H
#define THEATER_CHASE_RAINBOW_EFFECT_H

#include "../Effect.h"

/**
 * @brief Theater Chase Rainbow Effect - Creates a theater chase pattern using rainbow colors
 * 
 * This effect creates a classic theater chase pattern where every third LED is lit,
 * but instead of using a solid color, it cycles through rainbow colors from the
 * current palette. The pattern moves along the strip creating a dynamic rainbow
 * chase effect.
 * 
 * The effect uses minimal internal state - just a counter that increments each
 * frame to create the color cycling and movement.
 */
class TheaterChaseRainbowEffect : public Effect {
private:
    /**
     * @brief Counter for color cycling through the rainbow palette
     * 
     * This counter increments each frame to select different colors from
     * the current palette, creating the rainbow cycling effect.
     * Masked to 8 bits to prevent overflow and cycle through full palette range.
     */
    uint8_t _colorCounter;
    

public:
    TheaterChaseRainbowEffect() : _colorCounter(0) {}
    virtual ~TheaterChaseRainbowEffect() = default;

    /**
     * @brief Initialize the theater chase rainbow effect
     * @param strip Pointer to the WS2812FX instance providing LED control
     * @return true if initialization successful
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the theater chase rainbow effect for one frame
     * @param strip Pointer to the WS2812FX instance providing LED control
     * @return Delay in milliseconds until next frame update
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the human-readable name of this effect
     * @return Flash string containing "Theater Chase Rainbow"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_THEATER_CHASE_RAINBOW constant
     */
    uint8_t getModeId() const override;
};

#endif // THEATER_CHASE_RAINBOW_EFFECT_H