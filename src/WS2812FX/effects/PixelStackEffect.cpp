#include "PixelStackEffect.h"
#include "../WS2812FX_FastLed.h"

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_PIXEL_STACK, PixelStackEffect)

bool PixelStackEffect::init(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Initialize effect state variables to starting conditions
    initializeEffectState();
    
    // Mark initialization as complete
    runtime->modeinit = false;
    
    return true;
}

uint16_t PixelStackEffect::update(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Check if we need to reinitialize
    if (runtime->modeinit) {
        return init(strip) ? strip->getStripMinDelay() : strip->getStripMinDelay();
    }
    
    // Calculate effect parameters
    const uint16_t effectSpeed = calculateEffectSpeed(strip);
    const uint16_t nLeds = runtime->length / 2;  // Half the strip length for the effect
    const uint8_t baseHue = runtime->baseHue;    // Base hue for color calculations
    
    // Apply background fade effect for smooth transitions
    applyBackgroundFade(strip);
    
    // Render the static LEDs that are already in their final positions
    renderStackedLEDs(strip, nLeds, baseHue);
    
    // Get current beat position for movement calculations
    uint16_t beatPosition = beat88(effectSpeed);
    
    // Handle movement based on current direction
    if (up) {
        handleUpwardMovement(strip, nLeds, baseHue, beatPosition);
    } else {
        handleDownwardMovement(strip, nLeds, baseHue, beatPosition);
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* PixelStackEffect::getName() const {
    return F("Pixel Stack");
}

uint8_t PixelStackEffect::getModeId() const {
    return FX_MODE_PIXEL_STACK;
}

void PixelStackEffect::initializeEffectState() {
    // Start with upward movement
    up = true;
    
    // No LEDs have been moved yet
    leds_moved = 0;
    
    // Reset position tracking
    ppos16 = 0;
}

void PixelStackEffect::applyBackgroundFade(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    const uint16_t effectSpeed = calculateEffectSpeed(strip);
    
    // Apply speed-dependent fade to create smooth trails
    // Faster speeds get more fading to prevent blur
    const uint8_t MIN_FADE_AMOUNT = 2;
    uint8_t fadeAmount = max(MIN_FADE_AMOUNT, (uint8_t)(effectSpeed >> 8));
    
    fadeToBlackBy(strip->leds + runtime->start, runtime->length, fadeAmount);
}

void PixelStackEffect::renderStackedLEDs(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue) {
    auto runtime = strip->getSegmentRuntime();
    
    // Render the lower half (static LEDs that haven't moved yet)
    for (uint16_t i = 0; i < nLeds; i++) {
        if (i < nLeds - leds_moved) {
            // Calculate color for this position in the stack
            CRGB color = getStackColor(strip, i, nLeds, baseHue);
            strip->leds[runtime->start + i] = color;
        }
    }
    
    // Render the upper half (LEDs that have been moved to their final position)
    for (uint16_t i = 0; i < leds_moved; i++) {
        // Calculate color for this position (mapped from the top)
        CRGB color = getStackColor(strip, nLeds - i - 1, nLeds, baseHue);
        strip->leds[runtime->start + runtime->length - 1 - i] = color;
    }
}

void PixelStackEffect::handleUpwardMovement(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue, uint16_t beatPosition) {
    auto runtime = strip->getSegmentRuntime();
    
    // Map beat position to movement range (from current position to top)
    uint16_t startPos = 16 * (nLeds - leds_moved);
    uint16_t endPos = 16 * (runtime->length - 1 - leds_moved) - 16;
    uint16_t pos16 = map(beatPosition, 0, 65535, startPos, endPos);
    
    // Check if we've completed a cycle (sawtooth wave reset)
    if (ppos16 > pos16) {
        // Reset position tracking
        ppos16 = 0;
        
        // Check if we've moved all LEDs to the top
        if (leds_moved == nLeds) {
            // Switch to downward movement
            leds_moved--;
            up = false;
            ppos16 = 65535;
            return;
        }
        
        // Move to the next LED
        leds_moved++;
    } else {
        // Draw the currently moving LED using fractional positioning
        if (pos16 > FRACTIONAL_POSITION_UNIT) {
            pos16 -= FRACTIONAL_POSITION_UNIT; // Start within an active LED to avoid flicker
        }
        
        // Calculate color for the moving LED
        uint8_t colorIndex = baseHue + map(nLeds - leds_moved, 0, nLeds - 1, 0, 255);
        
        // Draw the moving LED with fractional positioning
        strip->drawFractionalBar(runtime->start * FRACTIONAL_POSITION_UNIT + pos16, 2, 
                                *strip->getCurrentPalette(), 
                                colorIndex, 255, true, 1);
        
        // Update position tracking
        ppos16 = pos16;
    }
}

void PixelStackEffect::handleDownwardMovement(WS2812FX* strip, uint16_t nLeds, uint8_t baseHue, uint16_t beatPosition) {
    auto runtime = strip->getSegmentRuntime();
    
    // Map beat position to movement range (from top back to bottom)
    uint16_t startPos = 16 * (runtime->length - 1 - leds_moved) - 16;
    uint16_t endPos = 16 * (nLeds - leds_moved);
    uint16_t pos16 = map(beatPosition, 0, 65535, startPos, endPos);
    
    // Check if we've completed a cycle (sawtooth wave reset)
    if (ppos16 < pos16) {
        // Reset position tracking
        ppos16 = 65535;
        
        // Check if we've moved all LEDs back to the bottom
        if (leds_moved == 0) {
            // Switch back to upward movement
            leds_moved++;
            up = true;
            ppos16 = 0;
            return;
        }
        
        // Move to the next LED
        leds_moved--;
    } else {
        // Draw the currently moving LED using fractional positioning
        
        // Calculate color for the moving LED
        uint8_t colorIndex = baseHue + map(nLeds - leds_moved, 0, nLeds - 1, 0, 255);
        
        // Draw the moving LED with fractional positioning
        strip->drawFractionalBar(runtime->start * PIXEL_SCALING_FACTOR + pos16, FRACTIONAL_BAR_WIDTH, 
                                *strip->getCurrentPalette(), 
                                colorIndex, 255, true, 1);
        
        // Update position tracking
        ppos16 = pos16;
    }
}

uint16_t PixelStackEffect::calculateEffectSpeed(WS2812FX* strip) const {
    auto seg = strip->getSegment();
    
    // Convert beat88 to effect speed, taking segment count into account
    // This ensures consistent timing across different segment configurations
    uint16_t maxBeat = 20000 / seg->segments;
    uint16_t clampedBeat = (seg->beat88 > maxBeat) ? maxBeat : seg->beat88;
    
    return map(clampedBeat, 0, maxBeat, 0, 65535);
}

CRGB PixelStackEffect::getStackColor(WS2812FX* strip, uint16_t position, uint16_t nLeds, uint8_t baseHue) const {
    auto seg = strip->getSegment();
    
    // Calculate color index based on position in the stack
    // This creates a gradient effect across the stack
    uint8_t colorIndex = baseHue + map(position, 0, nLeds - 1, 0, 255);
    
    // Get color from current palette with full brightness
    return strip->ColorFromPaletteWithDistribution(*strip->getCurrentPalette(), 
                                                  colorIndex, 
                                                  seg->brightness, 
                                                  seg->blendType);
}