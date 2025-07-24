#ifndef BREATH_EFFECT_H
#define BREATH_EFFECT_H

#include "../Effect.h"

/**
 * @brief Breath effect - creates a smooth pulsing brightness across the entire strip
 * 
 * This effect fills the LED strip with colors from the current palette and modulates
 * the brightness using a sine wave to create a breathing or pulsing effect.
 * The speed is controlled by the beat88 setting, with faster speeds creating 
 * more rapid breathing patterns.
 */
class BreathEffect : public Effect {
public:
    BreathEffect() = default;
    virtual ~BreathEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;  ///< Time reference for consistent breathing animation
};

#endif // BREATH_EFFECT_H