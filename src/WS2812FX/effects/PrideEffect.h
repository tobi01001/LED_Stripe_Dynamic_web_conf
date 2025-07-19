#ifndef PRIDE_EFFECT_H
#define PRIDE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Pride effect - creates beautiful, slow-moving rainbow colors reminiscent of pride flags
 * 
 * This effect generates a dynamic rainbow pattern with varying hue and brightness.
 * The effect creates smooth, wave-like color transitions that slowly move and change
 * over time, creating an organic, flowing appearance.
 * 
 * Key features:
 * - Dynamic hue cycling with beatsin-based modulation
 * - Variable brightness creates depth and movement
 * - Self-contained timing and state management
 * - Smooth color blending across the strip
 */
class PrideEffect : public Effect {
public:
    PrideEffect() = default;
    virtual ~PrideEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Internal state variables - replaces the shared modevars union
    uint16_t sPseudotime;     ///< Accumulated pseudo-time for brightness waves
    uint16_t sLastMillis;     ///< Last update timestamp for delta calculations
    uint16_t sHue16;          ///< Current hue position with sub-pixel precision
    
    /**
     * @brief Calculate brightness depth modulation
     * @param beat88 Current beat value from strip speed setting
     * @return Brightness depth value (96-224 range)
     */
    uint8_t calculateBrightDepth(uint16_t beat88) const;
    
    /**
     * @brief Calculate brightness theta increment for wave effect
     * @param beat88 Current beat value from strip speed setting
     * @return Theta increment value for brightness waves
     */
    uint16_t calculateBrightnessInc(uint16_t beat88) const;
    
    /**
     * @brief Calculate time multiplier for effect speed
     * @param beat88 Current beat value from strip speed setting
     * @return Multiplier for time-based calculations
     */
    uint8_t calculateTimeMultiplier(uint16_t beat88) const;
    
    /**
     * @brief Calculate hue increment for color progression
     * @param beat88 Current beat value from strip speed setting
     * @return Hue increment value
     */
    uint16_t calculateHueInc(uint16_t beat88) const;
};

#endif // PRIDE_EFFECT_H