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
    uint32_t timebase;        ///< Base time for beat calculations and smooth motion
    
    // Effect configuration constants
    static const uint16_t BAR_WIDTH = 2;  ///< Width of the scanning bar in pixels
    
    /**
     * @brief 16-bit triangle wave function for smooth back-and-forth motion
     * @param in Input value (0-65535)
     * @return Triangle wave output (0-65534, with smooth transitions)
     */
    static inline uint16_t triwave16(uint16_t in) {
        if (in & 0x8000) {
            in = 65535 - in;
        }
        return in << 1;
    }
    
    /**
     * @brief Calculate the fractional position of the scanning bar
     * @param strip Pointer to the WS2812FX instance for accessing speed settings
     * @return 16-bit fractional position (position * 16) for sub-pixel accuracy
     */
    uint16_t calculateBarPosition(WS2812FX* strip) const;
    
    /**
     * @brief Calculate the color index for the bar based on its position
     * @param position_16bit The current 16-bit fractional position of the bar
     * @param baseHue The base hue offset from the segment runtime
     * @return Color index for palette lookup
     */
    uint8_t calculateColorIndex(uint16_t position_16bit, uint8_t baseHue) const;
};

#endif // SCAN_EFFECT_H