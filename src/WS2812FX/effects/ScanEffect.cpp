#include "ScanEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool ScanEffect::init(WS2812FX* strip) {
    return standardInit(strip);
}

uint16_t ScanEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000;
    }
    
    // Get access to runtime data
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return strip->getStripMinDelay();
    }
    
    // Calculate the current position of the scanning bar using helper
    uint16_t trianglePosition = EffectHelper::calculateTrianglePosition(strip, millis());
    uint16_t ledOffset = calculateBarPosition(trianglePosition, runtime);
    
    // Clear the entire segment to black using helper
    EffectHelper::clearSegment(strip);
    
    // Calculate color index based on position for dynamic color changing
    uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, ledOffset, 0);
    
    // Draw the scanning bar using helper
    EffectHelper::drawBar(strip, ledOffset, BAR_WIDTH, colorIndex, 255);
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* ScanEffect::getName() const {
    return F("Scan");
}

uint8_t ScanEffect::getModeId() const {
    return FX_MODE_SCAN;
}

// Helper method implementations
uint16_t ScanEffect::calculateBarPosition(uint16_t trianglePosition, const void* runtime_ptr) const {
    // Cast runtime pointer back to proper type
    const auto* runtime = static_cast<const WS2812FX::segment_runtime*>(runtime_ptr);
    
    // Map the triangular wave to the segment boundaries
    // We map to 16-bit fractional positions for sub-pixel accuracy
    // The range is adjusted to account for the bar width to prevent overflow
    uint16_t segmentLength = runtime->length * 16;
    uint16_t maxPosition = segmentLength - BAR_WIDTH * 16;  // Account for bar width
    
    // Map from the full triangle wave range (0-65535) to our segment range
    return EffectHelper::safeMapuint16_t(trianglePosition, 
                               (uint16_t)0, (uint16_t)65535,    // Input range
                               (uint16_t)0, maxPosition);       // Output range
}

// Register this effect with the factory so it can be created by mode ID
REGISTER_EFFECT(FX_MODE_SCAN, ScanEffect)