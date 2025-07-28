#include "MeteorShowerEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool MeteorShowerEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    auto seg = strip->getSegment();
    
    // Initialize all meteor state arrays
    // Clear all timebase, active status, and color indices
    for (uint8_t i = 0; i < 10; i++) {  // MAX_NUM_BARS is typically 10
        state.timebase[i] = 0;
        state.actives[i] = false;
        state.cind[i] = random8();  // Random color offset for each slot
    }
    
    // Initialize timing variables
    lastFadeTime = millis();
    lastSpawnTime = millis();
    
    // Start with first meteor active
    if (seg->numBars > 0) {
        state.timebase[0] = millis();
        state.actives[0] = true;
        state.cind[0] = random8();
    }
    
    setInitialized(true);
    return true;
}

uint16_t MeteorShowerEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    uint32_t currentTime = millis();
    
    // Apply background fade at regular intervals for trailing effect using helper
    if (currentTime - lastFadeTime >= FADE_INTERVAL) {
        // Fade amount is controlled by speed setting using helper
        uint8_t fadeAmount = EffectHelper::safeMapuint16_t(seg->beat88, 100, 7968, 3, 255);
        EffectHelper::applyFadeEffect(strip, fadeAmount);
        lastFadeTime = currentTime;
    }
    
    // Update and draw all active meteors
    for (uint8_t i = 0; i < seg->numBars && i < 10; i++) { // Ensure i does not exceed array bounds
        if (state.actives[i]) {
            // Calculate meteor position using beat88 function for smooth movement
            // Position moves from top (stop) to bottom (start) of segment
            uint16_t beatPos = beat88(seg->beat88*3, state.timebase[i]);
            //uint16_t pos16 = EffectHelper::safeMapuint16_t(beatPos, 0, 65535, 
            //                                     runtime->stop * 16, runtime->start * 16);
            uint16_t pos16 = map(beatPos, 0, 65535, runtime->stop * 16, runtime->start * 16);

            strip->drawFractionalBar(pos16, 4, *strip->getCurrentPalette(), runtime->baseHue + state.cind[i], 255, true, 0);

            // Deactivate meteor when it reaches the bottom
            if ((pos16 == runtime->start * 16) || (pos16 / 16 <= 0)) {
                state.actives[i] = false;
            }
        }
    }
    
    // Spawn new meteors at regular intervals
    if (currentTime - lastSpawnTime >= SPAWN_INTERVAL) {
        // Check if spawn area is clear (minimum distance from bottom) using helper
        uint16_t minDistance = EffectHelper::calculateProportionalWidth(strip, 12, 1);
        
        if (isSpawnAreaClear(strip, minDistance)) {
            // Try to spawn a new meteor in an available slot
            uint8_t availableSlot = findAvailableSlot(seg->numBars);
            
            // Random chance to spawn (1 in 4 chance) and available slot exists
            if (availableSlot != 255 && availableSlot < 10 && !random8(4)) {
                state.actives[availableSlot] = true;
                state.timebase[availableSlot] = currentTime;
                state.cind[availableSlot] = EffectHelper::get_random_wheel_index(state.cind[availableSlot], 42);
                
                // Draw initial meteor position using helper
                uint16_t beatPos = EffectHelper::calculateBeatPosition(strip, state.timebase[availableSlot], EffectHelper::FAST_SPEED);
                uint16_t initialPos = EffectHelper::safeMapuint16_t(beatPos, 0, 65535,
                                                          runtime->stop * 16, runtime->start * 16);

                strip->drawFractionalBar(initialPos, 4, *strip->getCurrentPalette(), runtime->baseHue + state.cind[availableSlot], 255, true, 0);
            }
        }
        lastSpawnTime = currentTime;
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}


bool MeteorShowerEffect::isSpawnAreaClear(WS2812FX* strip, uint16_t minDistance) {
    // Check if the spawn area (near the top/end of strip) is sufficiently dark
    // This prevents meteors from spawning too close together
    auto runtime = strip->getSegmentRuntime();
    
    for (uint16_t i = 0; i < minDistance && i < runtime->length; i++) {
        uint16_t checkPos = runtime->stop - i;
        if (strip->leds[checkPos] != CRGB(0x000000)) {
            return false;  // Area not clear
        }
    }
    return true;  // Area is clear for spawning
}

uint8_t MeteorShowerEffect::findAvailableSlot(uint8_t maxMeteors) {
    // Find first inactive meteor slot for spawning new meteor
    for (uint8_t i = 0; i < maxMeteors; i++) {
        if (!state.actives[i]) {
            return i;  // Found available slot
        }
    }
    return 255;  // No available slots
}

const __FlashStringHelper* MeteorShowerEffect::getName() const {
    return F("Meteor Shower");
}

uint8_t MeteorShowerEffect::getModeId() const {
    return FX_MODE_RAIN;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_RAIN, MeteorShowerEffect)