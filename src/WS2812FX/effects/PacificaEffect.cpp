#include "PacificaEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

bool PacificaEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Initialize internal state for Pacifica ocean wave effect
    auto runtime = strip->getSegmentRuntime();
    auto seg = strip->getSegment();
    runtime->modeinit = false;
    
    // Initialize wave layer color index starting positions with random values
    state.colorIndexStart1 = random16();
    state.colorIndexStart2 = random16();
    state.colorIndexStart3 = random16();
    state.colorIndexStart4 = random16();
    
    // Initialize timing tracking
    state.lastMillis = millis() - strip->getStripMinDelay();
    
    // Calculate effective speed using EffectHelper safe mapping
    state.effectiveSpeed = calculateEffectiveSpeed(seg->beat88);
    
    return true;
}

uint16_t PacificaEffect::update(WS2812FX* strip) {
    // Access segment and runtime data through strip public getters
    auto seg = strip->getSegment();
    auto runtime = strip->getSegmentRuntime();
    
    // Calculate time delta for consistent animation speed
    uint32_t currentMillis = millis();
    uint32_t deltaMillis = currentMillis - state.lastMillis;
    state.lastMillis = currentMillis;
    
    // Update effective speed if segment settings have changed
    state.effectiveSpeed = calculateEffectiveSpeed(seg->beat88);
    
    // Calculate speed factors for the two primary wave groups
    // Each group has its own speed variation to create natural wave interference
    uint16_t speedFactor1 = beatsin16(max(state.effectiveSpeed / 333, 2), 179, 269);
    uint16_t speedFactor2 = beatsin16(max(state.effectiveSpeed / 250, 2), 179, 269);
    
    // Apply speed factors to time delta for each wave group
    uint32_t deltaMillis1 = (deltaMillis * speedFactor1) / 256;
    uint32_t deltaMillis2 = (deltaMillis * speedFactor2) / 256;
    uint32_t deltaMillis21 = (deltaMillis1 + deltaMillis2) / 2;  // Blended timing
    
    // Update color index starting positions for each wave layer
    // Each layer moves at a different rate and direction to create complex patterns
    state.colorIndexStart1 += (deltaMillis1 * beatsin88(state.effectiveSpeed, 10, 13));
    state.colorIndexStart2 -= (deltaMillis21 * beatsin88((state.effectiveSpeed * 2) / 3, 8, 11));
    state.colorIndexStart3 -= (deltaMillis1 * beatsin88(state.effectiveSpeed / 2, 5, 7));
    state.colorIndexStart4 -= (deltaMillis2 * beatsin88(state.effectiveSpeed / 4, 4, 6));
    
    // Initialize strip with dim background blue-green ocean color
    // This provides the base oceanic atmosphere
    fill_solid(strip->leds, runtime->length, CRGB(2, 6, 10));
    
    // Render four wave layers with different palettes, scales, and speeds
    // Each layer contributes to the overall oceanic effect
    
    // Layer 1: Primary wave pattern with medium scale
    strip->pacifica_layer(pacifica_palette_p1, state.colorIndexStart1,
                         beatsin16(max(state.effectiveSpeed / 333, 2), 11 * 256, 14 * 256),
                         beatsin8(10, 70, 130),
                         0 - beat16(max(state.effectiveSpeed / 3, 2)));
    
    // Layer 2: Secondary wave pattern with smaller scale
    strip->pacifica_layer(pacifica_palette_p2, state.colorIndexStart2,
                         beatsin16(max(state.effectiveSpeed / 240, 2), 6 * 256, 9 * 256),
                         beatsin8(17, 40, 80),
                         beat16(max((state.effectiveSpeed * 2) / 5, 2)));
    
    // Layer 3: Tertiary wave pattern with fixed scale
    strip->pacifica_layer(pacifica_palette_p3, state.colorIndexStart3,
                         max(state.effectiveSpeed / 166, 2) * 256,
                         beatsin8(9, 10, 38),
                         0 - beat16(max(state.effectiveSpeed / 2, 2)));
    
    // Layer 4: Quaternary wave pattern with fixed scale
    strip->pacifica_layer(pacifica_palette_p3, state.colorIndexStart4,
                         max(state.effectiveSpeed / 200, 2) * 256,
                         beatsin8(8, 10, 28),
                         beat16(max((state.effectiveSpeed * 3) / 5, 2)));
    
    // Add whitecap effects where wave patterns constructively interfere
    strip->pacifica_add_whitecaps();
    
    // Deepen blues and greens for enhanced oceanic appearance
    strip->pacifica_deepen_colors();
    
    // Return minimum delay for smooth animation
    return strip->getStripMinDelay();
}

uint16_t PacificaEffect::calculateEffectiveSpeed(uint16_t beat88) {
    // Ensure minimum speed to prevent division by zero and maintain smooth animation
    // Map from beat88 range to effective speed range suitable for wave calculations
    return max(beat88, (uint16_t)780);
}

const __FlashStringHelper* PacificaEffect::getName() const {
    return F("Pacifica - Specific Colors");
}

uint8_t PacificaEffect::getModeId() const {
    return FX_MODE_PACIFICA;
}

// Register this effect with the factory
REGISTER_EFFECT(FX_MODE_PACIFICA, PacificaEffect)