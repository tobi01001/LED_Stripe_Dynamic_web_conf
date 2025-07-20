#include "HeartBeatEffect.h"
#include "../WS2812FX_FastLed.h"

bool HeartBeatEffect::init(WS2812FX* strip) {
    // Initialize internal state for heartbeat effect
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    runtime->modeinit = false;
    
    // Initialize heartbeat state variables
    state.lastBeat = 0;
    state.secondBeatActive = false;
    
    // Calculate timing based on speed setting (beat88 parameter)
    // Convert speed to beats per minute, ensuring minimum rate
    state.msPerBeat = calculateBeatsPerMinute(seg->beat88);
    state.secondBeat = (state.msPerBeat / 3);  // Secondary beat at 1/3 of cycle
    
    // Calculate pulse size based on strip length
    state.size = calculatePulseSize(runtime->length);
    
    // Set up center position and pixel processing count
    state.centerOffset = (runtime->length / 2);
    state.pCount = state.centerOffset - state.size;
    
    return true;
}

uint16_t HeartBeatEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Create the shifting/spreading effect by moving pixels outward from center
    // This creates the visual effect of the pulse spreading from the heart center
    for (uint16_t i = 0; i < state.pCount; i++) {
        // Shift pixels outward on both sides of center
        strip->leds[runtime->start + i] = strip->leds[runtime->start + i + state.size];
        strip->leds[runtime->start + i + state.centerOffset + state.size] = 
            strip->leds[runtime->start + i + state.centerOffset];
    }
    
    // Apply continuous fade to create smooth pulse decay
    // Fade amount is influenced by speed setting with minimum fade of 32
    uint8_t fadeAmount = (seg->beat88 >> 8) | 32;
    fadeToBlackBy(&strip->leds[runtime->start], runtime->length, fadeAmount);
    
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
    // Convert speed parameter to realistic heartbeat timing
    // Speed ranges from BEAT88_MIN to BEAT88_MAX, map to reasonable BPM range
    // Ensure minimum rate to prevent division by zero and overly slow beats
    uint16_t effectiveSpeed = (speed > 20) ? speed / 20 : 1;
    return (60000 / effectiveSpeed);  // Convert to milliseconds per beat
}

uint8_t HeartBeatEffect::calculatePulseSize(uint16_t stripLength) {
    // Calculate appropriate pulse size based on strip length
    // Map strip length range to reasonable pulse size range (1-6 pixels)
    // Longer strips get larger pulses for better visibility
    return map(stripLength, (uint16_t)25, (uint16_t)300, (uint16_t)1, (uint16_t)6);
}

const __FlashStringHelper* HeartBeatEffect::getName() const {
    return F("Heart Beat");
}

uint8_t HeartBeatEffect::getModeId() const {
    return FX_MODE_HEARTBEAT;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_HEARTBEAT, HeartBeatEffect)