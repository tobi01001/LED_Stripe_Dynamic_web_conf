#ifndef SUNRISE_EFFECT_H
#define SUNRISE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Sunrise effect - simulates a natural sunrise progression
 * 
 * This effect creates a smooth sunrise simulation by progressively changing colors
 * from dark/black through deep orange/red to bright yellow/white. The progression
 * follows a natural color curve that mimics the appearance of an actual sunrise.
 * 
 * The effect:
 * - Starts from complete darkness (black)
 * - Progresses through deep reds and oranges
 * - Continues to warm yellows and oranges
 * - Ends at bright white light
 * 
 * Color progression is calculated using bezier curves for smooth transitions
 * between predefined color points that represent different phases of sunrise.
 * 
 * Each instance manages its own state independently to minimize shared resource usage.
 */
class SunriseEffect : public Effect {
private:
    /**
     * @brief Internal state structure for sunrise effect
     * 
     * This structure contains all state variables needed for the effect,
     * eliminating dependency on external shared memory structures.
     */
    struct SunriseState {
        uint32_t nextStepTime;      ///< Timestamp for next progression step
        uint16_t currentStep;       ///< Current step in sunrise progression (0 to DEFAULT_SUNRISE_STEPS)
        bool alternateToggle;       ///< Toggle for dithering effect to smooth color transitions
        uint8_t noiseValues[300];   ///< Random noise values for realistic flickering effect (max LED_COUNT)
        uint32_t lastNoiseUpdate;   ///< Last time noise values were updated
    } state;

    /**
     * @brief Calculate color value for a specific step in sunrise progression
     * 
     * Uses bezier curve interpolation to create smooth color transitions
     * between predefined color points that simulate natural sunrise colors.
     * 
     * @param step Current step in progression (0 = dark, DEFAULT_SUNRISE_STEPS = bright)
     * @return CRGB color value for this step
     */
    CRGB calculateSunriseColor(uint16_t step) const;

    /**
     * @brief Draw current sunrise step to LED strip
     * 
     * Fills the LED strip with the color corresponding to the current step,
     * applies dithering for smooth transitions, and adds subtle noise for
     * a more realistic sunrise appearance.
     * 
     * @param strip Pointer to WS2812FX instance
     */
    void drawSunriseStep(WS2812FX* strip);

    /**
     * @brief Update noise values for realistic flickering
     * 
     * Generates random noise values used to create subtle brightness
     * variations that simulate the natural variations in sunlight.
     */
    void updateNoiseValues();

public:
    SunriseEffect() = default;
    virtual ~SunriseEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
    void cleanup() override;
};

#endif // SUNRISE_EFFECT_H