#include "MeteorShowerEffect.h"
#include "../WS2812FX_FastLed.h"

bool MeteorShowerEffect::init(WS2812FX* strip) {
    // Initialize internal state for meteor shower effect
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    runtime->modeinit = false;
    
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
    
    return true;
}

uint16_t MeteorShowerEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    uint32_t currentTime = millis();
    
    // Apply background fade at regular intervals for trailing effect
    if (currentTime - lastFadeTime >= FADE_INTERVAL) {
        // Fade amount is controlled by speed setting - faster speed = more fade
        uint8_t fadeAmount = map(seg->beat88, (uint16_t)100, (uint16_t)7968, (uint16_t)3, (uint16_t)255);
        fadeToBlackBy(&strip->leds[runtime->start], runtime->length, fadeAmount);
        lastFadeTime = currentTime;
    }
    
    // Update and draw all active meteors
    for (uint8_t i = 0; i < seg->numBars && i < 10; i++) {
        if (state.actives[i]) {
            // Calculate meteor position using beat88 function for smooth movement
            // Position moves from top (stop) to bottom (start) of segment
            uint16_t pos16 = map((uint16_t)(beat88(seg->beat88 * 3, state.timebase[i])), 
                               (uint16_t)0, (uint16_t)65535, 
                               (uint16_t)(runtime->stop * 16), (uint16_t)(runtime->start * 16));
            
            // Draw the meteor at current position
            drawMeteor(strip, pos16, state.cind[i]);
            
            // Deactivate meteor when it reaches the bottom
            if (!(pos16 / 16)) {
                state.actives[i] = false;
            }
        }
    }
    
    // Spawn new meteors at regular intervals
    if (currentTime - lastSpawnTime >= SPAWN_INTERVAL) {
        // Check if spawn area is clear (minimum distance from bottom)
        uint16_t minDistance = max(runtime->length / 12, 1);
        
        if (isSpawnAreaClear(strip, minDistance)) {
            // Try to spawn a new meteor in an available slot
            uint8_t availableSlot = findAvailableSlot(seg->numBars);
            
            // Random chance to spawn (1 in 4 chance) and available slot exists
            if (availableSlot < 10 && !random8(4)) {
                state.actives[availableSlot] = true;
                state.timebase[availableSlot] = currentTime;
                state.cind[availableSlot] = strip->get_random_wheel_index(state.cind[availableSlot]);
                
                // Draw initial meteor position
                uint16_t initialPos = map((uint16_t)(beat88(seg->beat88 * 3, state.timebase[availableSlot])), 
                                        (uint16_t)0, (uint16_t)65535, 
                                        (uint16_t)(runtime->stop * 16), (uint16_t)(runtime->start * 16));
                drawMeteor(strip, initialPos, state.cind[availableSlot]);
            }
        }
        lastSpawnTime = currentTime;
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

void MeteorShowerEffect::drawMeteor(WS2812FX* strip, uint16_t pos16, uint8_t colorIndex) {
    // Draw meteor using fractional bar drawing for smooth movement
    // The meteor appears as a short bright bar with the specified color offset
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    CRGB meteorColor = strip->ColorFromPaletteWithDistribution(
        *strip->getCurrentPalette(), 
        runtime->baseHue + colorIndex,  // Add color variation
        255,                           // Full brightness for meteor
        seg->blendType
    );
    
    // Use the strip's fractional bar drawing function for smooth positioning
    // This draws a bar at fractional pixel positions for fluid movement
    strip->drawFractionalBar(pos16, METEOR_WIDTH, *strip->getCurrentPalette(), 
                           runtime->baseHue + colorIndex, 255, true, 0);
}

bool MeteorShowerEffect::isSpawnAreaClear(WS2812FX* strip, uint16_t minDistance) {
    // Check if the spawn area (near the top/end of strip) is sufficiently dark
    // This prevents meteors from spawning too close together
    auto runtime = strip->getSegmentRuntime();
    
    for (uint16_t i = 0; i < minDistance && i < runtime->length; i++) {
        uint16_t checkPos = runtime->stop - i;
        if (checkPos < runtime->length && 
            strip->leds[runtime->start + checkPos] != CRGB(0x000000)) {
            return false;  // Area not clear
        }
    }
    return true;  // Area is clear for spawning
}

uint8_t MeteorShowerEffect::findAvailableSlot(uint8_t maxMeteors) {
    // Find first inactive meteor slot for spawning new meteor
    for (uint8_t i = 0; i < maxMeteors && i < 10; i++) {
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