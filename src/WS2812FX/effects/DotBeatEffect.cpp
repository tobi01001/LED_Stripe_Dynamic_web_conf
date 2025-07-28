#include "DotBeatEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool DotBeatEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize effect-specific state
    lastBeat88 = 0;
    numBars = 0;
    barStates = nullptr;
    
    return true;
}

void DotBeatEffect::initializeBars(WS2812FX* strip) {
    auto seg = strip->getSegment();
    if (!seg) return;
    
    // Clean up existing allocation
    if (barStates) {
        delete[] barStates;
        barStates = nullptr;
    }
    
    // Set number of bars from segment setting, default to 3
    numBars = seg->numBars;
    if (numBars == 0) numBars = 3;
    
    // Allocate memory for bar states  
    barStates = new BarState[numBars];
    if (!barStates) {
        numBars = 0;
        return;
    }
    
    // Initialize each bar with random parameters
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < numBars; i++) {
        // Create varied speeds by dividing and multiplying beat88
        uint8_t divisor = random8(MIN_BEAT, MAX_BEAT_MULTIPLIER);
        uint8_t multiplier = random8(MIN_SPEED_MULTIPLIER, MAX_SPEED_MULTIPLIER);
        barStates[i].beat = max((uint16_t)((seg->beat88 / divisor) * multiplier), seg->beat88);
        
        // Set timebase and initialization flags
        barStates[i].timebase = currentTime;
        barStates[i].newBase = false;
        
        // Distribute colors across spectrum based on bar index
        barStates[i].colorOffset = random8(i * (255 / numBars), (i + 1) * (numBars));
    }
    
    lastBeat88 = seg->beat88;
}

void DotBeatEffect::updateBarSpeeds(WS2812FX* strip) {
    auto seg = strip->getSegment();
    if (!seg || !barStates) return;
    
    // Update speeds when beat88 changes
    for (uint8_t i = 0; i < numBars; i++) {
        uint8_t divisor = random8(MIN_BEAT, MAX_BEAT_MULTIPLIER);
        uint8_t multiplier = random8(MIN_SPEED_MULTIPLIER, MAX_SPEED_MULTIPLIER);
        barStates[i].beat = max((uint16_t)((seg->beat88 / divisor) * multiplier), seg->beat88);
    }
    
    lastBeat88 = seg->beat88;
}

uint16_t DotBeatEffect::calculateBarPosition(uint8_t barIndex, uint8_t waveType, WS2812FX* strip) {
    if (!barStates || barIndex >= numBars || !strip) return 0;
    
    // Calculate beat position using bar's individual timing
    uint16_t beatPosition = beat88(barStates[barIndex].beat, barStates[barIndex].timebase);
    
    // Apply different wave functions based on bar index
    uint16_t wavePosition;
    switch (waveType) {
        case 0:
            wavePosition = EffectHelper::triwave16(beatPosition);
            break;
        case 1:
            wavePosition = EffectHelper::quadwave16(beatPosition);
            break;
        case 2:
            wavePosition = EffectHelper::cubicwave16(beatPosition);
            break;
        default:
            wavePosition = EffectHelper::quadwave16(beatPosition);
            break;
    }
    
    return wavePosition;
}

void DotBeatEffect::updateBarColor(uint8_t barIndex, uint16_t position, WS2812FX* strip) {
    if (!barStates || barIndex >= numBars) return;
    
    auto runtime = strip->getSegmentRuntime();
    if (!runtime) return;
    
    // Check if bar has reached the start position (completed a cycle)
    if (position == runtime->start * 16) {
        // Reset timebase if needed
        if (barStates[barIndex].newBase) {
            barStates[barIndex].timebase = millis();
            barStates[barIndex].newBase = false;
        }
        
        // Randomize beat speed slightly for variation
        int16_t speedDelta = (int16_t)256 - (int16_t)random16(0, 512);
        barStates[barIndex].beat = max((uint16_t)(barStates[barIndex].beat + speedDelta), 
                                     strip->getSegment()->beat88);
        
        // Clamp beat values to reasonable range
        if (barStates[barIndex].beat <= 256) {
            barStates[barIndex].beat = 256;
        }
        if (barStates[barIndex].beat >= 65535 - 512) {
            barStates[barIndex].beat = 65535 - 512;
        }
        
        // Update color using helper function for wheel-based color cycling
        barStates[barIndex].colorOffset = EffectHelper::get_random_wheel_index(
            barStates[barIndex].colorOffset, 64);
    } else {
        barStates[barIndex].newBase = true;
    }
}

uint16_t DotBeatEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer using helper
    if (!EffectHelper::validateStripPointer(strip)) {
        return 1000;
    }
    
    // Access segment data for configuration
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    if (!seg || !runtime) {
        return strip->getStripMinDelay();
    }
    
    // Initialize bars on first run or if numBars changed
    if (!barStates || numBars != seg->numBars) {
        initializeBars(strip);
    }
    
    // Update speeds if beat88 changed
    if (lastBeat88 != seg->beat88) {
        updateBarSpeeds(strip);
    }
    
    // Apply background fade
    EffectHelper::applyFadeEffect(strip, FADE_AMOUNT);
    
    // Process each bar
    for (uint8_t i = 0; i < numBars && barStates; i++) {
        // Calculate wave type based on bar index
        uint8_t waveType = i % 3;
        
        // Calculate bar position using appropriate wave function
        uint16_t wavePosition = calculateBarPosition(i, waveType, strip);
        
        // Map wave position to strip coordinates
        uint16_t barPosition = map(wavePosition, 
                                 (uint16_t)0, (uint16_t)65535,
                                 (uint16_t)(runtime->start * 16), 
                                 (uint16_t)(runtime->stop * 16 - BAR_WIDTH * 16));
        
        // Update bar color and timing
        updateBarColor(i, barPosition, strip);
        
        // Draw the bar
        strip->drawFractionalBar(barPosition, BAR_WIDTH, *strip->getCurrentPalette(),
                               barStates[i].colorOffset, 255, false, 1);
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* DotBeatEffect::getName() const {
    return F("Dots");
}

uint8_t DotBeatEffect::getModeId() const {
    return FX_MODE_DOT_BEAT;
}

void DotBeatEffect::cleanup() {
    if (barStates) {
        delete[] barStates;
        barStates = nullptr;
    }
    numBars = 0;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_DOT_BEAT, DotBeatEffect)