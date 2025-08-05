#include "FireworkRocketEffectSimple.h"
#include <FastLED.h>

// Implementation of Effect interface for FireworkRocketEffectSimple

bool FireworkRocketEffectSimple::init(WS2812FX* strip) {
    standardInit(strip);
    for (uint8_t i = 0; i < MAX_ROCKETS; i++) {
        rockets[i].active = false;
        rockets[i].explodeTime = 0;
    }
    return true;
}

uint16_t FireworkRocketEffectSimple::update(WS2812FX* strip) {
    uint32_t now = millis();
    for (uint8_t i = 0; i < MAX_ROCKETS; i++) {
        SimpleRocket& r = rockets[i];
        // Launch logic
        if (!r.active && r.explodeTime == 0 && random8() < 8) {
            r.pos_mm = 0;
            r.acc_mm_per_ms2 = getGravityScaled(strip);
            r.vel_mm_per_ms = sqrt(-2.0 * r.acc_mm_per_ms2 * getSegmentLengthMM(strip)) * (0.85 + 0.3 * (random8() / 255.0));
            r.launch_time = now;
            r.active = true;
            r.color_index = random8();
            r.brightness = random8(128, 255);
        }
        // Flight
        double dt = (double)(now - r.launch_time);
        r.pos_mm = r.vel_mm_per_ms * dt + 0.5 * r.acc_mm_per_ms2 * dt * dt;
        double vel = r.vel_mm_per_ms + r.acc_mm_per_ms2 * dt;
        uint16_t segment_start = strip->getSegmentRuntime()->start;
        uint16_t segment_stop = strip->getSegmentRuntime()->stop;
        uint16_t fracPos = map(r.pos_mm, 0.0, getSegmentLengthMM(strip), segment_start * 16.0, segment_stop * 16.0);
        
        if (r.active) {
            // Explosion
            if (vel < 0.2 && r.explodeTime == 0) {
                r.explodeTime = 30 + random8(20);
                r.active = false;
            }
            // Out of bounds
            if (r.pos_mm < 0) {
                r.pos_mm = 0;
                r.active = false;
            }
        }
        // Rendering
        if (r.active) {
            drawRocketTrail(strip, fracPos, r.color_index, r.brightness);
        } else if (r.explodeTime > 0) {
            drawExplosion(strip, fracPos, r.color_index, r.brightness, r.explodeTime);
            r.explodeTime--;
        }
        r.prev_fract_pos = fracPos;
    }
    return strip->getStripMinDelay();
}

void FireworkRocketEffectSimple::drawRocketTrail(WS2812FX* strip, uint16_t fracPos, uint8_t color_index, uint8_t brightness) {
    uint16_t segment_start = strip->getSegmentRuntime()->start;
    uint16_t segment_stop = strip->getSegmentRuntime()->stop;
    double ledPos = fracPos / 16.0;
    uint16_t ledIdx = (uint16_t)ledPos;
    double frac = ledPos - ledIdx;
    strip->fade_out(64);
    // Draw fractional brightness to two adjacent LEDs
    if (ledIdx >= segment_start && ledIdx <= segment_stop) {
        uint8_t b1 = (1.0 - frac) * brightness;
        strip->leds[segment_start + ledIdx] += ColorFromPalette(*strip->getCurrentPalette(), color_index, b1);
    }
    if ((ledIdx + 1) >= segment_start && (ledIdx + 1) <= segment_stop) {
        uint8_t b2 = frac * brightness;
        strip->leds[segment_start + ledIdx + 1] += ColorFromPalette(*strip->getCurrentPalette(), color_index, b2);
    }
}

void FireworkRocketEffectSimple::drawExplosion(WS2812FX* strip, uint16_t fracPos, uint8_t color_index, uint8_t brightness, uint16_t explodeTime) {
    uint16_t segment_start = strip->getSegmentRuntime()->start;
    uint16_t segment_stop = strip->getSegmentRuntime()->stop;
    double ledPos = fracPos / 16.0;
    uint16_t ledIdx = (uint16_t)ledPos;
    double frac = ledPos - ledIdx;
    strip->fade_out(32);
    for (int8_t d = -2; d <= 2; d++) {
        int16_t idx = segment_start + ledIdx + d;
        if (idx >= segment_start && idx <= segment_stop) {
            uint8_t fade = brightness * (5 - abs(d)) / 5 * (explodeTime) / 50;
            // Fractional blending for explosion center
            uint8_t b1 = (1.0 - frac) * fade;
            uint8_t b2 = frac * fade;
            strip->leds[idx] += ColorFromPalette(*strip->getCurrentPalette(), color_index + d * 8, b1);
            if ((idx + 1) <= segment_stop) {
                strip->leds[idx + 1] += ColorFromPalette(*strip->getCurrentPalette(), color_index + d * 8, b2);
            }
        }
    }
    // Apply blur only to a window around the explosion center
    int margin = 8;
    int blur_start = max((int)segment_start, (int)ledIdx - margin);
    int blur_end = min((int)segment_stop, (int)ledIdx + margin);
    int blur_count = blur_end - blur_start + 1;
    if (blur_count > 1) {
        blur1d(strip->leds + blur_start, blur_count, 64);
    }
}
#ifdef DEF_FX_MODE_FIREWORKROCKETSIMPLE
// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETSIMPLE, FireworkRocketEffectSimple)
#endif