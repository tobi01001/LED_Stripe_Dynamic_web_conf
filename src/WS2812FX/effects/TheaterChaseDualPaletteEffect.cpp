#include "TheaterChaseDualPaletteEffect.h"
#include "../WS2812FX_FastLed.h"

/**
 * @brief Initialize the theater chase dual palette effect when first activated
 * 
 * Sets up the initial state for the theater chase dual palette effect, 
 * establishing the timing baseline and resetting initialization flags. 
 * This method is idempotent and safe to call multiple times.
 * 
 * @param strip Pointer to WS2812FX instance providing access to:
 *              - Segment configuration (speed, palette, etc.)
 *              - Runtime state management
 *              - LED strip parameters
 * @return true Always succeeds unless strip is null
 */
bool TheaterChaseDualPaletteEffect::init(WS2812FX* strip) {
    if (!strip) {
        return false; // Safety check for null pointer
    }
    
    if (initialized) {
        return true; // Already initialized, avoid redundant setup
    }
    
    // Get runtime data to reset modeinit flag (compatibility with existing system)
    auto runtime = strip->getSegmentRuntime();
    if (runtime) {
        runtime->modeinit = false; // Mark mode as initialized
    }
    
    // Initialize timing baseline for consistent animation
    timebase = millis(); // Current time in milliseconds since boot
    
    initialized = true;
    return true;
}

/**
 * @brief Render one frame of the theater chase dual palette effect
 * 
 * This method implements the core theater chase dual palette animation logic:
 * 1. Calculates which of the 3 chase positions should be active this frame
 * 2. Iterates through all LEDs in the segment
 * 3. For active chase LEDs: applies full brightness palette color (foreground)
 * 4. For background LEDs: applies dim offset palette color (background)
 * 5. Creates rich visual effect with moving bright pattern over colored background
 * 
 * Dual palette strategy:
 * - Foreground (active chase): Uses direct palette lookup with full brightness
 * - Background (inactive): Uses palette lookup + 128 offset with reduced brightness
 * - The 128 offset typically provides complementary or contrasting colors
 * - Background brightness (32/255) ensures chase pattern remains prominent
 * 
 * Chase pattern explanation:
 * - Same as regular theater chase: LEDs divided into groups of 3
 * - Only one position within each group is "active" (foreground) at any time
 * - Other positions show background colors instead of black
 * - The active position cycles through 0->1->2->0... creating movement
 * - Speed is controlled by beat88 timing system
 * 
 * Mathematical approach:
 * - beat88() provides time-based position scaled by speed parameter
 * - Modulo 3 operation creates the three-position cycling pattern
 * - Each LED's index modulo 3 determines its group membership
 * - Color selection uses base palette index ± offset for contrast
 * 
 * @param strip Pointer to WS2812FX instance for accessing:
 *              - Current segment configuration (speed, palette, blend mode)
 *              - Runtime state (baseHue, start/stop positions, length)
 *              - LED array for rendering output
 * @return STRIP_MIN_DELAY Minimum delay for smooth animation timing
 */
uint16_t TheaterChaseDualPaletteEffect::update(WS2812FX* strip) {
    // Safety check for null pointer
    if (!strip) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Get segment and runtime configuration data
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    if (!seg || !runtime) {
        return 1000; // Return reasonable delay if data is unavailable
    }
    
    // Calculate current chase position (0, 1, or 2)
    // beat88() creates smooth timing based on BPM parameter divided by speed divisor
    // Formula: beat88(bpm / divisor, timebase) for slower, more visible movement
    uint16_t beat_position = beat88(seg->beat88 / SPEED_DIVISOR, timebase);
    
    // Map beat position to 8-bit range then take modulo 3 for chase pattern
    uint16_t chase_offset = map(beat_position, 
                               (uint16_t)0, (uint16_t)65535,        // Input range (beat88 output)
                               (uint16_t)0, (uint16_t)BEAT_RANGE_MAX) // Output range for modulo
                           % CHASE_PATTERN_SIZE; // Results in 0, 1, or 2
    
    // Apply the dual palette chase pattern to each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate absolute LED index in the strip
        uint16_t led_index = runtime->start + i;
        
        // Calculate base palette index for this LED position
        // Maps LED position to palette color range (0-255) with hue offset
        uint8_t base_palette_index = map(i, 
                                        (uint16_t)0, (uint16_t)(runtime->length - 1), // LED position range
                                        (uint8_t)0, (uint8_t)255)                     // Palette index range
                                    + runtime->baseHue; // Add base hue offset for color shifting
        
        // Determine if this LED should be foreground (active chase) or background
        if ((i % CHASE_PATTERN_SIZE) == chase_offset) {
            // This LED is part of the current active chase position (FOREGROUND)
            
            // Get foreground color from current palette with full brightness
            // Uses direct palette lookup for primary chase colors
            CRGB led_color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),    // Current color palette
                base_palette_index,            // Direct position in palette (0-255)
                FOREGROUND_BRIGHTNESS,         // Full brightness for visibility
                seg->blendType);               // Color blending mode
            
            // Set the LED to the calculated foreground color
            strip->leds[led_index] = led_color;
        } else {
            // This LED is not part of current active chase position (BACKGROUND)
            
            // Calculate offset palette index for background contrast
            // Adding 128 typically gives complementary or contrasting colors
            uint8_t background_palette_index = base_palette_index + PALETTE_OFFSET;
            
            // Get background color from offset palette position with reduced brightness
            // This creates subtle colored background instead of black
            CRGB led_color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),    // Current color palette
                background_palette_index,      // Offset position for contrast
                BACKGROUND_BRIGHTNESS,         // Reduced brightness for background
                seg->blendType);               // Color blending mode
            
            // Set the LED to the calculated background color
            strip->leds[led_index] = led_color;
        }
    }
    
    // Return minimum delay for smooth animation
    // STRIP_MIN_DELAY ensures consistent frame rate without overloading processor
    return strip->getStripMinDelay();
}

/**
 * @brief Get the display name for this effect
 * 
 * Provides a user-friendly name for the effect that will be displayed
 * in the web interface or other user-facing components. The name clearly
 * indicates this is the dual palette variant of theater chase.
 * 
 * @return Flash string containing "Theater Chase Dual palette" (stored in program memory to save RAM)
 */
const __FlashStringHelper* TheaterChaseDualPaletteEffect::getName() const {
    return F("Theater Chase Dual palette");
}

/**
 * @brief Get the mode identifier for this effect
 * 
 * Returns the mode ID that corresponds to this effect in the MODES enum.
 * This ID is used by the effect system to map mode selections to effect classes.
 * 
 * @return FX_MODE_THEATER_CHASE_DUAL_P from the MODES enumeration
 */
uint8_t TheaterChaseDualPaletteEffect::getModeId() const {
    return FX_MODE_THEATER_CHASE_DUAL_P;
}

// Register this effect with the effect factory system
// This macro creates the necessary glue code to instantiate the effect
// when FX_MODE_THEATER_CHASE_DUAL_P is requested by the main effect system
REGISTER_EFFECT(FX_MODE_THEATER_CHASE_DUAL_P, TheaterChaseDualPaletteEffect)