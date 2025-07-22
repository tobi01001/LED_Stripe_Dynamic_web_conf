#include "DualScanEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool DualScanEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false;
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t DualScanEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Get access to runtime data
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the current positions of both scanning bars using helper
    uint16_t forwardBarOffset = calculateForwardBarPosition(strip);
    uint16_t reverseBarOffset = calculateReverseBarPosition(strip);
    
    // Clear the segment using helper
    EffectHelper::clearSegment(strip);
    
    // Calculate color indices for both bars using helper
    uint8_t forwardColorIndex = EffectHelper::calculateColorIndex(strip, forwardBarOffset, runtime->baseHue);
    uint8_t reverseColorIndex = EffectHelper::calculateColorIndex(strip, 255 - reverseBarOffset, runtime->baseHue);
    
    // Draw the forward-moving bar using helper
    EffectHelper::drawBar(strip, forwardBarOffset, BAR_WIDTH, forwardColorIndex);
    
    // Draw the reverse-moving bar using helper
    uint16_t reversePosition = (runtime->length - 1) * 16 - reverseBarOffset;
    EffectHelper::drawBar(strip, reversePosition, BAR_WIDTH, reverseColorIndex);
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* DualScanEffect::getName() const {
    return F("Dual Scan");
}

uint8_t DualScanEffect::getModeId() const {
    return FX_MODE_DUAL_SCAN;
}

// Helper method implementations
uint16_t DualScanEffect::calculateForwardBarPosition(WS2812FX* strip) const {
    // Calculate triangular position using helper
    uint16_t triangularPosition = EffectHelper::calculateTrianglePosition(strip, timebase);
    
    // Get runtime to access segment boundaries
    auto runtime = strip->getSegmentRuntime();
    
    // Map the triangular wave to the segment boundaries using helper
    uint16_t maxPosition = (runtime->length - BAR_WIDTH) * 16;
    return EffectHelper::safeMapuint16_t(triangularPosition, 0, 65535, 0, maxPosition);
}

uint16_t DualScanEffect::calculateReverseBarPosition(WS2812FX* strip) const {
    // Use the same calculation as forward bar for synchronized movement
    return calculateForwardBarPosition(strip);
}

// Register this effect with the factory so it can be created by mode ID
REGISTER_EFFECT(FX_MODE_DUAL_SCAN, DualScanEffect)