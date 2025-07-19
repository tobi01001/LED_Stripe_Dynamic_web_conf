#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include "../Effect.h"

/**
 * @brief Rainbow effect - cycles all LEDs through rainbow colors as one solid color
 * 
 * This effect fills the entire LED strip with a single color that cycles through
 * the rainbow spectrum over time. All LEDs display the same color simultaneously,
 * creating a smooth, uniform color transition across the entire strip.
 * 
 * Key characteristics:
 * - All LEDs show the same color at any given time
 * - Smooth cycling through rainbow colors using current palette
 * - Speed controlled by beat88 parameter
 * - Uses palette-based color generation for consistent appearance
 * - Maintains private timing state to avoid shared resource conflicts
 */
class RainbowEffect : public Effect {
public:
    RainbowEffect() = default;
    virtual ~RainbowEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
    void cleanup() override;

private:
    /**
     * @brief Private state variables to minimize shared resource usage
     * These replace the union-based variables in the original implementation
     */
    uint32_t timebase = 0;          ///< Time reference for beat calculations (replaces rainbow.timebase)
    bool initialized = false;       ///< Flag to track initialization state
};

#endif // RAINBOW_EFFECT_H