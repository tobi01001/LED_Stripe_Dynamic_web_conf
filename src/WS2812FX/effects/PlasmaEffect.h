#ifndef PLASMA_EFFECT_H
#define PLASMA_EFFECT_H

#include "../Effect.h"

/**
 * @brief Plasma effect - creates flowing plasma-like patterns across the strip
 * 
 * This effect creates a dynamic plasma-like pattern by combining multiple 
 * mathematical waves (cubic wave and cosine wave) with different phases.
 * The result is a smooth, organic-looking flow of colors that continuously
 * moves and morphs across the LED strip.
 */
class PlasmaEffect : public Effect {
public:
    PlasmaEffect() = default;
    virtual ~PlasmaEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;  ///< Time reference for phase calculations
    bool initialized = false;  ///< Initialization flag to ensure proper setup
    
    // Constants for wave calculations
    static const uint8_t CUBIC_WAVE_FREQUENCY = 15;  ///< Frequency multiplier for cubic wave
    static const uint8_t COS_WAVE_FREQUENCY = 8;     ///< Frequency multiplier for cosine wave
    static const uint8_t BLEND_AMOUNT = 64;          ///< Amount for color blending
};

#endif // PLASMA_EFFECT_H