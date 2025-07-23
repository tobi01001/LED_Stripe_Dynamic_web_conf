#ifndef TO_INNER_EFFECT_H
#define TO_INNER_EFFECT_H

#include "../Effect.h"

/**
 * @brief To Inner effect - creates pulsing animation towards the center
 * 
 * This effect creates a pulsing pattern that moves from both ends toward
 * the center of the strip. The center portion is filled with palette colors
 * and then mirrored to create symmetrical movement. Background fading is
 * applied for trailing effects.
 */
class ToInnerEffect : public Effect {
public:
    ToInnerEffect() = default;
    virtual ~ToInnerEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    uint32_t timebase = 0;  ///< Time reference for position calculations
    bool initialized = false;  ///< Initialization flag to ensure proper setup
    
    // Constants for effect behavior
    static const uint8_t MIN_FADE = 16;           ///< Minimum fade amount
    static const uint8_t HUE_INCREMENT = 5;      ///< Hue increment for color distribution
    static const uint16_t SPEED_MULTIPLIER = 5;  ///< Speed multiplier for beat calculations
    static const uint16_t SPEED_THRESHOLD = 13107; ///< Threshold for speed clamping
    static const uint16_t FADE_THRESHOLD = 16320;  ///< Threshold for fade calculation
};

#endif // TO_INNER_EFFECT_H