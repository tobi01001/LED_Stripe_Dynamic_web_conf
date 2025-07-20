#ifndef MULTI_DYNAMIC_EFFECT_H
#define MULTI_DYNAMIC_EFFECT_H

#include "../Effect.h"

/**
 * @brief Multi Dynamic effect - displays random colors on all LEDs, changing simultaneously
 * 
 * This effect lights every LED in random colors and changes all LEDs at the same time
 * to new random colors. The timing of color changes is controlled by the speed parameter.
 * Colors are selected randomly from the current palette with distribution settings applied.
 * 
 * Key characteristics:
 * - All LEDs change color simultaneously
 * - Random color selection from current palette
 * - Timing controlled by beat88 speed parameter
 * - Uses palette distribution for color variety
 * - Minimizes shared state by maintaining private variables
 */
class MultiDynamicEffect : public Effect {
public:
    MultiDynamicEffect() = default;
    virtual ~MultiDynamicEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
    void cleanup() override;

private:
    /**
     * @brief Private state variables to avoid shared resources
     * These replace the union-based variables in the original implementation
     */
    uint32_t nextUpdate = 0;        ///< Timestamp when next color change should occur (replaces multi_dyn.last)
    uint8_t lastColorIndex = 0;     ///< Last palette index used for random generation (replaces multi_dyn.last_index)
    bool initialized = false;       ///< Flag to track initialization state
};

#endif // MULTI_DYNAMIC_EFFECT_H