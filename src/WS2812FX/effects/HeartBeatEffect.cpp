#include "HeartBeatEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool HeartBeatEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false;
    uint32_t timebase = 0;
    if (!EffectHelper::standardInit(strip, timebase, initialized)) {
        return false;
    }
    
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    
    // Initialize heartbeat state variables
    state.lastBeat = 0;
    state.secondBeatActive = false;
    
    // Calculate timing based on speed setting (beat88 parameter)
    // Convert speed to beats per minute, ensuring minimum rate
    state.msPerBeat = calculateBeatsPerMinute(seg->beat88);
    state.secondBeat = (state.msPerBeat / 3);  // Secondary beat at 1/3 of cycle
    
    // Calculate pulse size based on strip length using helper
    state.size = EffectHelper::calculateProportionalWidth(strip, 25, 1);
    
    // Set up center position and pixel processing count
    state.centerOffset = (runtime->length / 2);
    state.pCount = state.centerOffset - state.size;
    
    return true;
}

uint16_t HeartBeatEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    // Ensure effect is properly initialized
    if (!isInitialized()) {
        if (!init(strip)) {
            return strip->getStripMinDelay(); // Return minimum delay if init failed
        }
    }
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Create the shifting/spreading effect by moving pixels outward from center
    // This creates the visual effect of the pulse spreading from the heart center
    for (uint16_t i = 0; i < state.pCount; i++) {
        // Ensure indices are within bounds before accessing the array
        if ((runtime->start + i + state.size < runtime->length) &&
            (runtime->start + i + state.centerOffset + state.size < runtime->length)) {
            // Shift pixels outward on both sides of center
            strip->leds[runtime->start + i] = strip->leds[runtime->start + i + state.size];
            strip->leds[runtime->start + i + state.centerOffset + state.size] = 
                strip->leds[runtime->start + i + state.centerOffset];
        }
    }
    
    // Apply continuous fade to create smooth pulse decay using helper
    uint8_t fadeAmount = (seg->beat88 >> 8) | 32;
    EffectHelper::applyFadeEffect(strip, fadeAmount);
    
    // Calculate elapsed time since last primary beat
    state.beatTimer = millis() - state.lastBeat;
    
    // Check if it's time for the secondary beat (lub-dub pattern)
    if ((state.beatTimer > state.secondBeat) && !state.secondBeatActive) {
        // Create the secondary beat pulse at center
        CRGB pulseColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            runtime->baseHue, 
            seg->brightness, 
            seg->blendType
        );
        
        fill_solid(&strip->leds[runtime->start + state.centerOffset - state.size], 
                   state.size * 2, pulseColor);
        
        state.secondBeatActive = true;
    }
    
    // Check if it's time to reset the beat cycle (primary beat)
    if (state.beatTimer > state.msPerBeat) {
        // Create the primary beat pulse at center
        CRGB pulseColor = strip->ColorFromPaletteWithDistribution(
            *strip->getCurrentPalette(), 
            runtime->baseHue, 
            seg->brightness, 
            seg->blendType
        );
        
        fill_solid(&strip->leds[runtime->start + state.centerOffset - state.size], 
                   state.size * 2, pulseColor);
        
        // Reset timing for next cycle
        state.secondBeatActive = false;
        state.lastBeat = millis();
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

uint16_t HeartBeatEffect::calculateBeatsPerMinute(uint16_t speed) {
    // Convert speed parameter to realistic heartbeat timing using helper
    uint16_t effectiveSpeed = (speed > 20) ? speed / 20 : 1;
    return (60000 / effectiveSpeed);  // Convert to milliseconds per beat
}

const __FlashStringHelper* HeartBeatEffect::getName() const {
    return F("Heart Beat");
}

uint8_t HeartBeatEffect::getModeId() const {
    return FX_MODE_HEARTBEAT;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_HEARTBEAT, HeartBeatEffect)