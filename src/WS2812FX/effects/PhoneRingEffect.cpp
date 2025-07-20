#include "PhoneRingEffect.h"
#include "../WS2812FX_FastLed.h"

bool PhoneRingEffect::init(WS2812FX* strip) {
    // Initialize internal state for phone ring effect
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    // Initialize phone ring state variables
    state.now = millis();
    state.isOn = true;              // Start with "on" state
    state.nextmillis = 0;           // Reset timing
    state.pausemillis = state.now + 10; // Start pause timer slightly in future
    state.isPause = false;          // Begin in active ring mode
    
    return true;
}

uint16_t PhoneRingEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Update current time
    state.now = millis();
    
    if (state.isPause) {
        // During pause period: fade to black and check if pause is over
        fadeToBlackBy(&strip->leds[runtime->start], runtime->length, 32);
        
        // Check if pause period has ended
        if (state.now > (state.pausemillis + PAUSE_TIME)) {
            state.pausemillis = state.now;  // Reset pause timer
            state.isPause = false;          // Exit pause mode
        }
    } else {
        // During active ring sequence: alternate between on/off states
        if (state.isOn) {
            // "On" state: fill strip with current palette colors
            CRGBPalette16* currentPalette = strip->getCurrentPalette();
            uint8_t paletteDistribution = seg->paletteDistribution;
            uint8_t deltaHue = max(1, (256 * 100 / (runtime->length * paletteDistribution)));
            
            fill_palette(&strip->leds[runtime->start], runtime->length, 
                        runtime->baseHue, deltaHue, *currentPalette, 
                        255, seg->blendType);
            
            // Check if on time has elapsed
            if (state.now > (state.nextmillis + ON_TIME)) {
                state.nextmillis = state.now;  // Reset timer
                state.isOn = false;            // Switch to off state
            }
        } else {
            // "Off" state: turn all LEDs black
            fill_solid(&strip->leds[runtime->start], runtime->length, CRGB::Black);
            
            // Check if off time has elapsed
            if (state.now > (state.nextmillis + OFF_TIME)) {
                state.nextmillis = state.now;  // Reset timer
                state.isOn = true;             // Switch back to on state
            }
        }
        
        // Check if the entire ring sequence duration has elapsed
        if (state.now > (state.pausemillis + RUN_TIME)) {
            state.pausemillis = state.now;  // Start new pause period
            state.isPause = true;           // Enter pause mode
        }
    }
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

const __FlashStringHelper* PhoneRingEffect::getName() const {
    return F("Phone Ring");
}

uint8_t PhoneRingEffect::getModeId() const {
    return FX_MODE_RING_RING;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_RING_RING, PhoneRingEffect)