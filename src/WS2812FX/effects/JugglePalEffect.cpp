#include "JugglePalEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool JugglePalEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize effect-specific state
    currentHue = 0;
    lastHueChange = millis();
    
    return true;
}

uint16_t JugglePalEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate bar width based on segment length, with minimum width
    uint8_t barWidth = max((uint8_t)(runtime->length / 15), MIN_BAR_WIDTH);
    
    // Handle periodic hue changes for color variation
    uint32_t currentTime = millis();
    if (currentTime - lastHueChange >= HUE_CHANGE_INTERVAL) {
        // Create subtle random variation in hue
        uint8_t previousHue = currentHue;
        currentHue = random8(previousHue, qadd8(previousHue, MAX_HUE_DELTA));
        lastHueChange = currentTime;
    }
    
    // Apply background fade to create trailing effect
    EffectHelper::applyFadeEffect(strip, FADE_AMOUNT);
    
    // Create multiple moving bars based on numBars setting
    uint8_t numBars = seg->numBars;
    if (numBars == 0) numBars = 3; // Default to 3 bars if not set
    
    uint8_t workingHue = currentHue; // Local copy for per-bar variations
    
    for (int i = 0; i < numBars; i++) {
        // Calculate speed for this bar - each bar moves at a different speed
        // Base speed is beat88/2, with per-bar variation
        uint16_t barSpeed = max(seg->beat88 / 2, 1) + i * (seg->beat88 / numBars);
        
        // Calculate position using beatsin88 for smooth sinusoidal movement
        // Each bar oscillates between segment start and end, accounting for bar width
        uint16_t position = beatsin88(barSpeed, 
                                     runtime->start * 16, 
                                     runtime->stop * 16 - barWidth * 16, 
                                     timebase);
        
        // Calculate color index for this bar
        // Distribute colors evenly across the color spectrum based on bar number
        uint8_t colorIndex = workingHue + (255 / numBars) * i;
        
        // Draw the bar using fractional positioning for smooth movement
        strip->drawFractionalBar(position, barWidth, *strip->getCurrentPalette(), 
                               colorIndex, 255, true, 1);
        
        // Apply random color variation for next bar
        // This creates subtle color variations between bars
        uint8_t deltaHue = random8(9); // Random variation 0-8
        if (deltaHue < 5) {
            // Subtract hue variation (with base hue offset)
            workingHue = workingHue - deltaHue + runtime->baseHue;
        } else {
            // Add hue variation (with base hue offset)
            workingHue = workingHue + (deltaHue / 2) + runtime->baseHue;
        }
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* JugglePalEffect::getName() const {
    return F("Juggle Pixels");
}

uint8_t JugglePalEffect::getModeId() const {
    return FX_MODE_JUGGLE_PAL;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_JUGGLE_PAL, JugglePalEffect)