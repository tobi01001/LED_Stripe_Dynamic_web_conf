#include "SunriseEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"
#include "../../include/defaults.h"

// Ensure the SunriseEffectState struct has a fixed-size noiseValues array
// Example: If not present, add this to SunriseEffect.h
// struct SunriseEffectState {
//     uint32_t nextStepTime;
//     uint16_t currentStep;
//     bool alternateToggle;
//     uint32_t lastNoiseUpdate;
//     uint8_t noiseValues[32]; // Adjust size as needed
// };

bool SunriseEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize effect state - start from darkness
    state.nextStepTime = millis();
    state.currentStep = 0;
    state.alternateToggle = false;
    state.lastNoiseUpdate = 0;
    
    // Initialize noise array to zero
    memset(state.noiseValues, 0, sizeof(state.noiseValues));
    
    // Configure strip settings for sunrise
    auto seg = strip->getSegment();
    
    // Disable autoplay during sunrise
    seg->autoplay = AUTO_MODE_OFF;
    
    // Set target brightness to maximum for full sunrise effect
    seg->targetBrightness = 255;
    
    // Clear background color settings as they interfere with sunrise
    strip->setBckndBri(0);
    
    return true;
}

/**
 * @brief Update the sunrise effect for one frame
 * 
 * Handles the progression of the sunrise effect by:
 * 1. Drawing the current sunrise step
 * 2. Checking if it's time to advance to the next step
 * 3. Advancing the step if the time interval has elapsed
 * 
 * The effect progresses through DEFAULT_SUNRISE_STEPS steps over the
 * configured sunrise duration (stored in segment.sunrisetime).
 * 
 * @param strip Pointer to the WS2812FX instance
 * @return Always returns 0 to ensure smooth continuous updates
 */
uint16_t SunriseEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    auto seg = strip->getSegment();
    
    // Calculate step interval based on configured sunrise duration
    // sunrisetime is in minutes, converted to milliseconds per step
    const uint16_t totalSteps = DEFAULT_SUNRISE_STEPS;
    uint16_t stepIntervalMs = (uint16_t)(seg->sunrisetime * (60 * 1000) / totalSteps);
    
    // Draw current step
    drawSunriseStep(strip);
    
    // Check if it's time to advance to next step
    if (millis() >= state.nextStepTime) {
        state.nextStepTime = millis() + stepIntervalMs;
        
        // Advance to next step if not yet complete
        if (state.currentStep < totalSteps) {
            state.currentStep++;
        }
        // Note: Unlike sunset, sunrise doesn't auto-stop when complete
        // The effect stays at full brightness until manually changed
    }
    
    // Return 0 for smooth continuous updates without artificial delays
    return 0;
}

/**
 * @brief Calculate color value for a specific step in sunrise progression
 * 
 * This function implements a sophisticated color progression algorithm that
 * simulates natural sunrise colors using bezier curve interpolation between
 * predefined color control points.
 * 
 * The progression has several phases:
 * - Phase 1 (0 to SRSS_MidValue): Dark to warm orange/red
 * - Phase 2 (SRSS_MidValue to SRSS_Endvalue): Orange/red to bright white
 * 
 * Each phase uses quadratic bezier interpolation for smooth color transitions.
 * 
 * @param step Current step in progression (0 = dark, DEFAULT_SUNRISE_STEPS = bright)
 * @return CRGB color value for this step
 */
CRGB SunriseEffect::calculateSunriseColor(uint16_t step) const {
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
 * @brief Draw current sunrise step to LED strip
 * 
 * This function renders the current sunrise step with several enhancements:
 * 1. Base color from calculateSunriseColor()
 * 2. Dithering for smoother transitions between steps
 * 3. Subtle noise variations for realistic appearance
 * 
 * The dithering alternates between the current step and next step color
 * to create smoother visual transitions. The noise effect simulates the
 * natural variations in sunlight intensity.
 * 
 * @param strip Pointer to WS2812FX instance
 */
void SunriseEffect::drawSunriseStep(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate effective step with dithering for smoother transitions
    uint16_t effectiveStep = state.currentStep;
    if (state.alternateToggle) {
        effectiveStep += 1;  // Alternate between current and next step
    }
    state.alternateToggle = !state.alternateToggle;  // Toggle for next frame
    
    // Get base color for this step
    CRGB baseColor = calculateSunriseColor(effectiveStep);
    
    // Fill entire strip with base color
    fill_solid(&strip->leds[runtime->start], runtime->length, baseColor);
    
    // Update noise values periodically for realistic variations
    updateNoiseValues();
    
    // Apply noise effect to create subtle brightness variations
    // This simulates the natural flickering and variations in real sunlight
    uint16_t maxNoiseIndex = (uint16_t)(sizeof(state.noiseValues) - 1);
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Get noise value for this LED (ensure we don't exceed array bounds)
        uint8_t noiseIndex = min(i, maxNoiseIndex);
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
 * This creates a more realistic sunrise appearance by simulating
 * the natural variations in sunlight intensity.
 */
void SunriseEffect::updateNoiseValues() {
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
 * @brief Get the name of this effect
 * @return Flash string containing "Sunrise"
 */
const __FlashStringHelper* SunriseEffect::getName() const {
    return F("Sunrise");
}

/**
 * @brief Get the mode ID for this effect
 * @return FX_MODE_SUNRISE
 */
uint8_t SunriseEffect::getModeId() const {
    return FX_MODE_SUNRISE;
}

/**
 * @brief Clean up when effect is being deactivated
 * 
 * Performs any necessary cleanup when switching away from this effect.
 * For sunrise effect, no special cleanup is needed as all state is
 * contained within the effect instance.
 */
void SunriseEffect::cleanup() {
    // No special cleanup required for sunrise effect
    // All state is self-contained within this instance
}

// Register this effect with the factory system
REGISTER_EFFECT(FX_MODE_SUNRISE, SunriseEffect)