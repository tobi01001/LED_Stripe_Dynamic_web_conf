#include "FireworkRocketEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"


// Gravity scaling: at beat88=1000, gravity = 16*60*9.81/1000^2 per ms^2 (scaled for 16xLED units)
// We'll use a base gravity of 16*60*10/1000000 = 0.0096 per ms^2, but as integer math:
// For each ms, v += gravity; pos += v; (gravity negative)
// We'll use gravity = -((16*60*10)/1000000) * (beat88/1000)
// To avoid floats, precompute: 16*60*10 = 9600, so gravity = -9600 * beat88 / 1000000
// To keep more precision, use gravity = -((int32_t)9600 * beat88) / 1000000

bool FireworkRocketEffect::init(WS2812FX* strip) {
    if (!standardInit(strip)) {
        return false;
    }
    numRockets = min(strip->getSegment()->numBars, (uint8_t)32);
    uint32_t currentTime = millis();
    for (uint8_t i = 0; i < numRockets; i++) {
        rockets[i].timebase = currentTime;
        rockets[i].color_index = map(i, (uint16_t)0, (uint16_t)(numRockets-1), (uint16_t)0, (uint16_t)255);
        rockets[i].pos = 0;         // 16xLED units
        rockets[i].prev_pos = 0;
        rockets[i].v0 = 0;          // velocity in 16xLED/ms
        rockets[i].v = 0;
        rockets[i].v_explode = 0;
        rockets[i].explodeTime = 0;
        rockets[i].brightness = 0;
    }
    initialized = true;
    return true;
}

uint16_t FireworkRocketEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    
    applyGlobalFade(strip);
    const uint32_t currentTime = millis();
    const int16_t gravity = getGravity(strip); // integer gravity in 16xLED/ms^2
    const uint16_t segmentLength = calculateSegmentLength(strip); // in 16xLED units
    const int16_t maxVelocity = calculateMaxVelocity(strip, gravity, segmentLength); // in 16xLED/ms
    const uint16_t maxBlendWidth = EffectHelper::calculateProportionalWidth(strip, 2, 1);
    for (uint8_t i = 0; i < numRockets; i++) {
        // Only relaunch if not in explosion
        if (rockets[i].pos <= 0 && rockets[i].explodeTime == 0 && shouldRelaunch(i)) {
            initializeRocketLaunch(i, strip, currentTime, maxVelocity);
        }
        // If flying, update physics and render
        if (rockets[i].pos > 0 && rockets[i].explodeTime == 0) {
            updateRocketPhysics(i, strip, currentTime, gravity, segmentLength, maxVelocity);
            if (rockets[i].v > rockets[i].v_explode) {
                renderLaunchPhase(i, strip, segmentLength);
            } else {
                renderExplosionPhase(i, strip, maxBlendWidth);
            }
        } else if (rockets[i].explodeTime > 0) {
            // If in explosion phase, just render explosion and count down
            renderExplosionPhase(i, strip, maxBlendWidth);
            rockets[i].explodeTime--;
        }
    }
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FireworkRocketEffect::getName() const {
    return F("Firework Rocket");
}

uint8_t FireworkRocketEffect::getModeId() const {
    #ifdef DEF_FX_MODE_FIREWORKROCKETS
    return FX_MODE_FIREWORKROCKETS;
    #else
    return 255;
    #endif
}

int16_t FireworkRocketEffect::calculateMaxVelocity(WS2812FX* strip, int16_t gravity, uint16_t segmentLength) const {
    // Integer kinematics: v0 = sqrt(-2 * gravity * segmentLength)
    // gravity is negative, segmentLength in 16xLED units
    // To avoid sqrt on negative, use abs(gravity)
    uint32_t v0sq = 2UL * (uint32_t)abs(gravity) * (uint32_t)segmentLength;
    uint16_t v0 = sqrt16(v0sq); // sqrt16 is integer sqrt
    return v0;
}

