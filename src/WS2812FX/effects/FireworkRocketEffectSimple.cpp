#include "FireworkRocketEffectSimple.h"
#include <FastLED.h>
#include "../EffectHelper.h"

// Implementation of Effect interface for FireworkRocketEffectSimple

bool FireworkRocketEffectSimple::init(WS2812FX* strip) {
    standardInit(strip);
    blendWidth = strip->getSegmentRuntime()->length / 2 ; // Calculate blend width from segment runtime
    if(blendWidth < 4) blendWidth = 4;

    gravity = EffectHelper::getGravity(strip);
    maxVelocity = EffectHelper::calculateMaxVelocity(strip, gravity, blendWidth/2);
    maxRockets = min(strip->getSegment()->numBars, MAX_ROCKETS);

    for (uint8_t i = 0; i < maxRockets; i++) {
        initializeRocket(rockets[i]);
        rockets[i].active = false; // Randomly activate some rockets
    }
    setInitialized(true);
    return true;
}

uint16_t FireworkRocketEffectSimple::update(WS2812FX* strip) {
    
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    strip->fade_out(32); // Less aggressive fade for better visibility
    uint32_t now = millis();
    for (uint8_t i = 0; i < maxRockets; i++) {
        SimpleRocket& r = rockets[i];
        // Launch logic
        if (!r.active && r.pos_mm < EffectHelper::MM_PER_LED && random8() < 4) {
            initializeRocket(r);
            r.active = true;
        }
        // Flight
        double dt = (double)(now - r.launch_time);
        r.pos_mm = (gravity / 2.0 * dt + r.vel_mm_per_ms) * dt;
        double vel = r.vel_mm_per_ms + gravity * dt;
        uint16_t segment_start = strip->getSegmentRuntime()->start;
        uint16_t segment_stop = strip->getSegmentRuntime()->stop;
        double ledPos = r.pos_mm / EffectHelper::MM_PER_LED;
        uint16_t ledIdx = (uint16_t)ledPos;
        // Clamp ledIdx to segment
        if (ledIdx < segment_start) ledIdx = segment_start;
        if (ledIdx > segment_stop) ledIdx = segment_stop;

        // Explosion trigger: when velocity is downward and position is near the top
        if (r.active && (vel < r.explosionTrigger || r.pos_mm >= (segment_stop - segment_start) * EffectHelper::MM_PER_LED)) {
            r.active = false;
            r.explodeTime = (MAX_EXPLODE_TIME/3 * 2) + random8(MAX_EXPLODE_TIME / 3);
        }
        // Out of bounds
        if (r.pos_mm < 0) {
            r.pos_mm = 0;        
            r.active = false;
        }

        // Rendering
        if (r.active) {
            drawRocketTrail(strip, r.pos_mm, r);
        } else if (r.pos_mm >= EffectHelper::MM_PER_LED) {
            drawExplosion(strip, r.pos_mm, r);
        }
    }
    return strip->getStripMinDelay();
}

void FireworkRocketEffectSimple::initializeRocket(SimpleRocket& rocket) {
    rocket.pos_mm = 0;
    rocket.vel_mm_per_ms = maxVelocity * (0.85 + 0.15 * (random8() / 255.0));
    rocket.explosionTrigger = maxVelocity * 0.3 * (0.85 + 0.15 * (random8() / 255.0));
    rocket.launch_time = millis() - 1; // <-- ensures dt > 0 on first update
    rocket.color_index = EffectHelper::get_random_wheel_index(rocket.color_index, 32);
    rocket.brightness = random8(192, 255);
    rocket.prev_pos = 0;
}

void FireworkRocketEffectSimple::drawRocketTrail(WS2812FX* strip, double pos, SimpleRocket& r) {
    // Convert position to LED coordinates
    uint16_t currentPos = (uint16_t)(pos);
    uint16_t prevPosition = r.prev_pos;
    // Calculate width for motion blur effect
    uint8_t width = 1;
    if (currentPos > prevPosition) {
        width = (uint8_t)((double)(currentPos - prevPosition) / EffectHelper::MM_PER_LED);
    } else {
        width = (uint8_t)((double)(prevPosition - currentPos) / EffectHelper::MM_PER_LED);
    }
    if (!width)
    {
        width = 1;
    }
    
    // Render kernel using the strip's drawing function with current palette
    strip->drawFractionalBar(
        currentPos,                                              // Position
        width,                                                   // Width for motion blur
        HeatColors_p,
        64,
        //*strip->getCurrentPalette(),                             // Current color palette
        //r.color_index,                                           // Color index in palette
        196,                                                     // Full brightness
        true,                                                    // Additive blending
        1                                                        // Single pixel precision
    );
    // Update previous position for next frame
    r.prev_pos = currentPos;
}

void FireworkRocketEffectSimple::drawExplosion(WS2812FX* strip, double pos, SimpleRocket& r) {
    // Convert position to LED coordinates
    uint16_t currentPos = (uint16_t)(pos);
    uint16_t prevPosition = r.prev_pos;
    // Calculate width for motion blur effect
    uint8_t width = 1;
    if (currentPos > prevPosition) {
        width = (uint8_t)((double)(currentPos - prevPosition) / EffectHelper::MM_PER_LED);
    } else {
        width = (uint8_t)((double)(prevPosition - currentPos) / EffectHelper::MM_PER_LED);
    }
    if (width < 6)
    {
        width = 6;
    }
    uint16_t segment_start = strip->getSegmentRuntime()->start;
    uint16_t segment_stop = strip->getSegmentRuntime()->stop;
    uint8_t explodeTime = r.explodeTime > MAX_EXPLODE_TIME ? MAX_EXPLODE_TIME : r.explodeTime;
    uint8_t blur = 172;
    if(explodeTime < MAX_EXPLODE_TIME/2) {
        blur = (uint8_t)(16 + ((172 - 16) * explodeTime) / (MAX_EXPLODE_TIME/2));
    }
    // Apply blur only to a window around the explosion center
    int margin = blendWidth;
    int blur_start = max((int)segment_start, (int)currentPos - margin);
    int blur_end = min((int)segment_stop, (int)currentPos + margin);
    int blur_count = blur_end - blur_start + 1;
    if(explodeTime < MAX_EXPLODE_TIME/2) {
        r.brightness = r.brightness > 3 ? r.brightness - 3 : 0;
    }
    // Render kernel using the strip's drawing function with current palette

    strip->drawFractionalBar(
        currentPos,                                              // Position
        width,                                                   // Width for motion blur
        *strip->getCurrentPalette(),                             // Current color palette
        r.color_index,                                           // Color index in palette
        r.brightness,                                            // Full brightness
        true,                                                    // Additive blending
        0                                                        // No blur
    );
    if (blur_count > 1) {
        blur1d(strip->leds + blur_start, blur_count, blur);
    }
    CRGB centerColor = strip->leds[r.prev_pos / 16] + CRGB(0x202020);
    if (r.brightness > 128 && r.explodeTime > 0) { // Add white hot center
        strip->drawFractionalBar(currentPos, 3, CRGBPalette16(centerColor), 0, 255, true, 0);
    }
    else
    {
        strip->drawFractionalBar(currentPos, 3, CRGBPalette16(centerColor), 0, r.brightness, true, 0);
    }

    // Update previous position for next frame
    r.prev_pos = currentPos;
    // Decrease explosion time
    if(r.explodeTime > 0) {
        r.explodeTime--;
    }
}

#ifdef DEF_FX_MODE_FIREWORKROCKETSIMPLE
// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETSIMPLE, FireworkRocketEffectSimple)
#endif