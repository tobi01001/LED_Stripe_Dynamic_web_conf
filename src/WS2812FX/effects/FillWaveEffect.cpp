#include "FillWaveEffect.h"
#include "../WS2812FX_FastLed.h"

bool FillWaveEffect::init(WS2812FX* strip) {
    // Initialize the effect - set timebase for consistent wave animation
    timebase = millis();
    initialized = true;
    
    // Mark as initialized in the segment runtime
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t FillWaveEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Ensure we have a valid timebase (fallback if somehow uninitialized)
    if (!initialized) {
        timebase = millis();
        initialized = true;
    }
    
    // Get current palette from the strip
    CRGBPalette16* currentPalette = strip->getCurrentPalette();
    
    // Calculate hue offset using beatsin88 for smooth wave motion
    // The beat88 value controls the speed of the wave animation
    // Multiplying by 2 creates faster hue cycling for more dynamic waves
    uint8_t hueOffset = runtime->baseHue + (uint8_t)beatsin88(seg->beat88 * 2, 0, 255, timebase);
    
    // Calculate palette distribution delta hue
    // This determines how much the palette is stretched across the LED strip
    // Higher paletteDistribution values create tighter color bands
    uint8_t paletteDistribution = seg->paletteDistribution;
    uint8_t deltaHue = max(255 * 100 / (runtime->length * paletteDistribution) + 1, 1);
    
    // Calculate brightness using beatsin88 for pulsing wave effect
    // The brightness oscillates from dim (brightness/10) to full (255)
    // This creates the "wave" appearance as brightness pulses across the strip
    uint8_t brightness = (uint8_t)beatsin88(
        max(seg->beat88 * 1, 1),        // Speed of brightness oscillation
        seg->targetBrightness / 10,      // Minimum brightness (10% of target)
        255,                            // Maximum brightness
        timebase                        // Use same timebase for synchronized motion
    );
    
    // Fill the LED strip with palette colors using the calculated parameters
    // fill_palette distributes palette colors evenly across the strip length
    fill_palette(
        &strip->leds[runtime->start],   // Start position in LED array
        runtime->length,                // Number of LEDs to fill
        hueOffset,                      // Starting hue offset (animated)
        deltaHue,                       // Hue increment between LEDs
        *currentPalette,                // Current color palette
        brightness,                     // Brightness value (animated)
        seg->blendType                  // Color blending mode
    );
    
    // Return minimum delay for smooth animation
    // This ensures the effect updates as fast as the strip allows
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FillWaveEffect::getName() const {
    return F("FILL Wave");
}

uint8_t FillWaveEffect::getModeId() const {
    return FX_MODE_FILL_WAVE;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FILL_WAVE, FillWaveEffect)