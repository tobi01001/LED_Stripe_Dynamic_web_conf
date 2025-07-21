#ifndef TWINKLE_MAP_EFFECT_H
#define TWINKLE_MAP_EFFECT_H

#include "../Effect.h"
#include "../../include/defaults.h"

/**
 * @brief Twinkle Base Color Effect - Creates palette-based twinkling with consistent base color mapping
 * 
 * This effect creates a sophisticated twinkling pattern where each LED position is mapped
 * to a specific color from the current palette, creating a consistent base color layout.
 * Individual LEDs randomly twinkle by brightening from their base color to a peak brightness
 * and then fading back down, creating a starfield or fairy light effect.
 * 
 * Key Algorithm Features:
 * - **Palette Mapping**: Each LED position maps to a specific palette color based on its 
 *   position in the strip, creating a consistent color distribution
 * - **Base Color Layer**: All LEDs maintain a dim base color (32/255 brightness) when not twinkling
 * - **Random Twinkle Initiation**: New twinkles start randomly based on configurable density
 * - **Smooth Brightness Transitions**: Uses bit-based state tracking for smooth fade in/out
 * - **Speed Control**: Fade speed controlled by beat88 parameter (mapped to 4-64 range)
 * 
 * State Machine per LED:
 * - State 0: Off/base color, waiting for random twinkle start
 * - Odd states (bit 0 = 1): Brightening phase, blending from base to peak color
 * - Even states (bit 0 = 0): Dimming phase, blending from peak back to base color
 * - State transitions: 0 → 1 → 3 → 5 → ... → 255 → 2 → 4 → 6 → ... → 254 → 0
 * 
 * Visual Characteristics:
 * - Maintains palette color distribution across the strip
 * - Creates organic, randomized twinkling patterns
 * - Twinkle density controlled by segment twinkleDensity parameter
 * - Respects palette blend types and hue rotation
 * - Smooth, eye-pleasing transitions without abrupt changes
 */
class TwinkleMapEffect : public Effect {
private:
    /**
     * @brief Per-LED state tracking array
     * 
     * Each element tracks the current brightness/fade state of the corresponding LED:
     * - 0: LED is at base color, may start twinkling
     * - 1-255 (odd): LED is brightening, value indicates current brightness blend factor
     * - 2-254 (even): LED is dimming, value indicates current brightness blend factor
     * 
     * The state values are used directly as blend factors between base and peak colors,
     * providing smooth transitions without requiring separate brightness calculations.
     */
    uint8_t* _pixelStates;
    
    /**
     * @brief Number of LEDs for which we have allocated state storage
     * 
     * Tracks the current allocation size to handle dynamic strip length changes.
     * When the strip length changes, we need to reallocate the state array.
     */
    uint16_t _allocatedLength;

public:
    /**
     * @brief Constructor - Initialize with no allocated state
     */
    TwinkleMapEffect() : _pixelStates(nullptr), _allocatedLength(0) {}
    
    /**
     * @brief Destructor - Clean up allocated memory
     */
    virtual ~TwinkleMapEffect() {
        cleanup();
    }

    /**
     * @brief Initialize the twinkle map effect
     * 
     * Sets up the effect state and prepares all LEDs with their base colors:
     * 1. Allocates memory for per-LED state tracking
     * 2. Initializes all pixel states to 0 (base color)
     * 3. Sets all LEDs to their mapped palette color at base brightness
     * 
     * @param strip Pointer to the WS2812FX instance providing LED control and configuration
     * @return true if initialization successful, false if memory allocation failed
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the twinkle map effect for one frame
     * 
     * Performs the complete twinkle algorithm for the current frame:
     * 
     * 1. **Random Twinkle Initiation**: For each LED in state 0, randomly decide whether
     *    to start a new twinkle based on the configured twinkle density
     * 
     * 2. **Brightness State Updates**: For each LED, update its state according to the
     *    state machine:
     *    - Odd states: Increment brightness towards peak (255), transition to dimming at peak
     *    - Even states: Decrement brightness towards base, transition to idle at minimum
     * 
     * 3. **Color Calculation**: For each LED, calculate the final color by blending between:
     *    - Base Color: Palette color for this position at 32/255 brightness
     *    - Peak Color: Same palette color with +4 RGB enhancement for sparkle effect
     *    - Blend Factor: Current pixel state value (0-255)
     * 
     * 4. **LED Updates**: Set each LED to its calculated color
     * 
     * The algorithm ensures smooth, continuous animation with organic randomness while
     * maintaining the consistent palette-based color mapping across the strip.
     * 
     * @param strip Pointer to the WS2812FX instance providing LED control and configuration
     * @return Delay in milliseconds until next frame (calculated from beat88 setting)
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the human-readable name of this effect
     * @return Flash string containing "Twinkle Base Color"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_TWINKLE_MAP constant
     */
    uint8_t getModeId() const override;

    /**
     * @brief Clean up when the effect is being deactivated
     * 
     * Frees allocated memory to prevent memory leaks when switching effects.
     * Called automatically by destructor and when effect changes.
     */
    void cleanup() override;

private:
    /**
     * @brief Ensure pixel state array is properly allocated for current strip length
     * 
     * Handles dynamic memory allocation and reallocation when strip length changes.
     * Preserves existing state data when possible during reallocation.
     * 
     * @param strip Pointer to WS2812FX instance to get current strip length
     * @return true if allocation successful, false on memory allocation failure
     */
    bool ensureStateArrayAllocated(WS2812FX* strip);

    /**
     * @brief Calculate the base color for a given LED position
     * 
     * Maps the LED position to a palette color with consistent distribution across
     * the strip. Uses the same mapping algorithm as the original implementation.
     * 
     * @param strip Pointer to WS2812FX instance for palette and configuration access
     * @param ledIndex Position of the LED within the segment (0-based)
     * @return Base color at reduced brightness (32/255) for this LED position
     */
    CRGB calculateBaseColor(WS2812FX* strip, uint16_t ledIndex);

    /**
     * @brief Calculate the peak color for a given LED position
     * 
     * Creates an enhanced version of the base color for the twinkle peak.
     * Uses the same palette mapping but with RGB enhancement for sparkle effect.
     * 
     * @param strip Pointer to WS2812FX instance for palette and configuration access
     * @param ledIndex Position of the LED within the segment (0-based)
     * @return Peak color for twinkle effect at this LED position
     */
    CRGB calculatePeakColor(WS2812FX* strip, uint16_t ledIndex);
};

#endif // TWINKLE_MAP_EFFECT_H