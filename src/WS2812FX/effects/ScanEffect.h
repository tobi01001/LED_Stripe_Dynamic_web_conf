#ifndef SCAN_EFFECT_H
#define SCAN_EFFECT_H

#include "../Effect.h"

// Forward declaration to avoid circular dependency
class WS2812FX;

/**
 * @brief Scan effect - creates a single light bar that scans back and forth across the strip
 * 
 * This effect displays a moving light bar that smoothly travels from one end of the 
 * LED strip to the other and back again. The bar uses fractional positioning for
 * smooth animation and applies colors from the current palette.
 * 
 * Key features:
 * - Smooth back-and-forth scanning motion using triwave function
 * - Fractional positioning for sub-pixel accuracy and smoothness
 * - Palette-based coloring with hue progression
 * - Self-contained timing using internal timebase
 * - Configurable bar width (currently fixed at 2 pixels)
 */
class ScanEffect : public Effect {
public:
    ScanEffect() = default;
    virtual ~ScanEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Internal state variables - replaces the shared modevars union
    uint32_t timebase = 0;        ///< Base time for beat calculations and smooth motion
    
    // Effect configuration constants
    static const uint16_t BAR_WIDTH = 2;  ///< Width of the scanning bar in pixels
    
    /**
     * @brief Calculate the fractional position of the scanning bar within segment bounds
     * @param trianglePosition Triangle wave position (0-65535) from helper
     * @param runtime_ptr Pointer to runtime data for segment boundaries
     * @return 16-bit fractional position relative to segment start
     */
    uint16_t calculateBarPosition(uint16_t trianglePosition, const void* runtime_ptr) const;
};

#endif // SCAN_EFFECT_H