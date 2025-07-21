#include "SunsetEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../../include/defaults.h"

/**
 * @brief Initialize the sunset effect
 * 
 * Sets up the initial state for the sunset effect. The effect analyzes the
 * current LED strip brightness to determine an appropriate starting point
 * in the sunset progression, then continues from there to complete darkness.
 * 
 * @param strip Pointer to the WS2812FX instance
 * @return true if initialization was successful
 */
bool SunsetEffect::init(WS2812FX* strip) {
    // Initialize effect state
    state.nextStepTime = millis();
    state.alternateToggle = false;
    state.lastNoiseUpdate = 0;
    state.initialized = false;  // Will calculate initial step on first update
    
    // Initialize noise array to zero
    memset(state.noiseValues, 0, sizeof(state.noiseValues));
    
    // Configure strip settings for sunset
    auto seg = strip->getSegment();
    
    // Disable autoplay during sunset
    seg->autoplay = AUTO_MODE_OFF;
    
    // Clear background color settings as they interfere with sunset
    strip->setBckndBri(0);
    
    // Initial step will be calculated on first update based on current brightness
    // This allows sunset to start from current state rather than always from maximum
    
    return true;
}

/**
 * @brief Update the sunset effect for one frame
 * 
 * Handles the progression of the sunset effect by:
 * 1. Calculating initial step on first run based on current brightness
 * 2. Drawing the current sunset step
 * 3. Checking if it's time to advance to the next step
 * 4. Advancing the step (counting down) if the time interval has elapsed
 * 5. Automatically powering off when sunset is complete
 * 
 * The effect progresses through DEFAULT_SUNRISE_STEPS steps (in reverse)
 * over the configured sunset duration (stored in segment.sunrisetime).
 * 
 * @param strip Pointer to the WS2812FX instance
 * @return Always returns 0 to ensure smooth continuous updates
 */
uint16_t SunsetEffect::update(WS2812FX* strip) {
    auto seg = strip->getSegment();
    
    // Calculate step interval based on configured sunset duration
    // sunrisetime is in minutes, converted to milliseconds per step
    const uint16_t totalSteps = DEFAULT_SUNRISE_STEPS;
    uint16_t stepIntervalMs = (uint16_t)(seg->sunrisetime * (60 * 1000) / totalSteps);
    
    // Calculate initial step on first update
    if (!state.initialized) {
        state.currentStep = calculateInitialStep(strip);
        state.initialized = true;
    }
    
    // Draw current step
    drawSunsetStep(strip);
    
    // Check if it's time to advance to next step
    if (millis() >= state.nextStepTime) {
        state.nextStepTime = millis() + stepIntervalMs;
        
        // Count down to zero (sunset gets darker)
        if (state.currentStep > 0) {
            state.currentStep--;
        } else {
            // Sunset complete - switch to default mode and power off
            // This matches the behavior of the original implementation
            strip->setMode(DEFAULT_MODE);
            strip->setPower(false);
        }
    }
    
    // Return 0 for smooth continuous updates without artificial delays
    return 0;
}

/**
 * @brief Calculate color value for a specific step in sunset progression
 * 
 * This function uses the same color calculation algorithm as sunrise to ensure
 * consistent color transitions. The sunset effect simply uses the step values
 * in reverse (counting down from DEFAULT_SUNRISE_STEPS to 0).
 * 
 * The progression has the same phases as sunrise but in reverse:
 * - Phase 1: Bright white to warm orange/red (high step values)
 * - Phase 2: Warm orange/red to complete darkness (low step values)
 * 
 * @param step Current step in progression (DEFAULT_SUNRISE_STEPS = bright, 0 = dark)
 * @return CRGB color value for this step
 */
