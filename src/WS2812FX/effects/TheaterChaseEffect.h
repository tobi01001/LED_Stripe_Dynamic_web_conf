#ifndef THEATER_CHASE_EFFECT_H
#define THEATER_CHASE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Theater Chase effect - creates classic three-pixel chase pattern
 * 
 * This effect simulates the classic theater marquee chase lights where every
 * third LED is illuminated in a pattern that moves along the strip. The pattern
 * continuously shifts, creating a flowing chase effect reminiscent of old 
 * theater marquee signs.
 * 
 * Key characteristics:
 * - Three-pixel repeating pattern (on-off-off or similar)
 * - Smooth movement using beat timing system
 * - Uses current palette for color variation along strip
 * - Non-illuminated pixels are completely black
 * - Minimal external resource dependency
 * 
 * Pattern behavior:
 * - Position 0: LEDs 0,3,6,9... are lit, others are black
 * - Position 1: LEDs 1,4,7,10... are lit, others are black  
 * - Position 2: LEDs 2,5,8,11... are lit, others are black
 * - Cycles through positions creating chase movement
 * 
 * Memory usage: ~16 bytes (timebase + initialization flag)
 */
class TheaterChaseEffect : public Effect {
public:
    TheaterChaseEffect() = default;
    virtual ~TheaterChaseEffect() = default;

    /**
     * @brief Initialize the theater chase effect
     * @param strip Pointer to the WS2812FX instance for accessing segment data
     * @return true if initialization was successful
     * 
     * @brief Update the theater chase effect for one frame
     * 
     * This method:
     * 1. Calculates current chase position using beat timing (0-2 cycle)
     * 2. Iterates through all LEDs in the segment
     * 3. For each LED, determines if it should be lit based on position modulo 3
     * 4. Sets lit LEDs to palette color, others to black
     * 
     * @param strip Pointer to the WS2812FX instance
     * @return Minimum delay (STRIP_MIN_DELAY) for smooth animation
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the display name of this effect
     * @return Flash string containing "Theater Chase"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_THEATER_CHASE from the MODES enum
     */
    uint8_t getModeId() const override;

private:
    // Effect-specific state variables (minimizes shared resource usage)    // Constants for effect behavior
    static const uint8_t CHASE_PATTERN_SIZE = 3;     ///< Number of positions in chase pattern
    static const uint8_t SPEED_DIVISOR = 1;          ///< Divides the base timing interval to slow down the chase animation; higher values result in slower movement by increasing the delay between position shifts, allowing for smoother visual transitions.
    static const uint8_t FULL_BRIGHTNESS = 255;      ///< Maximum brightness for lit pixels
    static const uint16_t BEAT_RANGE_MAX = 255;      ///< Maximum value for beat mapping
};

#endif // THEATER_CHASE_EFFECT_H