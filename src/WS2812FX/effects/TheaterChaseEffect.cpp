#include "TheaterChaseEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

/**
 * @brief Initialize the theater chase effect using EffectHelper standard pattern
 */
bool TheaterChaseEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    return EffectHelper::standardInit(strip, timebase, initialized);
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
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 0; // Return reasonable delay if strip is invalid
    }
    // Ensure effect is properly initialized
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return minimum delay if init failed
        }
    }
    // Get segment and runtime configuration data
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate current chase position using helper
    uint16_t beat_position = EffectHelper::calculateBeatPosition(strip, timebase, SPEED_DIVISOR);
    
    // Map beat position to chase pattern using helper
    uint16_t chase_offset = EffectHelper::safeMapuint16_t(beat_position, 0, 65535, 0, BEAT_RANGE_MAX) % CHASE_PATTERN_SIZE;
    
    // Apply the chase pattern to each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate absolute LED index in the strip
        uint16_t led_index = runtime->start + i;
        
        // Determine if this LED should be lit in current chase position
        // LEDs are lit when (LED_position % 3) equals current_chase_offset
        if ((i % CHASE_PATTERN_SIZE) == chase_offset) {
            // This LED is part of the current active chase position
            
            // Calculate color index using helper
            uint8_t color_index = EffectHelper::calculateColorIndex(strip, i, runtime->baseHue);
            
            // Set LED to calculated palette color
            strip->leds[led_index] = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(),    // Current color palette
                color_index,                    // Color index within palette  
                seg->brightness,                // Use segment brightness setting
                seg->blendType);                // Use segment blend mode
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