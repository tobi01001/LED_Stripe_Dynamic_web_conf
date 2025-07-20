#ifndef EASE_BAR_EFFECT_H
#define EASE_BAR_EFFECT_H

#include "../Effect.h"

/**
 * @brief Ease Bar effect - creates a dynamic bar that changes size, speed, and position
 * 
 * This effect generates a colorful bar that:
 * - Continuously changes its size and position using beat-synchronized animations
 * - Uses dual beat frequencies to create complex motion patterns
 * - Applies palette-based coloring with automatic hue progression
 * - Incorporates brightness pulsing synchronized to the movement
 * - Includes smooth fade-out effects for trailing visuals
 * 
 * The effect creates an organic, breathing-like motion where the bar appears to
 * ease in and out of different positions and sizes. The timing is derived from
 * the speed setting (beat88 parameter) which controls the overall animation tempo.
 * 
 * Internal state minimizes external dependencies and maintains animation continuity.
 */
class EaseBarEffect : public Effect {
public:
    EaseBarEffect() = default;
    virtual ~EaseBarEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Internal state structure for effect animation
     * 
     * All variables needed for animation are contained within this structure
     * to minimize dependencies on external shared resources.
     */
    struct {
        uint8_t counter;           ///< General purpose counter for offset calculations
        uint32_t lastUpdate;      ///< Timestamp of last update for timing control
        uint16_t minLeds;          ///< Minimum number of LEDs in the bar
        uint8_t beatFreq1;         ///< Primary beat frequency for position
        uint8_t beatFreq2;         ///< Secondary beat frequency for size
        uint16_t phaseOffset1;     ///< Phase offset for primary animation
        uint16_t phaseOffset2;     ///< Phase offset for secondary animation
    } state;

    /**
     * @brief Calculate minimum bar size based on strip length
     * @param stripLength Total length of the LED strip
     * @return Minimum number of LEDs for the bar (minimum 10)
     */
    uint16_t calculateMinLeds(uint16_t stripLength);

    /**
     * @brief Calculate beat frequencies from speed parameter
     * @param speed Speed value from segment configuration (beat88)
     * @param freq1 Output: Primary frequency
     * @param freq2 Output: Secondary frequency
     */
    void calculateBeatFrequencies(uint16_t speed, uint8_t& freq1, uint8_t& freq2);

    /**
     * @brief Calculate palette increment for smooth color progression
     * @param stripLength Length of the strip
     * @param paletteDistribution Palette distribution setting
     * @return Increment value for palette indexing
     */
    uint8_t calculatePaletteIncrement(uint16_t stripLength, uint8_t paletteDistribution);
};

#endif // EASE_BAR_EFFECT_H