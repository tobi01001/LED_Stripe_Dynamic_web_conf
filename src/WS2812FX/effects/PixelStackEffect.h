#ifndef PIXEL_STACK_EFFECT_H
#define PIXEL_STACK_EFFECT_H

#include "../Effect.h"

/**
 * @brief Pixel Stack Effect - Creates a dynamic stacking animation of LEDs
 * 
 * This effect creates a visually striking "stacking" animation where LEDs light up
 * progressively from the bottom half to the top half of the strip, then reverse
 * direction in a continuous cycle. The effect simulates pixels being "stacked"
 * and "unstacked" in a fluid motion.
 * 
 * Key features:
 * - Bidirectional stacking animation (up and down phases)
 * - Smooth fractional bar movement for precise positioning
 * - Color palette distribution based on LED position
 * - Self-contained state management without external shared resources
 * - Speed-dependent fade effects for visual smoothness
 * 
 * Algorithm details:
 * 1. Uses half the strip length as the active effect area
 * 2. Maintains separate tracking for LEDs already moved vs. currently moving
 * 3. Up phase: LEDs stack from bottom to middle, then to top
 * 4. Down phase: LEDs unstack from top back to original positions
 * 5. Uses beat88() for timing and smooth sine-wave-like movement patterns
 * 6. Applies speed-dependent background fading to create trails
 * 
 * The effect creates an organic, breathing-like pattern that's particularly
 * effective for ambient lighting and rhythmic displays.
 */
class PixelStackEffect : public Effect {
public:
    PixelStackEffect() = default;
    virtual ~PixelStackEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Effect state variables - fully encapsulated within the class
    bool up;                 ///< Direction flag: true = stacking up, false = unstacking down
    int16_t leds_moved;      ///< Number of LEDs that have been moved to their final position
    uint16_t ppos16;         ///< Previous position in 16-bit fixed point format for movement tracking

    // Constants for fractional positioning
    static const uint16_t FRACTIONAL_POSITION_UNIT = 16;  ///< Scaling factor for sub-pixel positioning
    static const uint16_t PIXEL_SCALING_FACTOR = 16;      ///< Scaling factor for pixel position calculations  
    static const uint8_t FRACTIONAL_BAR_WIDTH = 2;        ///< Width of the bar drawn in fractional units
    /**
     * @brief Initialize effect state variables to starting conditions
     * Sets up the initial direction, position counters, and movement tracking
     */
    void initializeEffectState();

    /**
     * @brief Apply background fade effect based on current speed
     * @param strip Pointer to WS2812FX instance for accessing speed settings
     * 
     * Creates smooth trails and prevents harsh transitions by fading
     * the entire strip by a speed-dependent amount each frame.
     */
    void applyBackgroundFade(WS2812FX* strip);

    /**
     * @brief Render static LEDs that are in their final stacked positions
     * @param strip Pointer to WS2812FX instance
     * @param nLeds Number of LEDs in the effect (half strip length)
     * @param baseHue Base hue value for color calculations
     * 
     * Draws the LEDs that have completed their movement and are now
     * part of the static "stack" in both lower and upper sections.
     */
    void renderStackedLEDs(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue);

    /**
     * @brief Handle the upward stacking phase of the animation
     * @param strip Pointer to WS2812FX instance
     * @param nLeds Number of LEDs in the effect
     * @param baseHue Base hue value for color calculations
     * @param beatPosition Current beat position (0-65535)
     * 
     * Manages LED movement from bottom to top, tracking when LEDs
     * reach their destination and updating the movement counters.
     */
    void handleUpwardMovement(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue, uint16_t beatPosition);

    /**
     * @brief Handle the downward unstacking phase of the animation
     * @param strip Pointer to WS2812FX instance
     * @param nLeds Number of LEDs in the effect
     * @param baseHue Base hue value for color calculations
     * @param beatPosition Current beat position (0-65535)
     * 
     * Manages LED movement from top back to bottom, tracking when LEDs
     * complete their return journey and updating the movement counters.
     */
    void handleDownwardMovement(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue, uint16_t beatPosition);

    /**
     * @brief Calculate the effective speed value for the current segment
     * @param strip Pointer to WS2812FX instance
     * @return Mapped speed value (0-65535) based on beat88 and segment count
     * 
     * Converts the beat88 timing value into an appropriate speed for this effect,
     * taking into account the number of segments to ensure consistent timing.
     */
    uint16_t calculateEffectSpeed(WS2812FX* strip) const;

    /**
     * @brief Get color for a specific position in the stack
     * @param strip Pointer to WS2812FX instance
     * @param position Position index in the stack (0 to nLeds-1)
     * @param nLeds Total number of LEDs in the effect
     * @param baseHue Base hue value for color calculations
     * @return CRGB color value for the specified position
     * 
     * Calculates the appropriate color from the current palette based on
     * the LED's position in the stack, creating a gradient effect.
     */
    CRGB getStackColor(WS2812FX* strip, uint16_t position, uint16_t nLeds, uint8_t baseHue) const;
};

#endif // PIXEL_STACK_EFFECT_H