#include "VoidEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

/**
 * @brief Initialize the void effect
 * 
 * The void effect initialization primarily focuses on disabling autoplay mode
 * to prevent automatic effect transitions. This maintains the intended "do nothing"
 * behavior of the void effect.
 * 
 * @param strip Pointer to the WS2812FX instance providing access to segment data
 * @return Always true as void effect initialization cannot fail
 */
bool VoidEffect::init(WS2812FX* strip) {
    // Validate strip pointer before proceeding
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Access the segment runtime to mark initialization complete
    auto runtime = strip->getSegmentRuntime();
    runtime->modeinit = false;
    
    // Access the segment data to disable autoplay mode
    // This is the core functionality of the void effect - turning off autoplay
    auto segment = strip->getSegment();
    segment->autoplay = AUTO_MODE_OFF;
    
    return true;
}

/**
 * @brief Update the void effect (intentionally does nothing)
 * 
 * The void effect update method is designed to be a no-op. It performs no
 * LED manipulation, color changes, or visual effects. The only action is
 * returning the minimum delay to optimize system performance.
 * 
 * @param strip Pointer to the WS2812FX instance (unused in this implementation)
 * @return Minimum strip delay since no processing or LED updates are needed
 */
uint16_t VoidEffect::update(WS2812FX* strip) {
    // Void effect does nothing - just return minimum delay for optimal performance
    // No LED manipulation, no color changes, no visual effects
    return strip->getStripMinDelay();
}

/**
 * @brief Get the name of the void effect
 * 
 * Returns the descriptive name that clearly indicates this effect does nothing.
 * The name matches the original implementation to maintain consistency.
 * 
 * @return Flash string containing the effect name
 */
const __FlashStringHelper* VoidEffect::getName() const {
    return F("Void DOES NOTHING");
}

/**
 * @brief Get the mode ID for the void effect
 * 
 * Returns the mode enumeration value that corresponds to this effect.
 * This allows the effect system to properly identify and route to this effect.
 * 
 * @return FX_MODE_VOID enumeration value
 */
uint8_t VoidEffect::getModeId() const {
    return FX_MODE_VOID;
}

// Register this effect with the factory system
// This enables the effect system to create instances of VoidEffect when FX_MODE_VOID is requested
REGISTER_EFFECT(FX_MODE_VOID, VoidEffect)