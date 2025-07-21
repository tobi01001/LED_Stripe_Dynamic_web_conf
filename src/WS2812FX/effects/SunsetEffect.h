#ifndef SUNSET_EFFECT_H
#define SUNSET_EFFECT_H

#include "../Effect.h"

/**
 * @brief Sunset effect - simulates a natural sunset progression
 * 
 * This effect creates a smooth sunset simulation by progressively changing colors
 * from bright white/yellow through warm oranges and reds to complete darkness.
 * The progression follows a natural color curve that mimics the appearance of
 * an actual sunset, essentially reversing the sunrise sequence.
 * 
 * The effect:
 * - Starts from current LED strip brightness/colors
 * - Calculates appropriate starting step based on current brightness
 * - Progresses through warm yellows and oranges
 * - Continues through deep reds and oranges
 * - Ends at complete darkness (black)
 * - Automatically powers off the strip when complete
 * 
 * Color progression uses the same bezier curve calculations as sunrise
 * but in reverse order, ensuring consistent color transitions.
 * 
 * Each instance manages its own state independently to minimize shared resource usage.
 */
class SunsetEffect : public Effect {
private:
    /**
     * @brief Internal state structure for sunset effect
     * 
     * This structure contains all state variables needed for the effect,
     * eliminating dependency on external shared memory structures.
     */
    struct SunsetState {
        uint32_t nextStepTime;      ///< Timestamp for next progression step
        uint16_t currentStep;       ///< Current step in sunset progression (DEFAULT_SUNRISE_STEPS to 0)
        bool alternateToggle;       ///< Toggle for dithering effect to smooth color transitions
        uint8_t noiseValues[LED_COUNT];   ///< Random noise values for realistic flickering effect (max LED_COUNT)
        uint32_t lastNoiseUpdate;   ///< Last time noise values were updated
        bool initialized;           ///< Flag to track if initial step calculation is complete
    } state;

    /**
     * @brief Calculate color value for a specific step in sunset progression
     * 
     * Uses the same bezier curve interpolation as sunrise to ensure consistent
     * color transitions. The sunset effect uses the same step values as sunrise
     * but counts down from maximum to zero.
     * 
     * @param step Current step in progression (DEFAULT_SUNRISE_STEPS = bright, 0 = dark)
     * @return CRGB color value for this step
     */
    CRGB calculateSunsetColor(uint16_t step) const;

    /**
     * @brief Draw current sunset step to LED strip
     * 
     * Fills the LED strip with the color corresponding to the current step,
     * applies dithering for smooth transitions, and adds subtle noise for
     * a more realistic sunset appearance.
     * 
     * @param strip Pointer to WS2812FX instance
     */
    void drawSunsetStep(WS2812FX* strip);

    /**
     * @brief Update noise values for realistic flickering
     * 
     * Generates random noise values used to create subtle brightness
     * variations that simulate the natural variations in sunlight.
     */
    void updateNoiseValues();

    /**
     * @brief Calculate initial sunset step based on current strip brightness
     * 
     * Analyzes the current LED strip to determine what step in the sunset
     * progression most closely matches the current brightness. This allows
     * sunset to start from the current state rather than always from maximum.
     * 
     * @param strip Pointer to WS2812FX instance
     * @return Appropriate starting step for sunset progression
     */
    uint16_t calculateInitialStep(WS2812FX* strip);

public:
    SunsetEffect() = default;
    virtual ~SunsetEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
    void cleanup() override;
};

#endif // SUNSET_EFFECT_H
