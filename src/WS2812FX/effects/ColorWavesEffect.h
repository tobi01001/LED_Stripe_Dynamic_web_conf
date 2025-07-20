#ifndef COLOR_WAVES_EFFECT_H
#define COLOR_WAVES_EFFECT_H

#include "../Effect.h"

/**
 * @brief Color Waves effect - creates flowing waves of color using sine wave mathematics
 * 
 * This effect generates smooth, flowing waves of color by:
 * - Using sine wave calculations to create natural wave motion
 * - Applying dynamic brightness patterns that move across the strip
 * - Continuously shifting hue values for flowing color transitions
 * - Blending new colors with existing pixels for smooth color mixing
 * - Providing speed-responsive animation timing and wave characteristics
 * 
 * The effect creates multiple interacting patterns:
 * - Hue progression that flows along the strip length
 * - Brightness waves that create depth and movement
 * - Time-based pseudorandom variations that prevent repetitive patterns
 * - Palette-based color selection with distribution control
 * 
 * Speed control affects:
 * - Overall animation tempo
 * - Wave frequency and amplitude
 * - Color transition speed
 * - Brightness modulation rate
 * 
 * The implementation uses precise sine wave mathematics to ensure smooth,
 * organic-looking wave motion without jarring transitions.
 */
class ColorWavesEffect : public Effect {
public:
    ColorWavesEffect() = default;
    virtual ~ColorWavesEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Internal state structure for wave animation
     * 
     * Contains all timing, phase, and wave parameter information needed
     * for smooth color wave animation. This eliminates dependencies on
     * external shared state variables.
     */
    struct {
        uint16_t hue16;           ///< Current 16-bit hue value for smooth transitions
        uint16_t pseudotime;      ///< Accumulated time for wave calculations
        uint32_t lastMillis;      ///< Timestamp of last update for delta calculations
        uint16_t brightnessDepth; ///< Current brightness modulation depth
        uint16_t brightnessTheta; ///< Brightness wave phase accumulator
        uint16_t hueIncrement;    ///< Hue increment per pixel
        uint8_t multiplier;       ///< Time multiplier for animation speed
    } state;

    /**
     * @brief Calculate wave parameters based on speed setting
     * @param speed Speed parameter from segment configuration (beat88)
     * @param brightDepth Output: Brightness modulation depth
     * @param brightThetaInc Output: Brightness wave increment
     * @param msMultiplier Output: Time multiplier for animation
     * @param hueInc Output: Hue increment per pixel
     */
    void calculateWaveParameters(uint16_t speed, 
                                 uint8_t& brightDepth,
                                 uint16_t& brightThetaInc,
                                 uint8_t& msMultiplier,
                                 uint16_t& hueInc);

    /**
     * @brief Calculate hue increment for color progression
     * @param speed Speed parameter from segment configuration
     * @return Hue increment value for smooth color flow
     */
    uint16_t calculateHueIncrement(uint16_t speed);

    /**
     * @brief Convert 16-bit hue to 8-bit with sine wave shaping
     * @param hue16 16-bit hue value
     * @return Shaped 8-bit hue value
     * 
     * Applies sine wave shaping to create more organic color transitions
     * instead of linear hue progression.
     */
    uint8_t shapeHue(uint16_t hue16);

    /**
     * @brief Calculate brightness using sine wave for given theta
     * @param theta Brightness wave phase angle
     * @param brightDepth Modulation depth
     * @return Calculated brightness value (0-255)
     */
    uint8_t calculateBrightness(uint16_t theta, uint8_t brightDepth);
};

#endif // COLOR_WAVES_EFFECT_H