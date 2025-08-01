#ifndef FILL_WAVE_EFFECT_H
#define FILL_WAVE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fill Wave effect - creates animated waves using palette colors
 * 
 * This effect fills the LED strip with colors from the current palette,
 * creating smooth wave-like animations by modulating the hue offset and
 * brightness using beatsin functions. The wave effect is achieved by:
 * 
 * - Using beatsin88() to create smooth hue oscillations across the palette
 * - Modulating brightness with a second beatsin88() function
 * - Respecting palette distribution settings for color spacing
 * - Creating continuous flowing color waves
 * 
 * The effect uses minimal external shared resources by maintaining its own
 * timebase for consistent animation timing.
 */
class FillWaveEffect : public Effect {
public:
    FillWaveEffect() = default;
    virtual ~FillWaveEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
};

#endif // FILL_WAVE_EFFECT_H