CRGB SunsetEffect::calculateSunsetColor(uint16_t step) const {
    double stepDouble = (double)step;
    
    // Handle boundary conditions
    if (stepDouble < SRSS_StartValue) {
        return CRGB(SRSS_StartR, SRSS_StartG, SRSS_StartB);
    }
    if (stepDouble > SRSS_Endvalue) {
        return CRGB(SRSS_EndR, SRSS_EndG, SRSS_EndB);
    }
    
    double red, green, blue;
    
    // Phase 1: Start to mid-point (dark to warm colors)
    // Note: Same calculation as sunrise - step value determines the color
    if (stepDouble <= SRSS_MidValue) {
        // Calculate interpolation parameter (0.0 to 1.0)
        double t = (stepDouble - SRSS_StartValue) / (SRSS_MidValue - SRSS_StartValue);
        
        // Quadratic bezier interpolation: B(t) = (1-t)²P₀ + 2(1-t)tP₁ + t²P₂
        red = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_StartR) +     // Start color weighted by (1-t)²
            (2.0 * (1.0 - t) * t * SRSS_Mid1R) +        // First control point
            (t * t * SRSS_Mid2R)                        // Second control point weighted by t²
        ) + 0.5) / 100.0;
        
        green = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_StartG) +
            (2.0 * (1.0 - t) * t * SRSS_Mid1G) +
            (t * t * SRSS_Mid2G)
        ) + 0.5) / 100.0;
        
        blue = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_StartB) +
            (2.0 * (1.0 - t) * t * SRSS_Mid1B) +
            (t * t * SRSS_Mid2B)
        ) + 0.5) / 100.0;
    }
    // Phase 2: Mid-point to end (warm colors to bright white)
    else {
        // Calculate interpolation parameter (0.0 to 1.0)
        double t = (stepDouble - SRSS_MidValue) / (SRSS_Endvalue - SRSS_MidValue);
        
        // Quadratic bezier interpolation for second phase
        red = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_Mid2R) +     // Previous end becomes new start
            (2.0 * (1.0 - t) * t * SRSS_Mid3R) +        // Third control point
            (t * t * SRSS_EndR)                         // Final bright color
        ) + 0.5) / 100.0;
        
        green = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_Mid2G) +
            (2.0 * (1.0 - t) * t * SRSS_Mid3G) +
            (t * t * SRSS_EndG)
        ) + 0.5) / 100.0;
        
        blue = (100.0 * (
            ((1.0 - t) * (1.0 - t) * SRSS_Mid2B) +
            (2.0 * (1.0 - t) * t * SRSS_Mid3B) +
            (t * t * SRSS_EndB)
        ) + 0.5) / 100.0;
    }
    
    // Ensure values are within valid range and return color
    return CRGB((uint8_t)red, (uint8_t)green, (uint8_t)blue);
}

/**
 * @brief Draw current sunset step to LED strip
 * 
 * This function renders the current sunset step with the same enhancements
 * as the sunrise effect:
 * 1. Base color from calculateSunsetColor()
 * 2. Dithering for smoother transitions between steps
 * 3. Subtle noise variations for realistic appearance
 * 
 * The rendering approach is identical to sunrise to ensure consistent
 * visual quality between the two effects.
 * 
 * @param strip Pointer to WS2812FX instance
 */
void SunsetEffect::drawSunsetStep(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate effective step with dithering for smoother transitions
    uint16_t effectiveStep = state.currentStep;
    if (state.alternateToggle) {
        // For sunset, we subtract 1 to dither toward darker colors
        if (effectiveStep > 0) {
            effectiveStep -= 1;
        }
    }
    state.alternateToggle = !state.alternateToggle;  // Toggle for next frame
    
    // Get base color for this step
    CRGB baseColor = calculateSunsetColor(effectiveStep);
    
    // Fill entire strip with base color
    fill_solid(&strip->leds[runtime->start], runtime->length, baseColor);
    
    // Update noise values periodically for realistic variations
    updateNoiseValues();
    
    // Apply noise effect to create subtle brightness variations
    // This simulates the natural flickering and variations in real sunlight
    // Pre-calculate the maximum valid noise index
    uint16_t maxNoiseIndex = (uint16_t)(sizeof(state.noiseValues) - 1);
    
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Get noise value for this LED (ensure we don't exceed array bounds)
        uint8_t noiseIndex = (i < maxNoiseIndex) ? i : maxNoiseIndex;
        uint8_t noiseValue = state.noiseValues[noiseIndex];
        
        // Apply noise as brightness scaling (185 max = ~72% min brightness)
        CRGB pixelColor = strip->leds[runtime->start + i];
        pixelColor.nscale8_video(noiseValue);
        
        // Blend the noisy color back with original for subtle effect
        strip->leds[runtime->start + i] = nblend(strip->leds[runtime->start + i], pixelColor, 64);
    }
}

