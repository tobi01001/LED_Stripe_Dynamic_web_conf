#include "BubbleSortEffect.h"
#include "../WS2812FX_FastLed.h"

bool BubbleSortEffect::init(WS2812FX* strip) {
    // Clean up any existing memory first
    cleanupMemory();
    
    // Initialize state variables
    movedown = false;
    ci = co = cd = 0;
    initialized = false;
    
    // Get current strip length and allocate memory for hue array
    auto runtime = strip->getSegmentRuntime();
    strip_length = runtime->length;
    
    if (strip_length > 0) {
        // Allocate memory for hue values - one per LED position
        hues = new uint8_t[strip_length];
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
    
    // Calculate frame delay based on speed and strip length
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
        
        // Highlight the elements being compared
        if (ci < strip_length) {
            // Highlight current comparison position
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), hues[ci], seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + ci] = color;
        }
        
        if (co < strip_length) {
            // Highlight outer loop position  
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), hues[co], seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + co] = color;
        }
        
    } else {
        // Movement animation phase - showing the swap in progress
        updateLEDDisplay(strip);
        
        // Highlight the elements involved in the swap
        if (co < strip_length) {
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), hues[co], seg->brightness, seg->blendType
            );
            strip->leds[runtime->start + co] = color;
        }
        
        if (cd < strip_length) {
            CRGB color = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), hues[cd], seg->brightness, seg->blendType
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
        delete[] hues;
        hues = nullptr;
    }
    initialized = false;
}

uint16_t BubbleSortEffect::calculateFrameDelay(WS2812FX* strip) const {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate delay based on speed (beat88) and strip length
    // Slower speeds get longer delays, longer strips get slightly longer delays
    const uint16_t speed_delay = map(seg->beat88, (uint16_t)10000, (uint16_t)0, (uint16_t)0, (uint16_t)50);
    const uint16_t length_delay = map(runtime->length, (uint16_t)300, (uint16_t)0, (uint16_t)0, (uint16_t)25);
    
    return speed_delay + length_delay;
}

void BubbleSortEffect::updateLEDDisplay(WS2812FX* strip) {
    if (!initialized || hues == nullptr) {
        return;
    }
    
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Map all hue values to the current palette and display them
    // This shows the current state of the array being sorted
    for (uint16_t i = 0; i < strip_length; i++) {
        CRGB color = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), hues[i], 32, seg->blendType
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