#ifndef FILL_BRIGHT_EFFECT_H
#define FILL_BRIGHT_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fill Bright effect - creates waving brightness across the strip
 * 
 * This effect fills the entire strip with palette colors and modulates
 * the brightness using a sine wave. The hue position also changes over time
 * creating a moving wave of brightness and color across the strip.
 */
class FillBrightEffect : public Effect {
public:
    FillBrightEffect() = default;
    virtual ~FillBrightEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Constants for effect behavior
    static const uint8_t MIN_BRIGHTNESS = 10;    ///< Minimum brightness for wave
    static const uint8_t MAX_BRIGHTNESS = 255;   ///< Maximum brightness for wave
    static const uint8_t HUE_SPEED_DIVISOR = 128; ///< Divisor for hue movement speed
    static const uint8_t MIN_HUE_SPEED = 2;      ///< Minimum hue movement speed
    static const uint8_t BRIGHTNESS_SPEED_DIVISOR = 32; ///< Divisor for brightness wave speed
    static const uint8_t MIN_BRIGHTNESS_SPEED = 1; ///< Minimum brightness wave speed
};

#endif // FILL_BRIGHT_EFFECT_H