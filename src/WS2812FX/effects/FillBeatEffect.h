#ifndef FILL_BEAT_EFFECT_H
#define FILL_BEAT_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fill Beat effect - creates waving color and brightness patterns
 * 
 * This effect fills the entire strip with colors that vary both in hue and
 * brightness based on complex wave functions. Each LED has a position-dependent
 * brightness that pulses with the beat, and the color index is calculated
 * using triangular waves for smooth color transitions across the strip.
 */
class FillBeatEffect : public Effect {
public:
    FillBeatEffect() = default;
    virtual ~FillBeatEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;  ///< Time reference for beat calculations
    bool initialized = false;  ///< Initialization flag to ensure proper setup
    
    // Constants for brightness and blending
    static const uint8_t MIN_BRIGHTNESS = 20;  ///< Minimum brightness for beat modulation
    static const uint8_t MAX_BRIGHTNESS = 255; ///< Maximum brightness for beat modulation
    static const uint8_t BASE_BLEND_AMOUNT = 24; ///< Base amount for color blending
};

#endif // FILL_BEAT_EFFECT_H