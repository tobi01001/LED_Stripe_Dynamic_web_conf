#include "TheaterChaseEffect.h"
#include "../WS2812FX_FastLed.h"

/**
 * @brief Initialize the theater chase effect when first activated
 * 
 * Sets up the initial state for the theater chase effect, establishing the 
 * timing baseline and resetting initialization flags. This method is idempotent
 * and safe to call multiple times.
 * 
 * @param strip Pointer to WS2812FX instance providing access to:
 *              - Segment configuration (speed, palette, etc.)
 *              - Runtime state management  
 *              - LED strip parameters
 * @return true Always succeeds unless strip is null
 */
bool TheaterChaseEffect::init(WS2812FX* strip) {
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
 * @brief Render one frame of the theater chase effect
 * 
 * This method implements the core theater chase animation logic:
 * 1. Calculates which of the 3 chase positions should be active this frame
 * 2. Iterates through all LEDs in the segment
 * 3. For each LED, checks if its position matches the current chase pattern
 * 4. Sets matching LEDs to palette color, others to black
 * 
 * Chase pattern explanation:
 * - The chase divides LEDs into groups of 3: (0,1,2), (3,4,5), (6,7,8), etc.
 * - Only one position within each group is lit at any time
 * - The active position cycles through 0->1->2->0... creating movement
 * - Speed is controlled by beat88 timing system
 * 
 * Mathematical approach:
 * - beat88() provides time-based position scaled by speed parameter
 * - Modulo 3 operation creates the three-position cycling pattern
 * - Each LED's index modulo 3 determines its group membership
 * - LEDs are lit when their group position matches current active position
 * 
 * @param strip Pointer to WS2812FX instance for accessing:
 *              - Current segment configuration (speed, palette, blend mode)
 *              - Runtime state (baseHue, start/stop positions, length)
 *              - LED array for rendering output
 * @return STRIP_MIN_DELAY Minimum delay for smooth animation timing
 */
uint16_t TheaterChaseEffect::update(WS2812FX* strip) {
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
    
    // Apply the chase pattern to each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate absolute LED index in the strip
        uint16_t led_index = runtime->start + i;
        
        // Determine if this LED should be lit in current chase position
        // LEDs are lit when (LED_position % 3) equals current_chase_offset
        if ((i % CHASE_PATTERN_SIZE) == chase_offset) {
            // This LED is part of the current active chase position
            
            // Calculate palette index for color variation across strip
            // Maps LED position to palette color range (0-255)
            uint8_t palette_index = map(i, 
                                       (uint16_t)0, (uint16_t)(runtime->length - 1), // LED position range
                                       (uint8_t)0, (uint8_t)255)                     // Palette index range
                                   + runtime->baseHue; // Add base hue offset for color shifting
            
            // Get color from current palette with full brightness
            // ColorFromPaletteWithDistribution handles palette distribution settings
            CRGB led_color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),    // Current color palette
                palette_index,                  // Position in palette (0-255)
                FULL_BRIGHTNESS,               // Brightness level (full for chase)
                seg->blendType);               // Color blending mode
            
            // Set the LED to the calculated color
            strip->leds[led_index] = led_color;
        } else {
            // This LED is not part of current active chase position - turn it off
            strip->leds[led_index] = CRGB::Black; // Complete darkness for non-active LEDs
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
 * in the web interface or other user-facing components.
 * 
 * @return Flash string containing "Theater Chase" (stored in program memory to save RAM)
 */
const __FlashStringHelper* TheaterChaseEffect::getName() const {
    return F("Theater Chase");
}

/**
 * @brief Get the mode identifier for this effect
 * 
 * Returns the mode ID that corresponds to this effect in the MODES enum.
 * This ID is used by the effect system to map mode selections to effect classes.
 * 
 * @return FX_MODE_THEATER_CHASE from the MODES enumeration
 */
uint8_t TheaterChaseEffect::getModeId() const {
    return FX_MODE_THEATER_CHASE;
}

// Register this effect with the effect factory system
// This macro creates the necessary glue code to instantiate the effect
// when FX_MODE_THEATER_CHASE is requested by the main effect system
REGISTER_EFFECT(FX_MODE_THEATER_CHASE, TheaterChaseEffect)