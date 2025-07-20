#ifndef BEATSIN_GLOW_EFFECT_H
#define BEATSIN_GLOW_EFFECT_H

#include "../Effect.h"

// Maximum number of glow bars that can be active simultaneously
#ifndef MAX_NUM_BARS
#define MAX_NUM_BARS 8
#endif

/**
 * @brief Beatsin Glow Effect - Creates multiple sine wave-based glowing animations
 * 
 * This effect generates multiple glowing elements that move in sine wave patterns
 * across the LED strip. Each glow element has its own frequency, phase offset,
 * color, and timing parameters, creating a complex and visually appealing
 * synchronized light show.
 * 
 * Key features:
 * - Multiple independent sine wave oscillators (configurable via numBars)
 * - Dynamic frequency modulation that changes over time
 * - Automatic color cycling with controlled variation
 * - Phase relationships between oscillators for complex patterns
 * - Smooth transitions and position calculations using 16-bit precision
 * - Self-contained state management without external dependencies
 * 
 * Algorithm details:
 * 1. Each glow element has its own beat frequency with variation from base speed
 * 2. Phase offsets distribute elements across the sine wave cycle
 * 3. Color indices are distributed across the color wheel with variation
 * 4. Position calculation uses sine function with phase and time offsets
 * 5. Frequency and color modulation occurs at sine wave zero crossings
 * 6. Fade effects create smooth background transitions
 * 
 * The effect creates organic, breathing-like movements with complex color
 * interactions as multiple sine waves interfere with each other.
 */
class BeatsinGlowEffect : public Effect {
public:
    BeatsinGlowEffect() = default;
    virtual ~BeatsinGlowEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Effect state variables - fully encapsulated within the class
    uint16_t beats[MAX_NUM_BARS];    ///< Beat frequency for each glow element
    uint16_t theta[MAX_NUM_BARS];    ///< Phase offset for each glow element
    int16_t prev[MAX_NUM_BARS];      ///< Previous sine value for zero-crossing detection
    uint32_t times[MAX_NUM_BARS];    ///< Time offset for each glow element
    uint8_t cinds[MAX_NUM_BARS];     ///< Color index for each glow element
    bool newval[MAX_NUM_BARS];       ///< Flag indicating if element has new value
    uint8_t numBars;                 ///< Number of active glow elements

    /**
     * @brief Initialize all glow elements with random parameters
     * @param strip Pointer to WS2812FX instance for accessing settings
     */
    void initializeGlowElements(WS2812FX* strip);

    /**
     * @brief Update individual glow element position and appearance
     * @param strip Pointer to WS2812FX instance
     * @param elementIndex Index of the element to update (0 to numBars-1)
     */
    void updateGlowElement(WS2812FX* strip, uint8_t elementIndex);

    /**
     * @brief Calculate glow element position using sine wave
     * @param elementIndex Index of the element
     * @param strip Pointer to WS2812FX instance for strip boundaries
     * @return Position in 16-bit fixed point format (position * 16)
     */
    uint16_t calculateElementPosition(uint8_t elementIndex, WS2812FX* strip);

    /**
     * @brief Update element parameters when sine wave crosses zero
     * @param strip Pointer to WS2812FX instance
     * @param elementIndex Index of the element to update
     */
    void updateElementParameters(WS2812FX* strip, uint8_t elementIndex);

    /**
     * @brief Apply background fade effect
     * @param strip Pointer to WS2812FX instance
     */
    void applyBackgroundFade(WS2812FX* strip);

    /**
     * @brief Calculate variation limit based on current speed
     * @param strip Pointer to WS2812FX instance
     * @return Variation limit for parameter changes
     */
    uint16_t calculateVariationLimit(WS2812FX* strip) const;
};

#endif // BEATSIN_GLOW_EFFECT_H