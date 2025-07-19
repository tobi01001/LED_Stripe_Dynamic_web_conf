#ifndef THEATER_CHASE_DUAL_PALETTE_EFFECT_H
#define THEATER_CHASE_DUAL_PALETTE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Theater Chase Dual Palette effect - three-pixel chase with background colors
 * 
 * This effect extends the classic theater chase pattern by adding dim background
 * colors instead of completely black non-active pixels. This creates a richer
 * visual effect where the chase pattern moves over a subtle colored background.
 * 
 * Key characteristics:
 * - Three-pixel repeating chase pattern (same as theater chase)
 * - Active pixels: Full brightness from current palette
 * - Background pixels: Dim colors from offset palette position
 * - Uses dual palette positions for foreground/background contrast
 * - Smooth movement using beat timing system
 * - Minimal external resource dependency
 * 
 * Pattern behavior:
 * - Position 0: LEDs 0,3,6,9... are bright (foreground), others are dim (background)
 * - Position 1: LEDs 1,4,7,10... are bright (foreground), others are dim (background)
 * - Position 2: LEDs 2,5,8,11... are bright (foreground), others are dim (background)
 * - Background uses offset palette colors for visual contrast
 * 
 * Color strategy:
 * - Foreground: palette[position] at full brightness
 * - Background: palette[position + 128] at reduced brightness (32/255)
 * - Creates complementary color scheme with clear distinction
 * 
 * Memory usage: ~16 bytes (timebase + initialization flag)
 */
class TheaterChaseDualPaletteEffect : public Effect {
public:
    TheaterChaseDualPaletteEffect() = default;
    virtual ~TheaterChaseDualPaletteEffect() = default;

    /**
     * @brief Initialize the theater chase dual palette effect
     * @param strip Pointer to the WS2812FX instance for accessing segment data
     * @return true if initialization was successful
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the theater chase dual palette effect for one frame
     * 
     * This method:
     * 1. Calculates current chase position using beat timing (0-2 cycle)
     * 2. Iterates through all LEDs in the segment
     * 3. For active chase LEDs: sets full brightness palette color
     * 4. For background LEDs: sets dim offset palette color
     * 5. Creates rich dual-tone chase effect
     * 
     * @param strip Pointer to the WS2812FX instance
     * @return Minimum delay (STRIP_MIN_DELAY) for smooth animation
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the display name of this effect
     * @return Flash string containing "Theater Chase Dual palette"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_THEATER_CHASE_DUAL_P from the MODES enum
     */
    uint8_t getModeId() const override;

private:
    // Effect-specific state variables (minimizes shared resource usage)
    bool initialized = false;    ///< Tracks initialization state to avoid re-init
    uint32_t timebase = 0;      ///< Base timestamp for beat timing calculations
    
    // Constants for effect behavior
    static const uint8_t CHASE_PATTERN_SIZE = 3;     ///< Number of positions in chase pattern
    static const uint8_t SPEED_DIVISOR = 2;          ///< Speed reduction factor for smoother movement
    static const uint8_t FOREGROUND_BRIGHTNESS = 255;///< Full brightness for active chase pixels
    static const uint8_t BACKGROUND_BRIGHTNESS = 32; ///< Dim brightness for background pixels
    static const uint8_t PALETTE_OFFSET = 128;       ///< Offset for background palette colors
    static const uint16_t BEAT_RANGE_MAX = 255;      ///< Maximum value for beat mapping
};

#endif // THEATER_CHASE_DUAL_PALETTE_EFFECT_H