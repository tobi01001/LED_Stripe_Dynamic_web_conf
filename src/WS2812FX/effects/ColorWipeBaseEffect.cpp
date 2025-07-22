#include "ColorWipeBaseEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool ColorWipeBaseEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from EffectHelper
    bool initResult = EffectHelper::standardInit(strip, timebase, initialized);
    
    // Initialize effect-specific state
    currentColorIndex = strip->get_random_wheel_index(0, 32);
    previousColorIndex = strip->get_random_wheel_index(currentColorIndex, 32);
    previousPosition = 0;
    needNewColor = true;
    isMovingUp = true;
    
    return initResult;
}

void ColorWipeBaseEffect::updateColorIndices(WS2812FX* strip) {
    if (needNewColor) {
        previousColorIndex = currentColorIndex;
        currentColorIndex = strip->get_random_wheel_index(currentColorIndex, 32);
        needNewColor = false;
    }
}

void ColorWipeBaseEffect::fillWipeColors(WS2812FX* strip, uint16_t position) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) return;
    
    // Get colors from palette with current indices
    CRGB color1 = strip->ColorFromPaletteWithDistribution(
        *strip->getCurrentPalette(), 
        currentColorIndex + runtime->baseHue, 
        seg->targetBrightness, 
        seg->blendType);
        
    CRGB color2 = strip->ColorFromPaletteWithDistribution(
        *strip->getCurrentPalette(), 
        previousColorIndex + runtime->baseHue, 
        seg->targetBrightness, 
        seg->blendType);
    
    // Fill the strip based on direction and position
    if (isMovingUp) {
        // Moving up: fill from start to position with color1, rest with color2
        if (position > runtime->start) {
            uint16_t fillLength = position - runtime->start;
            fill_solid(&strip->leds[runtime->start], fillLength, color1);
        }
        if (position < runtime->stop) {
            uint16_t fillLength = runtime->stop - position;
            fill_solid(&strip->leds[position], fillLength, color2);
        }
    } else {
        // Moving down: fill from position to end with color1, start to position with color2
        if (position < runtime->stop) {
            uint16_t fillLength = runtime->stop - position;
            fill_solid(&strip->leds[position], fillLength, color1);
        }
        if (position > runtime->start) {
            uint16_t fillLength = position - runtime->start;
            fill_solid(&strip->leds[runtime->start], fillLength, color2);
        }
    }
}

uint16_t ColorWipeBaseEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000;
    }
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate wipe position using subclass-specific method
    uint16_t wavePosition = calculateWipePosition(strip, timebase);
    
    // Map wave position to LED strip coordinates
    uint16_t ledPosition = map(wavePosition, 
                              (uint16_t)0, (uint16_t)65535,
                              (uint16_t)runtime->start, 
                              (uint16_t)(runtime->stop + 2));
    
    // Clamp position to valid range
    if (ledPosition >= runtime->stop) {
        ledPosition = runtime->stop;
    }
    
    // Detect direction changes
    if ((isMovingUp && ledPosition < previousPosition) || 
        (!isMovingUp && ledPosition > previousPosition)) {
        // Direction changed - trigger color change
        isMovingUp = !isMovingUp;
        needNewColor = true;
    }
    
    // Update color indices if needed
    updateColorIndices(strip);
    
    // Fill the strip with the appropriate colors
    fillWipeColors(strip, ledPosition);
    
    // Store current position for next iteration
    previousPosition = ledPosition;
    
    return strip->getStripMinDelay();
}