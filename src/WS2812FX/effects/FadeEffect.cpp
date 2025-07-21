#include "FadeEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

/**
 * @brief Initialize the fade effect when first activated
 * 
 * Uses the standard initialization pattern from EffectHelper to set up
 * timing baseline and initialization flags consistently.
 */
bool FadeEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    return EffectHelper::standardInit(strip, timebase, initialized);
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
/**
 * @brief Render one frame of the fade effect
 * 
 * Uses EffectHelper utilities to calculate beat-based triangular wave
 * and apply it as brightness modulation to the current palette.
 */
uint16_t FadeEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Get segment and runtime configuration data
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return 1000; // Return reasonable delay if data is unavailable
    }
    
    // Calculate current position in animation cycle using helper
    uint16_t beatPosition = EffectHelper::calculateBeatPosition(strip, timebase, FADE_SPEED_MULTIPLIER);
    
    // Generate triangular wave for smooth fade in/out effect using helper
    uint8_t finalBrightness = EffectHelper::generateTriangleWave(beatPosition, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    
    // Apply the fade effect to the LED strip using helper
    EffectHelper::fillPaletteWithBrightness(strip, finalBrightness, HUE_DELTA);
    
    // Return minimum delay for smooth animation
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