#include "MoveBarSinEffect.h"
#include "../WS2812FX_FastLed.h"

bool MoveBarSinEffect::init(WS2812FX* strip) {
    // Access the segment runtime through the public getter
    auto runtime = strip->getSegmentRuntime();
    
    // Initialize timebase for consistent beat calculations
    timebase = millis();
    
    // Mark effect as initialized
    runtime->modeinit = false;
    
    return true;
}

uint16_t MoveBarSinEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through the strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate the bar width as half the segment length
    const uint16_t width = runtime->length / 2;
    
    // Calculate speed mapping from beat88 setting
    // Constrain speed to prevent extreme values and map to appropriate range
    uint16_t constrainedSpeed = seg->beat88 > (20000 / seg->segments) ? 
                                (20000 / seg->segments) : seg->beat88;
    const uint16_t speed = map(constrainedSpeed, 0, (20000 / seg->segments), 0, 65535);
    
    // Apply background fade effect based on current speed
    applyBackgroundFade(strip, speed);
    
    // Calculate the sine wave position for smooth movement
    uint16_t position = calculateSinePosition(speed, width);
    
    // Draw the moving bar at the calculated position
    drawMovingBar(strip, position, width);
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

uint16_t MoveBarSinEffect::calculateSinePosition(uint16_t speed, uint16_t width) {
    // Use beatsin16 for smooth sine wave movement
    // The bar oscillates from position 0 to (width * 16) for fractional positioning
    // Speed is halved to create a more pleasing movement rate
    return beatsin16(speed / 2, 0, width * 16, timebase);
}

void MoveBarSinEffect::applyBackgroundFade(WS2812FX* strip, uint16_t speed) {
    // Access segment runtime to get strip length
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate fade amount based on speed
    // Faster speeds create more fade, slower speeds preserve more background
    uint8_t fadeAmount = map(speed, (uint16_t)0, (uint16_t)65535, (uint16_t)64, (uint16_t)255);
    
    // Apply fade to the entire LED array
    fadeToBlackBy(strip->leds, runtime->length, fadeAmount);
}

void MoveBarSinEffect::drawMovingBar(WS2812FX* strip, uint16_t position, uint16_t width) {
    // Access segment runtime to get base hue
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate color increment for smooth palette distribution across the bar
    // Ensure minimum increment of 1 to avoid division by zero
    uint8_t colorIncrement = max(255 / width, 1);
    
    // Draw fractional bar using the current palette
    // Parameters:
    // - position: 16-bit fractional position for smooth movement
    // - width: width of the bar to draw
    // - palette: current color palette
    // - baseHue: starting hue from runtime
    // - brightness: maximum brightness (255)
    // - mixColor: false - don't mix with existing colors
    // - colorIncrement: hue increment for palette distribution
    strip->drawFractionalBar(position, width, *strip->getCurrentPalette(), 
                            runtime->baseHue, 255, false, colorIncrement);
}

const __FlashStringHelper* MoveBarSinEffect::getName() const {
    return F("1/2 Bar Sine");
}

uint8_t MoveBarSinEffect::getModeId() const {
    return FX_MODE_MOVE_BAR_SIN;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_MOVE_BAR_SIN, MoveBarSinEffect)