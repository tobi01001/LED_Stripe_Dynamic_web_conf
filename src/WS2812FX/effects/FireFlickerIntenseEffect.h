#ifndef FIRE_FLICKER_INTENSE_EFFECT_H
#define FIRE_FLICKER_INTENSE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fire Flicker Intense effect - creates realistic intense flickering fire simulation
 * 
 * This effect simulates the appearance of intense flickering flames by randomly
 * varying the brightness of each LED. Each pixel is filled with colors from the
 * current palette (typically fire colors) and then randomly darkened to create
 * the flickering effect. The "intense" variant uses the maximum flicker intensity.
 * 
 * Key features:
 * - Individual pixel-level random brightness variation
 * - Uses current color palette for themed fire colors
 * - Intense flickering with maximum variation range
 * - Distributed color mapping across the strip length
 * - Stateless design - no persistent variables needed
 * - Real-time random generation for organic appearance
 */
class FireFlickerIntenseEffect : public Effect {
public:
    FireFlickerIntenseEffect() = default;
    virtual ~FireFlickerIntenseEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Apply random flicker effect to a single pixel
     * @param pixel Reference to the pixel color to modify
     * @param flickerIntensity Maximum amount of flicker (higher = more intense)
     */
    void applyFlicker(CRGB& pixel, uint8_t flickerIntensity);
};

#endif // FIRE_FLICKER_INTENSE_EFFECT_H