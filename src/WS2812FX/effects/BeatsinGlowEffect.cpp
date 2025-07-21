#include "BeatsinGlowEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool BeatsinGlowEffect::init(WS2812FX* strip) {
    // Use standard initialization pattern from helper
    bool initialized = false;
    uint32_t timebase = 0;
    if (!EffectHelper::standardInit(strip, timebase, initialized)) {
        return false;
    }
    
    auto seg = strip->getSegment();
    
    // Store current number of bars (glow elements)
    numBars = seg->numBars;
    if (numBars > MAX_NUM_BARS) {
        numBars = MAX_NUM_BARS;
    }
    if (numBars == 0) {
        numBars = 1; // Ensure at least one glow element
    }
    
    // Initialize all glow elements
    initializeGlowElements(strip);
    
    return true;
}

uint16_t BeatsinGlowEffect::update(WS2812FX* strip) {
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    auto runtime = strip->getSegmentRuntime();
    
    // Check if we need to reinitialize
    if (runtime->modeinit) {
        return init(strip) ? strip->getStripMinDelay() : strip->getStripMinDelay();
    }
    
    // Apply background fade for smooth transitions using helper
    applyBackgroundFade(strip);
    
    // Update each glow element
    for (uint8_t i = 0; i < numBars; i++) {
        updateGlowElement(strip, i);
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* BeatsinGlowEffect::getName() const {
    return F("Sine glows");
}

uint8_t BeatsinGlowEffect::getModeId() const {
    return FX_MODE_BEATSIN_GLOW;
}

void BeatsinGlowEffect::initializeGlowElements(WS2812FX* strip) {
    auto seg = strip->getSegment();
    const uint16_t lim = calculateVariationLimit(strip);
    
    // Initialize each glow element with unique parameters
    for (uint8_t i = 0; i < numBars; i++) {
        // Set beat frequency with random variation around base speed
        beats[i] = seg->beat88 + lim / 2 - random16(lim);
        
        // Distribute phase offsets evenly with some randomization
        // This creates interesting interference patterns between elements
        theta[i] = (65535 / numBars) * i + (65535 / (4 * numBars)) - random16(65535 / (2 * numBars));
        
        // Distribute colors across the color wheel with variation
        uint8_t base_color = (255 / numBars) * i;
        uint8_t color_variation = random8(255 / (2 * numBars));
        
        if (color_variation & 0x01) {
            // Subtract variation (wrap around if needed)
            cinds[i] = base_color - color_variation;
        } else {
            // Add variation
            cinds[i] = base_color + color_variation;
        }
        
        // Initialize timing with random offset for each element
        times[i] = millis() + random8();
        
        // Initialize state tracking variables
        prev[i] = 0;
        newval[i] = false;
    }
}

void BeatsinGlowEffect::updateGlowElement(WS2812FX* strip, uint8_t elementIndex) {
    auto seg = strip->getSegment();
    
    // Calculate current sine value with beat, time, and phase offsets
    uint16_t beatval = beat88(beats[elementIndex], times[elementIndex] + theta[elementIndex]);
    int16_t si = sin16(beatval);
    
    // Check for zero crossing (sine wave transitioning through zero)
    // This is where we update element parameters for organic variation
    if (si > -2 && si < 2 && prev[elementIndex] < si) {
        updateElementParameters(strip, elementIndex);
        newval[elementIndex] = false;
    } else {
        newval[elementIndex] = true;
    }
    
    // Store current sine value for next frame's zero crossing detection
    prev[elementIndex] = si;
    
    // Calculate position from sine value
    uint16_t pos = calculateElementPosition(elementIndex, strip);
    
    // Draw the glow element using fractional bar for smooth positioning
    // Each element gets its own color with brightness variation
    uint8_t color_index = cinds[elementIndex] + elementIndex * (255 / numBars);
    strip->drawFractionalBar(pos, 2, *strip->getCurrentPalette(), 
                            color_index, seg->brightness, true, 1);
}

uint16_t BeatsinGlowEffect::calculateElementPosition(uint8_t elementIndex, WS2812FX* strip) {
    auto runtime = strip->getSegmentRuntime();
    
    // Get current sine value
    uint16_t beatval = beat88(beats[elementIndex], times[elementIndex] + theta[elementIndex]);
    int16_t si = sin16(beatval);
    
    // Map sine value (-32768 to 32767) to strip position using helper
    uint16_t pos = EffectHelper::safeMap((65535 >> 1) + si, 0, 65535, 
                                        runtime->start * 16, runtime->stop * 16);
    
    return pos;
}

void BeatsinGlowEffect::updateElementParameters(WS2812FX* strip, uint8_t elementIndex) {
    auto seg = strip->getSegment();
    const uint8_t rand_delta = 64;  // Amount of random variation to apply
    
    // Update beat frequency with controlled random variation
    // Keep it within reasonable bounds relative to base speed
    beats[elementIndex] = beats[elementIndex] + (seg->beat88 * 10) / 50 - random16((seg->beat88 * 10) / 25);
    
    // Constrain beat frequency to reasonable range
    if (beats[elementIndex] < (seg->beat88 / 2)) {
        beats[elementIndex] = seg->beat88 / 2;
    }
    if (beats[elementIndex] > (seg->beat88 + seg->beat88 / 2)) {
        beats[elementIndex] = seg->beat88 + seg->beat88 / 2;
    }
    
    // Update phase offset with random variation
    theta[elementIndex] = theta[elementIndex] + (rand_delta / 2) - random8(rand_delta);
    
    // Update color index with random variation
    cinds[elementIndex] = cinds[elementIndex] + (rand_delta / 2) - random8(rand_delta);
    
    // Update time offset to create complex timing relationships
    times[elementIndex] = millis() - theta[elementIndex];
}

void BeatsinGlowEffect::applyBackgroundFade(WS2812FX* strip) {
    auto seg = strip->getSegment();
    
    // Apply fade to create smooth background transitions using helper
    // Fade amount is based on speed with minimum value to ensure visibility
    uint8_t fadeAmount = (seg->beat88 >> 8) | 32;
    EffectHelper::applyFadeEffect(strip, fadeAmount);
}

uint16_t BeatsinGlowEffect::calculateVariationLimit(WS2812FX* strip) const {
    auto seg = strip->getSegment();
    
    // Calculate variation limit based on current speed
    // Faster speeds get more variation for dynamic effects
    return (seg->beat88 * 10) / 50;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_BEATSIN_GLOW, BeatsinGlowEffect)