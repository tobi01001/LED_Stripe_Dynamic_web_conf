#include "TwinkleMapEffect.h"
#include "../WS2812FX_FastLed.h"
#include <new>  // For std::nothrow

bool TwinkleMapEffect::init(WS2812FX* strip) {
    // Ensure pixel state array is allocated for current strip length
    if (!ensureStateArrayAllocated(strip)) {
        return false; // Memory allocation failed
    }

    // Access the segment runtime to get strip configuration
    auto runtime = strip->getSegmentRuntime();
    
    // Initialize all pixel states to 0 (base color, not twinkling)
    memset(_pixelStates, 0, runtime->length * sizeof(uint8_t));
    
    // Set all LEDs to their base colors using palette mapping
    for (uint16_t i = 0; i < runtime->length; i++) {
        CRGB baseColor = calculateBaseColor(strip, i);
        strip->leds[runtime->start + i] = baseColor;
    }
    
    // Mark initialization as complete
    runtime->modeinit = false;
    
    return true;
}

uint16_t TwinkleMapEffect::update(WS2812FX* strip) {
    // Get segment configuration and runtime data
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Ensure state array is still valid (handle potential strip length changes)
    if (!ensureStateArrayAllocated(strip)) {
        return strip->getStripMinDelay(); // Return safe delay on allocation failure
    }
    
    // Calculate speed parameters based on beat88 setting
    // Map beat88 range to speed values (4-64), matching original implementation
    uint16_t beat88_val = seg->beat88;
    uint8_t speedUp = ((uint8_t)((beat88_val - BEAT88_MIN) * (64 - 4)) / (BEAT88_MAX - BEAT88_MIN) + 4);
    uint8_t speedDown = speedUp / 2; // Dimming is slower than brightening
    
    // Process each LED in the segment
    for (uint16_t i = 0; i < runtime->length; i++) {
        uint16_t absoluteIndex = runtime->start + i;
        uint8_t& pixelState = _pixelStates[i];
        
        // Calculate colors for this LED position
        CRGB baseColor = calculateBaseColor(strip, i);
        CRGB peakColor = calculatePeakColor(strip, i);
        
        // State machine for each LED:
        if (pixelState == 0) {
            // LED is at base color - randomly decide whether to start twinkling
            // Use twinkleDensity setting with same logic as original (avoid values <3)
            uint8_t twinkleThreshold = (seg->twinkleDensity < 3) ? 1 : (seg->twinkleDensity - 2);
            if (random8() < twinkleThreshold) {
                pixelState = 1; // Start brightening phase
            }
            // LED stays at base color if no twinkle started
            strip->leds[absoluteIndex] = baseColor;
        }
        else if ((pixelState & 0x01) == 0x01) {
            // Odd state: LED is in brightening phase
            // Blend between base and peak colors based on current state
            strip->leds[absoluteIndex] = blend(baseColor, peakColor, pixelState);
            
            if (pixelState == 255) {
                // Reached maximum brightness - switch to dimming phase
                pixelState = 2;
            } else {
                // Continue brightening - increment state with speed control
                pixelState = 0x01 | strip->qadd8_lim(pixelState, speedUp, 255);
            }
        }
        else if ((pixelState & 0x01) == 0x00) {
            // Even state (and not 0): LED is in dimming phase
            if (pixelState == 254) {
                // Reached minimum brightness - return to base color
                strip->leds[absoluteIndex] = baseColor; // Ensure exact base color
                pixelState = 0; // Return to idle state
            } else {
                // Continue dimming - increment state with speed control
                // Use 0xFE mask to keep even values and avoid bit 0 being set
                pixelState = 0xFE & strip->qadd8_lim(pixelState, speedDown, 254);
                
                // Blend from peak to base colors based on current state
                strip->leds[absoluteIndex] = blend(peakColor, baseColor, pixelState);
            }
        }
    }
    
    // Calculate return delay based on beat88 setting (matches original implementation)
    // Higher beat88 values result in faster updates (shorter delays)
    uint32_t minDelay = strip->getStripMinDelay();
    uint32_t calculatedDelay = (uint32_t)(BEAT88_MAX - seg->beat88) / 1800;
    uint32_t delay = (minDelay > calculatedDelay) ? minDelay : calculatedDelay;
    
    return (uint16_t)delay;
}

const __FlashStringHelper* TwinkleMapEffect::getName() const {
    return F("Twinkle Base Color");
}

uint8_t TwinkleMapEffect::getModeId() const {
    return FX_MODE_TWINKLE_MAP;
}

void TwinkleMapEffect::cleanup() {
    if (_pixelStates != nullptr) {
        delete[] _pixelStates;
        _pixelStates = nullptr;
    }
    _allocatedLength = 0;
}

bool TwinkleMapEffect::ensureStateArrayAllocated(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Check if we need to allocate or reallocate
    if (_allocatedLength != runtime->length) {
        // Clean up existing allocation
        cleanup();
        
        // Allocate new array for current strip length
        _pixelStates = new (std::nothrow) uint8_t[runtime->length];
        if (_pixelStates == nullptr) {
            // Memory allocation failed
            _allocatedLength = 0;
            return false;
        }
        
        // Initialize new array to base state
        memset(_pixelStates, 0, runtime->length * sizeof(uint8_t));
        _allocatedLength = runtime->length;
    }
    
    return true;
}

CRGB TwinkleMapEffect::calculateBaseColor(WS2812FX* strip, uint16_t ledIndex) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Map LED position to palette index (same algorithm as original)
    uint8_t paletteIndex = (uint8_t)((ledIndex * 255) / runtime->length);
    
    // Get color from current palette with hue rotation and blend type
    CRGB color = strip->ColorFromPaletteWithDistribution(
        *(strip->getCurrentPalette()),
        paletteIndex + runtime->baseHue,  // Add base hue rotation
        255,                              // Full saturation
        seg->blendType                    // Segment blend type setting
    );
    
    // Scale to base brightness level (32/255 as in original)
    return color.nscale8_video(32);
}

CRGB TwinkleMapEffect::calculatePeakColor(WS2812FX* strip, uint16_t ledIndex) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Map LED position to palette index (same algorithm as original)
    uint8_t paletteIndex = (uint8_t)((ledIndex * 255) / runtime->length);
    
    // Get color from current palette with hue rotation and blend type
    CRGB color = strip->ColorFromPaletteWithDistribution(
        *(strip->getCurrentPalette()),
        paletteIndex + runtime->baseHue,  // Add base hue rotation
        255,                              // Full saturation
        seg->blendType                    // Segment blend type setting
    );
    
    // Add RGB enhancement for sparkle effect (+4 as in original)
    return color.addToRGB(4);
}

// Register this effect with the factory system
// This allows the effect to be created dynamically when FX_MODE_TWINKLE_MAP is selected
REGISTER_EFFECT(FX_MODE_TWINKLE_MAP, TwinkleMapEffect)