int16_t FireworkRocketEffect::getGravity(WS2812FX* strip) const {
    // Integer gravity: at beat88=1000, gravity = -((16*60*10)/1000000) = -0.0096 per ms^2
    // We'll use gravity = -((int32_t)9600 * beat88) / 1000000
    uint16_t beat88 = strip->getSegment()->beat88;
    int32_t gravity = -((int32_t)9600 * (int32_t)beat88) / 1000000;
    if (gravity == 0) gravity = -1; // always at least -1
    return (int16_t)gravity;
}

uint16_t FireworkRocketEffect::calculateSegmentLength(WS2812FX* strip) const {
    // Return segment length in 16xLED units
    return strip->getSegmentRuntime()->length * 16;
}

void FireworkRocketEffect::updateRocketPhysics(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, 
                                             int16_t gravity, uint16_t segmentLength, int16_t maxVelocity) {
    RocketData& rocket = rockets[rocketIndex];
    uint32_t deltaTime = (uint32_t)(currentTime - rocket.timebase); // ms
    // Integer kinematics: pos = v0 * t + (g/2) * t^2
    int32_t pos = (int32_t)rocket.v0 * (int32_t)deltaTime + ((int32_t)gravity * (int32_t)deltaTime * (int32_t)deltaTime) / 2;
    rocket.pos = pos;
    rocket.v = rocket.v0 + (int16_t)((int32_t)gravity * (int32_t)deltaTime);

    // Explosion trigger: when velocity is downward and position is near the top
    if (rocket.v < rocket.v_explode || rocket.pos >= segmentLength) {
        if (rocket.explodeTime == 0) {
            rocket.explodeTime = 80 + random8(80); // explosion duration
        }
        rocket.v0 = 0;
        rocket.v = 0;
        rocket.timebase = currentTime;
        // Clamp to top
        if (rocket.pos > segmentLength) rocket.pos = segmentLength;
    }

    // Out of bounds (below ground)
    if (rocket.pos < 0) {
        rocket.pos = 0;
        rocket.v0 = 0;
        rocket.v = 0;
        rocket.prev_pos = 0;
        rocket.timebase = currentTime;
        rocket.explodeTime = 0;
    }
}

void FireworkRocketEffect::renderLaunchPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t segmentLength) {
    RocketData& rocket = rockets[rocketIndex];
    uint16_t fracPos = (uint16_t)rocket.pos;
    uint16_t width = 1;
    if (rocket.prev_pos != 0) {
        uint16_t prevFrac = (uint16_t)rocket.prev_pos;
        width = max(abs(fracPos - prevFrac) / 16, 1);
        width = min(width, (uint16_t)5);
    }
    rocket.prev_pos = fracPos;
    strip->drawFractionalBar(
        fracPos,                 // Fractional LED position
        width,                   // Width for motion blur
        HeatColors_p,            // Heat palette for rocket trail
        64,                      // Color index in heat palette
        rocket.brightness,       // Full brightness for rocket
        true,                    // Additive blending
        1                        // Single pixel precision
    );
}

void FireworkRocketEffect::renderExplosionPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t maxBlendWidth) {
    const RocketData& rocket = rockets[rocketIndex];
    uint16_t fracPos = (uint16_t)rocket.pos;
    uint16_t blendWidth = maxBlendWidth - 3;
    if ((strip->getSegmentRuntime()->stop - fracPos / 16) < blendWidth / 2 + 3) {
        blendWidth = (strip->getSegmentRuntime()->stop - fracPos / 16) * 2;
    }
    if (fracPos / 16 < blendWidth / 2) {
        blendWidth = (fracPos / 16) * 2;
    }
    uint8_t renderBrightness = map(rocket.brightness, 0, 48, 0, 255);
    if (rocket.explodeTime > 10) {
        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, 255, true, 0);
        CRGB centerColor = strip->leds[fracPos / 16] + CRGB(0x202020);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(centerColor), 0, 255, true, 0);
        addExplosionSparks(rocketIndex, strip, fracPos, blendWidth/4);
        uint16_t ledArraySize = strip->getSegmentRuntime()->length;
        uint16_t startIndex = fracPos / 16 + 1;
        if (startIndex + blendWidth <= ledArraySize) {
            blur1d(&strip->leds[startIndex], blendWidth, 172);
        } else {
            uint16_t adjustedBlendWidth = ledArraySize - startIndex;
            if (adjustedBlendWidth > 0) {
                blur1d(&strip->leds[startIndex], adjustedBlendWidth, 172);
            }
        }
    } else if (rocket.explodeTime > 0) {
        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);
        blur1d(&strip->leds[fracPos / 16], blendWidth, 128);
    } else {
        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);
        blur1d(&strip->leds[fracPos / 16], blendWidth, 64);
    }
}

