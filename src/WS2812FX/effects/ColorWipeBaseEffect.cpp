#include "ColorWipeBaseEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool ColorWipeBaseEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from EffectHelper
    bool initResult = EffectHelper::standardInit(strip, timebase, initialized);
    
    // Initialize effect-specific state
    currentColorIndex = EffectHelper::get_random_wheel_index(0, 32);
    previousColorIndex = EffectHelper::get_random_wheel_index(currentColorIndex, 32);
    previousWavePosition = 0;
    needNewColor = true;
    isMovingUp = true;
    
    return initResult;
}

void ColorWipeBaseEffect::updateColorIndices(WS2812FX* strip) {
    if (needNewColor) {
        previousColorIndex = currentColorIndex;
        currentColorIndex = EffectHelper::get_random_wheel_index(currentColorIndex, 32);
        needNewColor = false;
    }
}

void ColorWipeBaseEffect::fillWipeColors(WS2812FX* strip, uint16_t fractionalPos16) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) return;
    
    // Calculate segment length for boundary checking
    uint16_t segmentLength = runtime->stop - runtime->start;
    if (segmentLength == 0) return;
    
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
    
    // Calculate the wipe bar width (minimum 1 pixel, maximum 3 pixels for smooth transition)
    uint16_t barWidth = min(max(segmentLength / 8, 1), 3);
    
    // Convert segment coordinates to 16-bit fractional format
    uint16_t segmentStart16 = runtime->start * 16;
    uint16_t segmentEnd16 = runtime->stop * 16;
    
    // Map the wave position to segment coordinates, accounting for bar width
    uint16_t mappedPos16 = map(fractionalPos16, 0, 65535, segmentStart16, segmentEnd16 - barWidth * 16);
    
    // Ensure the position stays within segment bounds
    mappedPos16 = constrain(mappedPos16, segmentStart16, segmentEnd16 - barWidth * 16);
    
    // Get the center pixel position for the wipe boundary
    uint16_t centerPixel = mappedPos16 / 16; // Convert back to pixel index
    centerPixel = constrain(centerPixel, runtime->start, runtime->stop - 1);
    
    // Fill the wipe effect: color1 from start up to center, color2 from center to end
    if (centerPixel > runtime->start) {
        fill_solid(&strip->leds[runtime->start], centerPixel - runtime->start, color1);
    }
    if (centerPixel < runtime->stop) {
        fill_solid(&strip->leds[centerPixel], runtime->stop - centerPixel, color2);
    }
    
    // Draw the transitional bar at the wipe boundary for smooth anti-aliased effect
    strip->drawFractionalBar(mappedPos16, barWidth, *strip->getCurrentPalette(), 
                           currentColorIndex + runtime->baseHue, seg->targetBrightness, true, 1);
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
    // This returns a value in range 0-65535
    uint16_t wavePosition = calculateWipePosition(strip, timebase);
    
    // Check if direction actually changed 
    if (((wavePosition > previousWavePosition) && !isMovingUp) || 
        ((wavePosition < previousWavePosition) && isMovingUp)) {
        isMovingUp = !isMovingUp;
        needNewColor = true;
    }
    
    // Update color indices if needed
    updateColorIndices(strip);
    
    // Fill the strip with the appropriate colors using fractional positioning
    fillWipeColors(strip, wavePosition);
    
    // Store current wave position for next iteration  
    previousWavePosition = wavePosition;
    
    return strip->getStripMinDelay();
}