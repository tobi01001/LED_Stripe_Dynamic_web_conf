#ifndef FADE_EFFECT_H
#define FADE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fade effect - creates smooth fading animation using current palette
 * 
 * This effect fills the entire LED strip with colors from the current palette,
 * applying a smooth triangular wave-based brightness modulation that creates
 * a gentle fade in/out effect. The fade speed is controlled by the beat88 
 * (speed) parameter, and the base color shifts slowly through the hue spectrum.
 * 
 * Key characteristics:
 * - Uses triwave8() for smooth fading transition
 * - Maps beat timing to brightness using triangle wave
 * - Applies palette colors with brightness modulation
 * - Minimal external resource dependency (only uses strip's current palette)
 * 
 * Memory usage: ~16 bytes (timebase + initialization flag)
 */
class FadeEffect : public Effect {
public:
    FadeEffect() = default;
    virtual ~FadeEffect() = default;

    /**
     * @brief Initialize the fade effect
     * @param strip Pointer to the WS2812FX instance for accessing segment data
     * @return true if initialization was successful
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the fade effect for one frame
     * 
     * This method:
     * 1. Calculates current fade position using beat88 timing
     * 2. Maps the position to a triangular wave for smooth fade
     * 3. Applies the brightness to palette colors across the strip
     * 4. Updates the timebase for next frame
     * 
     * @param strip Pointer to the WS2812FX instance
     * @return Minimum delay (STRIP_MIN_DELAY) for smooth animation
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the display name of this effect
     * @return Flash string containing "Fade"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_FADE from the MODES enum
     */
    uint8_t getModeId() const override;

private:
    // Effect-specific state variables (minimizes shared resource usage)
    uint32_t timebase = 0;      ///< Base timestamp for beat timing calculations
    
    // Constants for effect behavior
    static const uint8_t FADE_SPEED_MULTIPLIER = 10; ///< Speed scaling factor for beat timing
    static const uint8_t MIN_BRIGHTNESS = 24;        ///< Minimum brightness level for fade
    static const uint8_t MAX_BRIGHTNESS = 255;       ///< Maximum brightness level for fade
    static const uint8_t HUE_DELTA = 5;              ///< Hue increment across strip length
};

#endif // FADE_EFFECT_H