void FireworkRocketEffect::applyGlobalFade(WS2812FX* strip) {
    // Apply global fading using EffectHelper with beat88-based calculation
    uint8_t fadeAmount = EffectHelper::safeMapuint16_t(strip->getSegment()->beat88, 0, 6000, 24, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
}

bool FireworkRocketEffect::shouldRelaunch(uint8_t rocketIndex) const {
    const RocketData& rocket = rockets[rocketIndex];
    // Relaunch if velocity is zero and random chance triggers (higher probability)
    return (rocket.v0 == 0 && random8() < 2);
}

void FireworkRocketEffect::initializeRocketLaunch(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, int16_t maxVelocity) {
    RocketData& rocket = rockets[rocketIndex];
    // Use 85-99% of maxVelocity for variety
    int16_t v0 = (int16_t)((int32_t)maxVelocity * (int32_t)random(850, 990) / 1000);
    rocket.v0 = v0;
    rocket.v = v0;
    rocket.pos = 0; // start at ground
    rocket.prev_pos = 0;
    rocket.timebase = currentTime;
    rocket.color_index = EffectHelper::get_random_wheel_index(rocket.color_index, 32);
    rocket.brightness = random8(12, 48);
    rocket.explodeTime = 0;
    // v_explode = v0 * random(20, 40) / 100 (explode at 20-40% of initial velocity)
    rocket.v_explode = (int16_t)((int32_t)v0 * (int32_t)random8(20, 40) / 100);
}

void FireworkRocketEffect::addExplosionSparks(uint8_t rocketIndex, WS2812FX* strip, uint16_t explosionPos, uint16_t sparkRadius) {
    const RocketData& rocket = rockets[rocketIndex];

    // Generate 1-4 sparks around the explosion
    uint8_t numSparks = random8(1, 3);
    for (uint8_t i = 0; i < numSparks; i++) {
        // Random offset from explosion center (within spark radius)
        int16_t sparkOffset = random16(sparkRadius * 2) - sparkRadius;
        int16_t ledIndexRaw = (int16_t)(explosionPos / 16) + sparkOffset;

        // Ensure spark is within strip bounds and ledIndex is non-negative
        uint16_t stripStart = strip->getSegmentRuntime()->start;
        uint16_t stripEnd = strip->getSegmentRuntime()->stop;
        uint16_t ledIndex = constrain(ledIndexRaw, stripStart, stripEnd);

        // Random spark brightness (30-80% of main explosion)
        uint8_t sparkBrightness = random8(76, 206);

        // Random spark color variation (within 64 steps of main color)
        uint8_t sparkColorIndex = rocket.color_index + random8(128) - 64;

        CRGB sparkColor = ColorFromPalette(*strip->getCurrentPalette(), sparkColorIndex, sparkBrightness);

        // Add spark to existing LED color (single frame spark)
        strip->leds[ledIndex] |= sparkColor;

        // Occasionally add a white hot spark for extra sparkle
        if (random8() < 64) {
            CRGB whiteSparkColor = CRGB::White;
            whiteSparkColor.nscale8(sparkBrightness / 2);
            strip->leds[ledIndex] |= whiteSparkColor;
        }
    }
}

#ifdef DEF_FX_MODE_FIREWORKROCKETS
// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETS, FireworkRocketEffect)
#endif