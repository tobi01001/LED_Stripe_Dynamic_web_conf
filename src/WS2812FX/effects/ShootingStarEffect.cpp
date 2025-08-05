#include "ShootingStarEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool ShootingStarEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    auto seg = strip->getSegment();
    
    // Store current number of bars (shooting stars)
    numBars = seg->numBars;
    if (numBars > MAX_NUM_BARS) {
        numBars = MAX_NUM_BARS;
    }
    if (numBars == 0) {
        numBars = 1; // Ensure at least one shooting star
    }
    
    // Initialize timing and color parameters
    initializeStars(strip);
    
    setInitialized(true);
    return true;
}

uint16_t ShootingStarEffect::update(WS2812FX* strip) {
    auto seg = strip->getSegment();
    // Check if effect needs initialization
    if (!isInitialized() || basebeat != seg->beat88) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000; // Return reasonable delay if strip is invalid
    }
    
    
    // Apply fade and blur effects to create trailing stars using helper
    applyTrailEffects(strip);
    
    // Update each shooting star
    for (uint8_t i = 0; i < numBars; i++) {
        updateStar(strip, i);
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* ShootingStarEffect::getName() const {
    return F("Shooting Star");
}

uint8_t ShootingStarEffect::getModeId() const {
    return FX_MODE_SHOOTING_STAR;
}

void ShootingStarEffect::initializeStars(WS2812FX* strip) {
    auto seg = strip->getSegment();
    
    // Store base beat for change detection
    basebeat = seg->beat88;
    
    // Initialize timing offsets for each shooting star
    // This spreads them out across the animation cycle
    cind[0] = EffectHelper::get_random_wheel_index(0, 32);
    delta_b[0] = 0; // First star starts at beat 0
    new_cind[0] = false; // First star doesn't need new color immediately
    for (uint8_t i = 1; i < numBars; i++) {
        // Distribute stars evenly across the timing cycle
        delta_b[i] = (65535 / numBars) * i;
        
        // Generate color related to previous star but with variation
        cind[i] = EffectHelper::get_random_wheel_index(cind[i - 1], 32);
        
        // Mark that star doesn't need immediate new color
        new_cind[i] = false;
    }
}

void ShootingStarEffect::updateStar(WS2812FX* strip, uint8_t starIndex) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate current beat for this star (base + offset)
    uint16_t beat = (beat88(seg->beat88) * 2) + delta_b[starIndex];
    
    // Calculate position using quadratic acceleration for realistic motion
    uint16_t pos = calculateStarPosition(beat, strip);
    
    // Draw the shooting star using fractional bar for smooth movement
    // Width of 2 creates a nice star appearance, color index provides variation
    strip->drawFractionalBar(pos, 2, *strip->getCurrentPalette(), 
                            cind[starIndex], 255, true, 1);
    
    // Check if star has reached near the end of the strip
    if (pos / 16 > (runtime->stop - 4)) {
        handleStarEnd(strip, starIndex, pos);
    } else {
        // Star is still moving - check if it needs a new color
        if (new_cind[starIndex]) {
            generateNewColor(strip, starIndex);
        }
        new_cind[starIndex] = false;
    }
}

uint16_t ShootingStarEffect::calculateStarPosition(uint16_t beat, WS2812FX* strip) const {
    auto runtime = strip->getSegmentRuntime();
    
    // Use quadratic function for acceleration: position = (beat/100)^2
    // This creates realistic shooting star motion that starts slow and accelerates
    double_t q_beat = static_cast<double_t>(beat / 100) * (beat / 100);
    
    // Map the quadratic result to the strip boundaries
    // Using 16-bit fixed point for smooth sub-pixel positioning
    uint16_t pos = map(static_cast<uint32_t>(q_beat + 0.5), 
                      static_cast<uint32_t>(0), 
                      static_cast<uint32_t>(429484),  // Maximum value for mapping
                      static_cast<uint32_t>(runtime->start * 16), 
                      static_cast<uint32_t>(runtime->stop * 16));
    
    return pos;
}

void ShootingStarEffect::generateNewColor(WS2812FX* strip, uint8_t starIndex) {
    // Generate new color based on neighboring stars for smooth color transitions
    if (starIndex > 0) {
        // Base new color on previous star with some variation
        cind[starIndex] = EffectHelper::get_random_wheel_index(cind[starIndex - 1], 32);
    } else {
        // First star bases color on last star for continuous cycling
        cind[starIndex] = EffectHelper::get_random_wheel_index(cind[numBars - 1], 32);
    }
}

void ShootingStarEffect::applyTrailEffects(WS2812FX* strip) {
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Fade the entire strip to create trailing effect using helper
    // Fade amount is based on speed - faster = longer trails
    uint8_t fadeAmount = (seg->beat88 >> 8) | 0x60;  // Ensure minimum fade
    uint16_t fadeLength = runtime->length > 8 ? runtime->length - 8 : runtime->length;
    fadeToBlackBy(strip->leds, fadeLength, fadeAmount);
    
    // Apply blur to the end section for additional trail smoothing
    if (runtime->length > 8) {
        blur1d(&strip->leds[runtime->stop - 7], 8, 120);
    }
}

void ShootingStarEffect::handleStarEnd(WS2812FX* strip, uint8_t starIndex, uint16_t position) {
    auto seg = strip->getSegment();
    
    // Add sparkle effect when star reaches the end
    CRGB led = strip->ColorFromPaletteWithDistribution(
        *strip->getCurrentPalette(), cind[starIndex], seg->brightness, seg->blendType
    );
    
    if (led) {
        // Create sparkle by adding brightness to the end LED
        uint8_t sparkle_brightness = led.r | led.g | led.b;
        strip->leds[position / 16].addToRGB(sparkle_brightness % 128);
    }
    
    // Mark this star for color change on next cycle
    new_cind[starIndex] = true;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_SHOOTING_STAR, ShootingStarEffect)