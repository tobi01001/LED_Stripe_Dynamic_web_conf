#ifndef COLOR_WIPE_BASE_EFFECT_H
#define COLOR_WIPE_BASE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Base class for color wipe effects with different wave patterns
 * 
 * This class provides the common functionality for all color wipe effects.
 * The strip is filled with two colors that change when the wipe direction
 * reverses. Different wave functions create different wipe patterns.
 */
class ColorWipeBaseEffect : public Effect {
public:
    ColorWipeBaseEffect() = default;
    virtual ~ColorWipeBaseEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;

protected:
    // Abstract method for calculating wipe position - implemented by subclasses
    virtual uint16_t calculateWipePosition(WS2812FX* strip, uint32_t timebase) = 0;

private:
    uint32_t timebase = 0;               ///< Time reference for position calculations
    uint8_t currentColorIndex = 0;       ///< Current color index for wipe
    uint8_t previousColorIndex = 0;      ///< Previous color index for wipe
    uint16_t previousWavePosition = 0;   ///< Previous wave position (0-65535)
    bool isMovingUp = true;              ///< Direction of wipe movement
    bool needNewColor = true;            ///< Flag to trigger color change
    bool initialized = false;            ///< Initialization flag
    
    // Color transition state
    uint8_t targetColorIndex = 0;        ///< Target color for smooth transition
    uint8_t transitionStep = 0;          ///< Current step in color transition
    uint8_t transitionSteps = 0;         ///< Total steps for color transition
    
    // Helper methods
    void updateColorIndices(WS2812FX* strip);
    void fillWipeColors(WS2812FX* strip, uint16_t fractionalPos16);
};

#endif // COLOR_WIPE_BASE_EFFECT_H