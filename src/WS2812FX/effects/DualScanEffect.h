#ifndef DUAL_SCAN_EFFECT_H
#define DUAL_SCAN_EFFECT_H

#include "../Effect.h"

// Forward declaration to avoid circular dependency
class WS2812FX;

/**
 * @brief Dual Scan effect - creates two light bars scanning in opposite directions
 * 
 * This effect displays two moving light bars that travel across the LED strip
 * in opposite directions. One bar moves from start to end, while the other
 * moves from end to start, creating a mirrored scanning pattern.
 * 
 * Key features:
 * - Two bars moving in opposite directions simultaneously
 * - Smooth back-and-forth scanning motion using triwave function
 * - Fractional positioning for sub-pixel accuracy and smoothness
 * - Palette-based coloring with complementary hues for each bar
 * - Self-contained timing using internal timebase
 * - Configurable bar width (currently fixed at 2 pixels)
 */
class DualScanEffect : public Effect {
public:
    DualScanEffect() = default;
    virtual ~DualScanEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    
    // Effect configuration constants
    static const uint16_t BAR_WIDTH = 2;  ///< Width of each scanning bar in pixels
    
    /**
     * @brief Calculate the fractional position of the forward-moving bar
     * @param strip Pointer to the WS2812FX instance for accessing speed settings
     * @return 16-bit fractional position (position * 16) for sub-pixel accuracy
     */
    uint16_t calculateForwardBarPosition(WS2812FX* strip) const;
    
    /**
     * @brief Calculate the fractional position of the reverse-moving bar
     * @param strip Pointer to the WS2812FX instance for accessing speed settings
     * @return 16-bit fractional position (position * 16) for sub-pixel accuracy
     */
    uint16_t calculateReverseBarPosition(WS2812FX* strip) const;
};

#endif // DUAL_SCAN_EFFECT_H