/**
 * @brief Update noise values for realistic flickering
 * 
 * Generates new random noise values every 100ms to create subtle
 * brightness variations across the LED strip. The noise values range
 * from 0 to 185, providing brightness scaling from 0% to ~72%.
 * 
 * This creates a more realistic sunset appearance by simulating
 * the natural variations in sunlight intensity.
 */
void SunsetEffect::updateNoiseValues() {
    uint32_t currentTime = millis();
    
    // Update noise values every 100ms
    if (currentTime - state.lastNoiseUpdate >= 100) {
        state.lastNoiseUpdate = currentTime;
        
        // Generate new random values for brightness variations
        for (uint16_t i = 0; i < sizeof(state.noiseValues); i++) {
            state.noiseValues[i] = random8(0, 185);  // 0 to ~72% brightness
        }
    }
}

/**
 * @brief Calculate initial sunset step based on current strip brightness
 * 
 * This function analyzes the current LED strip to determine what step in the
 * sunset progression most closely matches the current brightness/color state.
 * This allows sunset to start from the current state rather than always
 * starting from maximum brightness.
 * 
 * The algorithm:
 * 1. Calculates average luminance of current LED strip
 * 2. Ensures minimum luminance threshold for meaningful comparison
 * 3. Compares against sunset color progression to find best match
 * 4. Returns the step that produces the closest color match
 * 
 * @param strip Pointer to WS2812FX instance
 * @return Appropriate starting step for sunset progression
 */
uint16_t SunsetEffect::calculateInitialStep(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate average luminance of current LED strip state
    uint32_t totalLuma = 0;
    uint8_t averageLuma = 255;  // Default to maximum brightness
    
    // Sum luminance values across all LEDs in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        totalLuma += strip->leds[runtime->start + i].getLuma();
    }
    
    // Calculate average if we have LEDs
    if (runtime->length > 0 && totalLuma > 0) {
        averageLuma = (uint8_t)(totalLuma / runtime->length);
    }
    
    // Ensure minimum threshold for meaningful comparison
    // If current brightness is very low, we'll start from a reasonable point
    if (averageLuma < 96) {
        averageLuma = 96;
    }
    
    // Find the step that produces a color closest to current average luminance
    // Start from maximum step and work backwards until we find a match
    for (uint16_t step = DEFAULT_SUNRISE_STEPS; step > 0; step--) {
        CRGB stepColor = calculateSunsetColor(step);
        uint8_t stepLuma = stepColor.getLuma();
        
        // If this step's luminance is less than or equal to current, use it
        if (stepLuma <= averageLuma) {
            return step;
        }
    }
    
    // Default fallback - start from maximum brightness
    return DEFAULT_SUNRISE_STEPS;
}

/**
 * @brief Get the name of this effect
 * @return Flash string containing "Sunset"
 */
const __FlashStringHelper* SunsetEffect::getName() const {
    return F("Sunset");
}

/**
 * @brief Get the mode ID for this effect
 * @return FX_MODE_SUNSET
 */
uint8_t SunsetEffect::getModeId() const {
    return FX_MODE_SUNSET;
}

/**
 * @brief Clean up when effect is being deactivated
 * 
 * Performs any necessary cleanup when switching away from this effect.
 * For sunset effect, no special cleanup is needed as all state is
 * contained within the effect instance.
 */
void SunsetEffect::cleanup() {
    // No special cleanup required for sunset effect
    // All state is self-contained within this instance
}

// Register this effect with the factory system
REGISTER_EFFECT(FX_MODE_SUNSET, SunsetEffect)