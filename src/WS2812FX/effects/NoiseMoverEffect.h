#ifndef NOISE_MOVER_EFFECT_H
#define NOISE_MOVER_EFFECT_H

#include "../Effect.h"

/**
 * @brief Noise Mover effect - creates a moving bar using Perlin noise for position
 * 
 * This effect creates a moving colored bar that travels along the strip.
 * The position is determined by Perlin noise (inoise8), which creates smooth,
 * organic movement patterns. The bar has a fixed width and the background
 * fades out over time, creating trailing effects.
 */
class NoiseMoverEffect : public Effect {
public:
    NoiseMoverEffect() = default;
    virtual ~NoiseMoverEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;      ///< Time reference for noise calculations
    uint16_t noiseDist = 1234;  ///< Distance parameter for noise function
    bool initialized = false;   ///< Initialization flag to ensure proper setup
    
    // Constants for noise calculation
    static const uint16_t NOISE_Y_SCALE = 30;  ///< Y-axis scale for noise
    static const uint16_t BAR_WIDTH = 6;       ///< Width of the moving bar
    static const uint8_t FADE_AMOUNT = 48;     ///< Amount to fade background
};

#endif // NOISE_MOVER_EFFECT_H