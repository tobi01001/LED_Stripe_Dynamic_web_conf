#include "FadeEffect.h"
#include "../WS2812FX_FastLed.h"

/**
 * @brief Initialize the fade effect when first activated
 * 
 * Sets up the initial state for the fade effect, establishing the timing
 * baseline and resetting initialization flags. This method is idempotent
 * and safe to call multiple times.
 * 
 * @param strip Pointer to WS2812FX instance providing access to:
 *              - Segment configuration (speed, palette, etc.)
 *              - Runtime state management
 *              - LED strip parameters
 * @return true Always succeeds unless strip is null
 */
bool FadeEffect::init(WS2812FX* strip) {
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
 * @brief Render one frame of the fade effect
 * 
 * This method implements the core fade animation logic:
 * 1. Calculates the current position in the fade cycle using beat timing
 * 2. Maps this position to a triangular wave for smooth brightness transitions
 * 3. Applies the brightness modulation to colors from the current palette
 * 4. Fills the entire LED segment with the modulated colors
 * 
 * The fade creates a breathing effect where the entire strip fades in and out
 * smoothly, with colors distributed across the strip length using the palette.
 * 
 * Mathematical approach:
 * - beat88() provides time-based position in animation cycle
 * - triwave8() converts linear position to triangular wave (smooth fade)
 * - map8() scales brightness to usable range
 * - fill_palette() applies brightness to palette colors across strip
 * 
 * @param strip Pointer to WS2812FX instance for accessing:
 *              - Current segment configuration (speed, palette, blend mode)
 *              - Runtime state (baseHue, start/stop positions)
 *              - LED array for rendering output
 * @return STRIP_MIN_DELAY Minimum delay for smooth animation timing
 */
uint16_t FadeEffect::update(WS2812FX* strip) {
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
    
    // Calculate current position in animation cycle
    // beat88() creates smooth timing based on BPM (beats per minute) parameter
    // Formula: beat88(bpm * multiplier, timebase) gives position from 0-65535
    uint16_t beat_position = beat88(seg->beat88 * FADE_SPEED_MULTIPLIER, timebase);
    
    // Convert beat position (0-65535) to 8-bit range (0-255) for triwave8
    uint8_t wave_position = map(beat_position, 
                               (uint16_t)0, (uint16_t)65535,    // Input range (beat88 output)
                               (uint8_t)0, (uint8_t)255);      // Output range (triwave8 input)
    
    // Generate triangular wave for smooth fade in/out effect
    // triwave8() produces smooth transition: 0->255->0 over full cycle
    uint8_t triangle_brightness = triwave8(wave_position);
    
    // Map triangle wave to practical brightness range
    // Avoids complete darkness (MIN_BRIGHTNESS) for better visual effect
    uint8_t final_brightness = map8(triangle_brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    
    // Apply the fade effect to the LED strip
    // fill_palette parameters:
    // - &(strip->leds[runtime->start]): Starting LED position in array
    // - runtime->length: Number of LEDs to fill
    // - 0 + runtime->baseHue: Starting hue offset (allows hue rotation)
    // - HUE_DELTA: Hue increment per LED (creates color distribution)
    // - *strip->getCurrentPalette(): Current color palette for effect
    // - final_brightness: Calculated brightness for fade effect  
    // - seg->blendType: Blending mode (LINEARBLEND or NOBLEND)
    fill_palette(&(strip->leds[runtime->start]), 
                runtime->length,                    // Number of LEDs to update
                0 + runtime->baseHue,              // Starting hue position
                HUE_DELTA,                         // Hue increment between LEDs
                *strip->getCurrentPalette(),       // Color palette to use
                final_brightness,                  // Brightness for this frame
                seg->blendType);                   // Color blending mode
    
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
 * @return Flash string containing "Fade" (stored in program memory to save RAM)
 */
const __FlashStringHelper* FadeEffect::getName() const {
    return F("Fade");
}

/**
 * @brief Get the mode identifier for this effect
 * 
 * Returns the mode ID that corresponds to this effect in the MODES enum.
 * This ID is used by the effect system to map mode selections to effect classes.
 * 
 * @return FX_MODE_FADE from the MODES enumeration
 */
uint8_t FadeEffect::getModeId() const {
    return FX_MODE_FADE;
}

// Register this effect with the effect factory system
// This macro creates the necessary glue code to instantiate the effect
// when FX_MODE_FADE is requested by the main effect system
REGISTER_EFFECT(FX_MODE_FADE, FadeEffect)