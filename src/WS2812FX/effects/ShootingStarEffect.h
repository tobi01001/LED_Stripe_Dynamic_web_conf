#ifndef SHOOTING_STAR_EFFECT_H
#define SHOOTING_STAR_EFFECT_H

#include "../Effect.h"

// Maximum number of shooting stars that can be active simultaneously
#ifndef MAX_NUM_BARS
#define MAX_NUM_BARS 8
#endif

/**
 * @brief Shooting Star Effect - Creates multiple shooting star animations across the LED strip
 * 
 * This effect simulates shooting stars moving across the LED strip with realistic motion
 * and trailing effects. Multiple shooting stars can be active simultaneously, each with
 * different colors, speeds, and trajectories.
 * 
 * Key features:
 * - Multiple independent shooting stars (configurable via numBars setting)
 * - Quadratic acceleration for realistic motion physics
 * - Automatic color cycling with controlled randomization
 * - Trailing fade effects for realistic star trails
 * - Sparkle effects when stars reach the end of the strip
 * - Self-contained state management without external dependencies
 * 
 * Algorithm details:
 * 1. Each shooting star has its own beat offset for independent timing
 * 2. Position calculation uses quadratic function for acceleration
 * 3. Colors are distributed across the color wheel with controlled variation
 * 4. Trail effect is created using fade and blur operations
 * 5. Stars restart with new colors when they reach the end
 * 
 * The effect manages its own timing and state, using minimal external resources
 * and providing smooth, visually appealing shooting star animations.
 */
class ShootingStarEffect : public Effect {
public:
    ShootingStarEffect() = default;
    virtual ~ShootingStarEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Effect state variables - fully encapsulated within the class
    uint16_t basebeat;              ///< Base beat value for timing calculations
    uint16_t delta_b[MAX_NUM_BARS]; ///< Beat offset for each shooting star
    uint8_t cind[MAX_NUM_BARS];     ///< Color index for each shooting star
    bool new_cind[MAX_NUM_BARS];    ///< Flag indicating if star needs new color
    uint8_t numBars;                ///< Number of active shooting stars

    /**
     * @brief Initialize shooting star parameters
     * @param strip Pointer to WS2812FX instance for accessing settings
     */
    void initializeStars(WS2812FX* strip);

    /**
     * @brief Update individual shooting star position and appearance
     * @param strip Pointer to WS2812FX instance
     * @param starIndex Index of the star to update (0 to numBars-1)
     */
    void updateStar(WS2812FX* strip, uint8_t starIndex);

    /**
     * @brief Calculate shooting star position using quadratic acceleration
     * @param beat Current beat value for the star
     * @param strip Pointer to WS2812FX instance for strip boundaries
     * @return Position in 16-bit fixed point format (position * 16)
     */
    uint16_t calculateStarPosition(uint16_t beat, WS2812FX* strip) const;

    /**
     * @brief Generate new color for a shooting star
     * @param strip Pointer to WS2812FX instance
     * @param starIndex Index of the star to recolor
     */
    void generateNewColor(WS2812FX* strip, uint8_t starIndex);

    /**
     * @brief Apply fade and blur effects to create star trails
     * @param strip Pointer to WS2812FX instance
     */
    void applyTrailEffects(WS2812FX* strip);

    /**
     * @brief Handle star reaching the end of the strip
     * @param strip Pointer to WS2812FX instance
     * @param starIndex Index of the star that reached the end
     * @param position Current position of the star
     */
    void handleStarEnd(WS2812FX* strip, uint8_t starIndex, uint16_t position);
};

#endif // SHOOTING_STAR_EFFECT_H