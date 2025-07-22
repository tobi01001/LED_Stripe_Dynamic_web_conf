#include "EffectHelper.h"
#include "WS2812FX_FastLed.h"

// ===== INITIALIZATION HELPERS =====

bool EffectHelper::standardInit(WS2812FX* strip, uint32_t& timebase, bool& initialized) {
    if (!validateStripPointer(strip)) {
        return false;
    }
    
    if (initialized) {
        return true; // Already initialized
    }
    
    // Set timebase for consistent animation timing
    timebase = millis();
    
    // Get runtime data and mark as initialized
    auto runtime = strip->getSegmentRuntime();
    if (runtime) {
        runtime->modeinit = false;
    }
    
    initialized = true;
    return true;
}

bool EffectHelper::validateStripPointer(WS2812FX* strip) {
    return (strip != nullptr);
}

// ===== TIMING AND ANIMATION HELPERS =====

uint16_t EffectHelper::calculateBeatPosition(WS2812FX* strip, uint32_t timebase, uint8_t speedMultiplier) {
    if (!validateStripPointer(strip)) {
        return 0;
    }
    
    auto seg = strip->getSegment();
    if (!seg) {
        return 0;
    }
    
    return beat88(seg->beat88 * speedMultiplier, timebase);
}

uint8_t EffectHelper::generateTriangleWave(uint16_t beatPosition, uint8_t minBrightness, uint8_t maxBrightness) {
    // Convert beat position (0-65535) to 8-bit range for triwave8
    uint8_t wavePosition = map(beatPosition, 
                              (uint16_t)0, (uint16_t)65535,
                              (uint8_t)0, (uint8_t)255);
    
    // Generate triangular wave for smooth fade in/out
    uint8_t triangleBrightness = triwave8(wavePosition);
    
    // Map to desired brightness range
    return map8(triangleBrightness, minBrightness, maxBrightness);
}

uint16_t EffectHelper::mapPositionToStrip(WS2812FX* strip, uint16_t beatPosition, uint8_t speedMultiplier) {
    if (!validateStripPointer(strip)) {
        return 0;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return 0;
    }
    
    // Map beat position to strip length with 16-bit precision for sub-pixel accuracy
    return safeMapuint16_t(beatPosition, 
                   (uint16_t)0, (uint16_t)65535,
                   (uint16_t)0, (uint16_t)(runtime->length * 16));
}

// ===== COLOR AND PALETTE HELPERS =====

uint8_t EffectHelper::calculateColorIndex(WS2812FX* strip, uint16_t position, uint8_t hueOffset) {
    if (!validateStripPointer(strip)) {
        return 0;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return 0;
    }
    
    // Calculate position-based color index with base hue offset
    uint8_t colorIndex = map(position,
                            (uint16_t)0, (uint16_t)(runtime->length * 16),
                            (uint16_t)0, (uint16_t)255);
    
    return colorIndex + runtime->baseHue + hueOffset;
}

uint16_t EffectHelper::calculateTrianglePosition(WS2812FX* strip, uint32_t timebase, uint8_t speedMultiplier) {
    if (!validateStripPointer(strip)) {
        return 0;
    }
    
    auto seg = strip->getSegment();
    if (!seg) {
        return 0;
    }
    
    // Get beat value for consistent timing
    uint16_t beatValue = beat88(seg->beat88 * speedMultiplier, timebase);
    
    // Apply triangular wave for smooth back-and-forth motion
    // Implement triwave16 manually: if high bit set, invert and shift
    if (beatValue & 0x8000) {
        beatValue = 65535 - beatValue;
    }
    return beatValue << 1;  // Double the value but it will wrap at 16-bit boundary
}

void EffectHelper::clearSegment(WS2812FX* strip) {
    if (!validateStripPointer(strip)) {
        return;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return;
    }
    
    // Clear entire segment to black
    fill_solid(&strip->leds[runtime->start], runtime->length, CRGB::Black);
}

void EffectHelper::drawBar(WS2812FX* strip, uint16_t relativePosition, uint16_t width, uint8_t colorIndex, uint8_t brightness) {
    if (!validateStripPointer(strip)) {
        return;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return;
    }
    
    // Calculate absolute position
    uint16_t absolutePosition = runtime->start * 16 + relativePosition;
    
    // Draw the bar using fractional positioning
    strip->drawFractionalBar(absolutePosition, width, 
                           *strip->getCurrentPalette(), colorIndex, 
                           brightness, true, 1);
}

// ===== SPECIAL EFFECT UTILITIES =====

