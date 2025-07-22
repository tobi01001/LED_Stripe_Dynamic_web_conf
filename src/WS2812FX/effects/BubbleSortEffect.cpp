#include "BubbleSortEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool BubbleSortEffect::init(WS2812FX* strip) {
    // Clean up any existing memory first
    cleanupMemory();
    
    // Initialize state variables
    movedown = false;
    ci = co = cd = 0;
    initialized = false;
    
    // Get current strip length and allocate memory for hue array using EffectHelper
    auto runtime = strip->getSegmentRuntime();
    strip_length = runtime->length;
    
    if (strip_length > 0) {
        // Use EffectHelper for safe memory allocation
        size_t currentSize = 0;
        hues = (uint8_t*)EffectHelper::safeAllocateArray(nullptr, currentSize, strip_length, sizeof(uint8_t));
        if (hues != nullptr) {
            initialized = true;
            initializeHues(strip);
        }
    }
    
    // Mark initialization as complete
    runtime->modeinit = false;
    
    return initialized;
}

uint16_t BubbleSortEffect::update(WS2812FX* strip) {
    // Ensure effect is properly initialized
    if (!initialized || hues == nullptr) {
        return strip->getStripMinDelay();
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate frame delay using EffectHelper safe mapping
    const uint16_t framedelay = calculateFrameDelay(strip);
    
    // Check if we need to reinitialize (e.g., after transition)
    if (runtime->modeinit) {
        return init(strip) ? strip->getStripMinDelay() : strip->getStripMinDelay();
    }
    
    // Main bubble sort algorithm visualization
    if (!movedown) {
        // Comparison phase - comparing adjacent elements
        if (co < strip_length) {
            if (ci < strip_length - 1) {
                // Compare current element with next element
                if (hues[ci] > hues[ci + 1]) {
                    // Elements are out of order - swap them
                    uint8_t tmp = hues[ci];
                    hues[ci] = hues[ci + 1];
                    hues[ci + 1] = tmp;
                    
                    // Set up movement animation
                    cd = ci;
                    movedown = true;
                }
                ci++;  // Move to next comparison
            } else {
                // Finished this pass - move to next outer loop iteration
                co++;
                ci = 0;  // Reset inner loop counter
            }
        } else {
            // Sorting complete - restart the effect
            initializeHues(strip);
            co = ci = 0;
            return strip->getStripMinDelay();
        }
        
        // Update LED display with current hue values
        updateLEDDisplay(strip);
        
        // Highlight the elements being compared using EffectHelper color calculations
        if (ci < strip_length) {
            uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, ci, hues[ci]);
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), colorIndex, seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + ci] = color;
        }
        
        if (co < strip_length) {
            uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, co, hues[co]);
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), colorIndex, seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + co] = color;
        }
        
    } else {
        // Movement animation phase - showing the swap in progress
        updateLEDDisplay(strip);
        
        // Highlight the elements involved in the swap
        if (co < strip_length) {
            uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, co, hues[co]);
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), colorIndex, seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + co] = color;
        }
        
        if (cd < strip_length) {
            uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, cd, hues[cd]);
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), colorIndex, seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + cd] = color;
        }
        
        // Animation countdown - when reaching the target position, end movement
        if (cd == co) {
            movedown = false;
        } else if (cd > 0) {
            cd--;
        } else {
            movedown = false;
        }
        
        return framedelay;
    }
    
    return framedelay;
}

const __FlashStringHelper* BubbleSortEffect::getName() const {
    return F("Bubble Sort");
}

uint8_t BubbleSortEffect::getModeId() const {
    return FX_MODE_BUBBLE_SORT;
}

void BubbleSortEffect::initializeHues(WS2812FX* strip) {
    if (!initialized || hues == nullptr) {
        return;
    }
    
    // Initialize first hue value randomly
    hues[0] = random8();
    
    // Generate subsequent hue values with some variation from previous values
    // This creates a visually pleasing but unsorted initial state
    for (uint16_t i = 1; i < strip_length; i++) {
        hues[i] = strip->get_random_wheel_index(hues[i - 1], 48);
    }
    
    // Reset algorithm state
    co = ci = 0;
    movedown = false;
}

void BubbleSortEffect::cleanupMemory() {
    if (hues != nullptr) {
        // Use EffectHelper for safe memory cleanup
        size_t arraySize = strip_length;
        EffectHelper::safeFreeArray((void*&)hues, arraySize);
        hues = nullptr;
    }
    initialized = false;
}

uint16_t BubbleSortEffect::calculateFrameDelay(WS2812FX* strip) const {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate delay using EffectHelper safe mapping
    const uint16_t speed_delay = EffectHelper::safeMapuint16_t(seg->beat88, 10000, 0, 0, 50);
    const uint16_t length_delay = EffectHelper::safeMapuint16_t(runtime->length, 300, 0, 0, 25);

    return speed_delay + length_delay;
}

void BubbleSortEffect::updateLEDDisplay(WS2812FX* strip) {
    if (!initialized || hues == nullptr) {
        return;
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Map all hue values to the current palette and display them
    for (uint16_t i = 0; i < strip_length; i++) {
        uint8_t colorIndex = EffectHelper::calculateColorIndex(strip, i, hues[i]);
        CRGB color = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), colorIndex, 32, seg->blendType
        );
        strip->leds[runtime->start + i] = color;
    }
}

void BubbleSortEffect::cleanup() {
    // Clean up dynamically allocated memory when effect is deactivated
    cleanupMemory();
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_BUBBLE_SORT, BubbleSortEffect)