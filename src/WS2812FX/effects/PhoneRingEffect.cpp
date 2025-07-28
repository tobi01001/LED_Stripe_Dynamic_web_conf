#include "PhoneRingEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool PhoneRingEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
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
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    // Update current time
    state.now = millis();
    
    if (state.isPause) {
        // During pause period: apply fade effect and check if pause is over
        EffectHelper::applyFadeEffect(strip, EffectHelper::LIGHT_FADE);
        
        // Check if pause period has ended
        if (state.now > (state.pausemillis + PAUSE_TIME)) {
            state.pausemillis = state.now;  // Reset pause timer
            state.isPause = false;          // Exit pause mode
        }
    } else {
        // During active ring sequence: alternate between on/off states
        if (state.isOn) {
            // "On" state: fill strip with current palette colors using helper
            EffectHelper::fillPaletteWithBrightness(strip, EffectHelper::MAX_BRIGHTNESS, EffectHelper::DEFAULT_HUE_DELTA);
            
            // Check if on time has elapsed
            if (state.now > (state.nextmillis + ON_TIME)) {
                state.nextmillis = state.now;  // Reset timer
                state.isOn = false;            // Switch to off state
            }
        } else {
            // "Off" state: clear segment to black
            EffectHelper::clearSegment(strip);
            
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