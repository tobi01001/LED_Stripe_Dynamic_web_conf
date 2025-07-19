#ifndef RAINBOW_CYCLE_EFFECT_H
#define RAINBOW_CYCLE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Rainbow Cycle effect - creates a rainbow pattern that cycles across the LED strip
 * 
 * This effect creates a rainbow pattern distributed across the entire LED strip,
 * with the rainbow slowly cycling/moving along the strip over time. Unlike the
 * Rainbow effect where all LEDs show the same color, this effect shows a gradient
 * of colors across the strip, creating a more dynamic rainbow appearance.
 * 
 * Key characteristics:
 * - Rainbow colors distributed across the full length of the strip
 * - Pattern cycles/moves over time based on speed setting
 * - Uses palette distribution settings for color spacing
 * - Smooth color transitions using FastLED's fill_palette function
 * - Speed controlled by beat88 parameter for consistent timing
 * - Private state management to avoid shared resource conflicts
 */
class RainbowCycleEffect : public Effect {
public:
    RainbowCycleEffect() = default;
    virtual ~RainbowCycleEffect() = default;

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
    uint32_t timebase = 0;          ///< Time reference for beat calculations (replaces rainbow_cycle.timebase)
    bool initialized = false;       ///< Flag to track initialization state
};

#endif // RAINBOW_CYCLE_EFFECT_H