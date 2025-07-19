#ifndef TWINKLE_FADE_EFFECT_H
#define TWINKLE_FADE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Twinkle Fade Effect - Creates randomly twinkling LEDs that fade out gradually
 * 
 * This effect simulates twinkling stars or fairy lights by randomly lighting up LEDs
 * with colors from the current palette and gradually fading them out. The effect
 * maintains a target density of active (lit) LEDs and continuously adds new twinkles
 * while existing ones fade away.
 * 
 * Key features:
 * - Random LED selection for twinkle placement
 * - Palette-based color selection with random brightness
 * - Configurable twinkle density via segment settings
 * - Smooth fading using FastLED fade functions
 * - Speed-controlled fade rate based on beat88 setting
 */
class TwinkleFadeEffect : public Effect {
private:
    /**
     * @brief Last update time for fade timing control
     * 
     * Tracks the last time the fade operation was performed to ensure
     * consistent timing regardless of frame rate variations.
     */
    uint32_t _lastFadeTime;

public:
    TwinkleFadeEffect() : _lastFadeTime(0) {}
    virtual ~TwinkleFadeEffect() = default;

    /**
     * @brief Initialize the twinkle fade effect
     * @param strip Pointer to the WS2812FX instance providing LED control
     * @return true if initialization successful
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the twinkle fade effect for one frame
     * 
     * This method performs the following operations each frame:
     * 1. Fades all LEDs slightly towards black (fade out existing twinkles)
     * 2. Counts currently active (lit) LEDs to determine spark density
     * 3. Compares against target density based on twinkleDensity setting
     * 4. Adds new random twinkles if below target density
     * 5. Dims existing LEDs if at target density to create breathing effect
     * 
     * @param strip Pointer to the WS2812FX instance providing LED control
     * @return Delay in milliseconds until next frame update (0 for immediate)
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the human-readable name of this effect
     * @return Flash string containing "Twinkle Fade"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_TWINKLE_FADE constant
     */
    uint8_t getModeId() const override;
};

#endif // TWINKLE_FADE_EFFECT_H