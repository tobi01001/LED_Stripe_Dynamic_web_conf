#include "FireworkEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool FireworkEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Initialize all firework state arrays to empty/inactive state
    memset(fireworkPositions, 0, sizeof(fireworkPositions));
    memset(colorIndices, 0, sizeof(colorIndices));
    memset(burnTimeRemaining, 0, sizeof(burnTimeRemaining));
    
    // Clear the entire LED strip using EffectHelper
    EffectHelper::clearSegment(strip);
    
    initialized = true;
    
    // Mark as initialized in the segment runtime
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    return true;
}

uint16_t FireworkEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Ensure initialization if somehow missed
    if (!initialized) {
        init(strip);
    }
    
    // Apply blur effect using EffectHelper safe mapping
    uint8_t blurAmount = EffectHelper::safeMap(seg->beat88 >> 8, 0, 255, 32, 172);
    blur1d(&strip->leds[runtime->start], runtime->length, blurAmount);
    
    // Update existing fireworks - decrease burn time and render
    for (uint8_t i = 0; i < MAX_FIREWORKS; i++) {
        if (burnTimeRemaining[i] > 0) {
            // Decrease burn time (firework fades over time)
            burnTimeRemaining[i]--;
            
            // Render the firework with palette color and blending
            CRGB fireworkColor = strip->ColorFromPaletteWithDistribution(
                *strip->getCurrentPalette(), 
                colorIndices[i], 
                255,                    // Full brightness
                seg->blendType
            );
            
            // Set firework pixel with additive blending for bright explosion effect
            if (fireworkPositions[i] < runtime->length) {
                strip->leds[runtime->start + fireworkPositions[i]] += fireworkColor;
            }
        }
    }
    
    // Launch new fireworks based on probability calculated from speed setting
    auto runtime_spawn = strip->getSegmentRuntime();
    
    // Calculate spawn probability using EffectHelper
    uint8_t spawnProbability = max(6, runtime_spawn->length / 7);
    uint8_t spawnThreshold = max(3, runtime_spawn->length / 14);
    
    if (random8(spawnProbability) <= spawnThreshold) {
        // Calculate minimum distance between fireworks to prevent clustering
        uint8_t minDistance = calculateMinDistance(strip);
        
        // Try to find a good position for a new firework
        uint16_t candidatePosition = random16(
            minDistance + runtime_spawn->start, 
            runtime_spawn->stop - minDistance
        );
        
        // Check if the candidate position is clear
        if (isPositionClear(candidatePosition, minDistance, strip)) {
            // Find an available slot for the new firework
            uint8_t availableSlot = findAvailableSlot();
            
            if (availableSlot != 0xff) {
                // Spawn the new firework
                spawnFirework(candidatePosition, availableSlot, strip);
            }
        }
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FireworkEffect::getName() const {
    return F("Firework");
}

uint8_t FireworkEffect::getModeId() const {
    return FX_MODE_FIREWORK;
}

uint8_t FireworkEffect::calculateMinDistance(WS2812FX* strip) const {
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate minimum distance as a fraction of strip length
    // Ensures fireworks don't spawn too close together
    // Minimum of 2 LEDs prevents immediate neighbors
    return max(runtime->length / 20, 2);
}

bool FireworkEffect::isPositionClear(uint16_t position, uint8_t minDistance, WS2812FX* strip) const {
    auto runtime = strip->getSegmentRuntime();
    
    // Check a range around the candidate position for existing light
    for (int8_t offset = -minDistance; offset <= minDistance; offset++) {
        uint16_t checkPosition = position + offset;
        
        // Ensure position is within strip bounds
        if (checkPosition >= runtime->start && checkPosition < runtime->stop) {
            // If any LED in the range is already lit, position is not clear
            if (!(strip->leds[checkPosition] == CRGB(0x0))) {
                return false;
            }
        }
    }
    
    return true;
}

uint8_t FireworkEffect::findAvailableSlot() const {
    // Look for a slot where the firework has finished burning
    for (uint8_t i = 0; i < MAX_FIREWORKS; i++) {
        if (burnTimeRemaining[i] == 0) {
            return i;
        }
    }
    
    // No available slots - all fireworks still burning
    return 0xff;
}

void FireworkEffect::spawnFirework(uint16_t position, uint8_t slotIndex, WS2812FX* strip) {
    auto seg = strip->getSegment();
    
    // Store firework position
    fireworkPositions[slotIndex] = position;
    
    // Select a random color from the palette, avoiding too similar colors
    // get_random_wheel_index helps distribute colors across the palette
    colorIndices[slotIndex] = strip->get_random_wheel_index(colorIndices[slotIndex], 64);
    
    // Set random burn time - fireworks last different amounts of time
    // Shorter burns create quick flashes, longer burns create sustained sparkles
    burnTimeRemaining[slotIndex] = random8(10, 30);
    
    // Immediately light up the LED with the firework color
    CRGB fireworkColor = strip->ColorFromPaletteWithDistribution(
        *strip->getCurrentPalette(),
        colorIndices[slotIndex],
        random8(192, 255),  // Random brightness for variety
        seg->blendType
    );
    
    strip->leds[position] = fireworkColor;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_FIREWORK, FireworkEffect)