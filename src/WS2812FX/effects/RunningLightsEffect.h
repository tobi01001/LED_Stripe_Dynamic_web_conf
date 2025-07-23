#ifndef RUNNING_LIGHTS_EFFECT_H
#define RUNNING_LIGHTS_EFFECT_H

#include "../Effect.h"

/**
 * @brief Running Lights effect - creates a wave of light that travels along the strip
 * 
 * This effect simulates a wave of light moving across the LED strip. Each LED has
 * a sine wave-based brightness that creates a smooth traveling wave pattern.
 * The wave moves continuously and uses palette colors with smooth blending.
 * The speed is controlled by the beat88 setting.
 */
class RunningLightsEffect : public Effect {
public:
    RunningLightsEffect() = default;
    virtual ~RunningLightsEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;  ///< Time reference for consistent wave movement
    bool initialized = false;  ///< Initialization flag to ensure proper setup
};

#endif // RUNNING_LIGHTS_EFFECT_H