uint8_t EffectHelper::attackDecayWave8(uint8_t phase) {
    /**
     * Create a natural-looking brightness curve with sharp attack and slow decay.
     * This mimics how real fireflies and stars appear to twinkle.
     * 
     * Phase 0-85: Attack phase - rapid brightness increase (linear ramp up)
     * Phase 86-255: Decay phase - slower brightness decrease (non-linear decay)
     */
    if (phase < 86) {
        // Attack phase: linear ramp from 0 to 255 over first 86 steps
        return phase * 3;  // 86 * 3 = 258, clamped to 255
    } else {
        // Decay phase: slower non-linear decay from 255 to 0
        phase -= 86;  // Normalize to 0-169 range
        return 255 - (phase + (phase / 2));  // Non-linear decay
    }
}

void EffectHelper::applyFadeEffect(WS2812FX* strip, uint8_t fadeAmount) {
    if (!validateStripPointer(strip)) {
        return;
    }
    
    strip->fade_out(fadeAmount);
}

void EffectHelper::fillPaletteWithBrightness(WS2812FX* strip, uint8_t brightness, uint8_t hueDelta) {
    if (!validateStripPointer(strip)) {
        return;
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return;
    }
    
    // Fill the segment with colors from current palette
    fill_palette(&strip->leds[runtime->start], 
                runtime->length,
                runtime->baseHue,           // Starting hue position
                hueDelta,                   // Hue increment between LEDs
                *strip->getCurrentPalette(), // Current color palette
                brightness,                 // Overall brightness
                seg->blendType);            // Blending mode
}

// ===== MEMORY MANAGEMENT HELPERS =====

void* EffectHelper::safeAllocateArray(void* currentArray, size_t& currentSize, size_t requiredSize, size_t elementSize) {
    // If we already have the right size, return current array
    if (currentArray != nullptr && currentSize == requiredSize) {
        return currentArray;
    }
    
    // Free existing array if size is wrong
    if (currentArray != nullptr && currentSize != requiredSize) {
        free(currentArray);
        currentArray = nullptr;
        currentSize = 0;
    }
    
    // Allocate new array if needed
    if (currentArray == nullptr) {
        size_t totalBytes = requiredSize * elementSize;
        currentArray = malloc(totalBytes);
        
        if (currentArray != nullptr) {
            currentSize = requiredSize;
            // Initialize memory to zero
            memset(currentArray, 0, totalBytes);
        } else {
            currentSize = 0;
        }
    }
    
    return currentArray;
}

void EffectHelper::safeFreeArray(void*& array, size_t& size) {
    if (array != nullptr) {
        free(array);
        array = nullptr;
        size = 0;
    }
}

// ===== MATHEMATICAL UTILITIES =====

uint16_t EffectHelper::safeMapuint16_t(uint16_t value, uint16_t fromMin, uint16_t fromMax, uint16_t toMin, uint16_t toMax) {
    // Handle edge cases
    if (fromMin == fromMax) {
        return toMin;
    }
    
    if (value <= fromMin) {
        return toMin;
    }
    
    if (value >= fromMax) {
        return toMax;
    }
    
    // Perform mapping with overflow protection
    uint32_t result = (uint32_t)(value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
    
    // Clamp to output range
    if (result > toMax) {
        return toMax;
    }
    
    return (uint16_t)result;
}

double EffectHelper::safeMapdouble(double value, double fromMin, double fromMax, double toMin, double toMax) {
    // Handle edge cases
    if (fromMin == fromMax) {
        return toMin;
    }
    
    if (value <= fromMin) {
        return toMin;
    }
    
    if (value >= fromMax) {
        return toMax;
    }
    
    // Perform mapping with overflow protection
    double result = (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
    
    // Clamp to output range
    if (result > toMax) {
        return toMax;
    }
    
    return result;
}

uint8_t EffectHelper::linearInterpolate(uint8_t a, uint8_t b, float fraction) {
    // Clamp fraction to valid range
    if (fraction <= 0.0f) {
        return a;
    }
    if (fraction >= 1.0f) {
        return b;
    }
    
    // Linear interpolation
    return (uint8_t)(a + fraction * (b - a));
}

uint16_t EffectHelper::calculateProportionalWidth(WS2812FX* strip, uint16_t divisor, uint16_t minimum) {
    if (!validateStripPointer(strip) || divisor == 0) {
        return minimum;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return minimum;
    }
    
    uint16_t calculatedWidth = runtime->length / divisor;
    return max(minimum, calculatedWidth);
}