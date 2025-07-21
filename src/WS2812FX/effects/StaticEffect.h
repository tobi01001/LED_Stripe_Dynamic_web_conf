#ifndef STATIC_EFFECT_H
#define STATIC_EFFECT_H

#include "../Effect.h"

/**
 * @brief Static effect - displays a solid color using the current palette
 * 
 * This effect fills the entire LED strip with colors from the current palette,
 * distributed evenly across the display length.
 */
class StaticEffect : public Effect {
public:
    StaticEffect() = default;
    virtual ~StaticEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0; ///< Time reference (required by helper, though not used for static effect)
};

#endif // STATIC_EFFECT_H