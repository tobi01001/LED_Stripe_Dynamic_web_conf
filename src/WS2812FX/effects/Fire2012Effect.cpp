#include "Fire2012Effect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool Fire2012Effect::init(WS2812FX* strip) {
    // Allocate and initialize heat array for fire simulation
    if (!allocateHeatArray(strip)) {
        return false; // Failed to allocate memory
    }
    
    // Clear heat array to start with a cold fire
    memset(heatArray, 0, heatArraySize * sizeof(byte));
    
    // Use standard initialization pattern from helper
    return EffectHelper::standardInit(strip, timebase, initialized);
}

uint16_t Fire2012Effect::update(WS2812FX* strip) {
    // Ensure initialization if somehow missed
    if (!initialized) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return early if initialization fails
        }
    }
    
    // Perform the four steps of fire simulation:
    
    // Step 1: Cool down every cell a little
    // This creates the natural cooling effect as heat dissipates
    performCooling(strip);
    
    // Step 2: Heat diffusion upward
    // Heat from lower cells influences higher cells, creating upward flow
    performHeatDiffusion(strip);
    
    // Step 3: Randomly ignite new sparks near the bottom
    // This creates the base fire source and maintains the effect
    performSparking(strip);
    
    // Step 4: Map heat values to LED colors
    // Convert the heat simulation to visible fire colors
    mapHeatToColors(strip);
    
    // Return minimum delay for smooth fire animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* Fire2012Effect::getName() const {
    return F("Fire 2012 - Specific Colors");
}

uint8_t Fire2012Effect::getModeId() const {
    return FX_MODE_FIRE2012;
}

void Fire2012Effect::cleanup() {
    // Free allocated memory when effect is deactivated
    freeHeatArray();
    initialized = false;
}

bool Fire2012Effect::allocateHeatArray(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) {
        return false; // Cannot allocate heat array without valid runtime
    }
    
    // Use helper to safely allocate memory for heat array
    size_t currentSize = heatArraySize; // Convert to size_t for helper function
    void* newArray = EffectHelper::safeAllocateArray(
        heatArray,              // Current array pointer
        currentSize,            // Current size (will be updated)
        runtime->length,        // Required size
        sizeof(byte)            // Element size
    );
    
    if (newArray == nullptr) {
        return false; // Memory allocation failed
    }
    
    heatArray = (byte*)newArray;
    heatArraySize = (uint16_t)currentSize; // Update the class member
    return true;
}

void Fire2012Effect::freeHeatArray() {
    // Use helper to safely free memory
    size_t currentSize = heatArraySize; // Convert to size_t for helper function
    EffectHelper::safeFreeArray((void*&)heatArray, currentSize);
    heatArraySize = (uint16_t)currentSize; // Update the class member (should be 0)
}

void Fire2012Effect::performCooling(WS2812FX* strip) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Cool down every heat cell by a random amount
    // The cooling rate is based on the cooling parameter and strip length
    for (uint16_t i = 0; i < runtime->length; i++) {
        // Calculate cooling amount: random value based on cooling setting
        // Longer strips have proportionally less cooling to maintain fire density
        uint8_t coolingAmount = random8(0, ((seg->cooling * 10) / runtime->length) + 2);
        
        // Apply cooling with qsub8 to prevent underflow
        heatArray[i] = qsub8(heatArray[i], coolingAmount);
    }
}

void Fire2012Effect::performHeatDiffusion(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Heat diffusion: each cell is influenced by the cells below it
    // Working from top to bottom to avoid affecting calculations
    for (int k = runtime->length - 1; k >= 2; k--) {
        // Each cell gets heat from the two cells below it, averaged
        // This creates a natural upward heat flow pattern
        heatArray[k] = (heatArray[k - 1] + heatArray[k - 2] + heatArray[k - 2]) / 3;
    }
}

void Fire2012Effect::performSparking(WS2812FX* strip) {
    auto seg = strip->getSegment();
    
    // Randomly ignite new sparks near the bottom of the fire
    // Sparking parameter controls how often new heat sources appear
    if (random8() < seg->sparking) {
        // Choose a random position near the bottom (first 7 LEDs)
        int sparkPosition = random8(7);
        
        // Add a random amount of heat to create the spark
        // Heat value is high (160-255) to create bright, hot sparks
        uint8_t sparkHeat = random8(160, 255);
        heatArray[sparkPosition] = qadd8(heatArray[sparkPosition], sparkHeat);
    }
}

void Fire2012Effect::mapHeatToColors(WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Convert heat values to LED colors using heat palette
    for (uint16_t j = 0; j < runtime->length; j++) {
        // Scale heat value from 0-255 down to 0-240
        // This provides the best color range in most heat palettes
        byte colorIndex = scale8(heatArray[j], 240);
        
        // Get color from the heat palette
        // Using HEAT_PAL (heat-based color palette) for realistic fire colors
        // Heat palettes typically go from black -> red -> orange -> yellow -> white
        CRGB color = ColorFromPalette(HeatColors_p, colorIndex);
        
        // Set the LED color
        strip->leds[j + runtime->start] = color;
    }
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FIRE2012, Fire2012Effect)