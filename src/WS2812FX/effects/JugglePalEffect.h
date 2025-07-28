#ifndef JUGGLE_PAL_EFFECT_H
#define JUGGLE_PAL_EFFECT_H

#include "../Effect.h"

/**
 * @brief Juggle Pal effect - creates multiple moving bars with varying colors
 * 
 * This effect creates several moving bars (dots) that move at different speeds
 * across the LED strip. Each bar has a slightly different color and moves
 * with a sinusoidal motion. The colors slowly shift over time for dynamic
 * variation. The number of bars is controlled by the numBars setting.
 */
class JugglePalEffect : public Effect {
public:
    JugglePalEffect() = default;
    virtual ~JugglePalEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t lastHueChange = 0;     ///< Timestamp of last hue change
    uint8_t currentHue = 0;         ///< Current base hue for color variations
    bool initialized = false;       ///< Initialization flag to ensure proper setup
    
    // Constants for effect behavior
    static const uint8_t MIN_BAR_WIDTH = 3;        ///< Minimum width for bars
    static const uint8_t FADE_AMOUNT = 96;         ///< Amount to fade background
    static const uint16_t HUE_CHANGE_INTERVAL = 100; ///< Milliseconds between hue changes
    static const uint8_t MAX_HUE_DELTA = 8;        ///< Maximum hue change per update
};

#endif // JUGGLE_PAL_EFFECT_H