#include "FireworkRocketEffectInt.h"
#include <FastLED.h>
#include "../EffectHelper.h"

bool FireworkRocketEffectInt::init(WS2812FX* strip) {
    standardInit(strip);
    blendWidth = strip->getSegmentRuntime()->length / 2;
    if (blendWidth < 4) blendWidth = 4;
    maxRockets = min(strip->getSegment()->numBars, MAX_ROCKETS);
    for (uint8_t i = 0; i < maxRockets; i++) {
        initializeRocket(strip, rockets[i]);
        rockets[i].active = true;
    }
    setInitialized(true);
    return true;
}

uint16_t FireworkRocketEffectInt::update(WS2812FX* strip) {
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000;
        }
    }
    strip->fade_out(32);
    uint32_t now = millis();
    uint16_t segment_len = strip->getSegmentRuntime()->length * LED_FRAC;
    for (uint8_t i = 0; i < maxRockets; i++) {
        IntRocket& r = rockets[i];
        // Launch logic
        if (!r.active && r.pos < LED_FRAC && random8() < 4) {
            initializeRocket(strip, r);
            r.active = true;
            strip->leds[strip->getSegmentRuntime()->length/3 + i] = CRGB::Magenta;
        }
        // Flight
        uint32_t dt = (now - r.launch_time) / 100;
        if(!dt) continue;
        int16_t vel = (int16_t)r.vel + (int16_t)gravity * dt;
        if (vel >= 0) {
            r.pos = r.vel * dt + ((int32_t)gravity * dt * dt) / 2;
            strip->leds[strip->getSegmentRuntime()->length*2/3 + i] = CRGB::Green;
        } else {
            r.pos = r.vel * dt + ((int32_t)gravity * dt * dt) / 2;
            strip->leds[strip->getSegmentRuntime()->length*2/3 + i] = CRGB::Red;
        }
        if (r.pos > (segment_len - LED_FRAC)) r.pos = (segment_len - LED_FRAC);
        // Explosion trigger
        if (r.active && (vel < (int32_t)r.explosionTrigger || r.pos >= (segment_len - LED_FRAC))) {
            r.active = false;
            r.explodeTime = ((uint16_t)MAX_EXPLODE_TIME * 2 / 3) + random8(MAX_EXPLODE_TIME / 3);
            strip->leds[strip->getSegmentRuntime()->length/2 + i] = CRGB::Blue;
        }
        // Out of bounds
        if (r.pos < 0) {
            r.pos = 0;
            //r.active = false;
        }
        // Rendering
        if (r.active) {
            drawRocketTrail(strip, r.pos, r);   
            uint8_t green = map(r.pos, 0, segment_len, 64, 255);
            strip->leds[0 + i] = CRGB(0, green, 0);
        } else if (r.pos >= LED_FRAC) {
            drawExplosion(strip, r.pos, r);
            uint8_t red = map(r.pos, 0, segment_len, 64, 255);
            strip->leds[strip->getSegmentRuntime()->start + maxRockets + i] = CRGB(red, 0, 0);
        }
    }
    return strip->getStripMinDelay();
}

void FireworkRocketEffectInt::initializeRocket(WS2812FX* strip, IntRocket& rocket) {
    gravity = getGravity(strip);
    maxVelocity = calculateMaxVelocityInt(gravity, strip->getSegmentRuntime()->length * LED_FRAC, LED_FRAC * blendWidth / 2);
    rocket.pos = 0;
    rocket.vel = (uint16_t)(maxVelocity * (850 + (random8() % 150)) / 1000); // 85-99% of maxVelocity
    rocket.explosionTrigger = (uint16_t)(maxVelocity * 15 / 100); // 15% of maxVelocity
    rocket.launch_time = millis() - 1;
    rocket.color_index = EffectHelper::get_random_wheel_index(rocket.color_index, 32);
    rocket.brightness = random8(192, 255);
    rocket.prev_pos = 0;
}

void FireworkRocketEffectInt::drawRocketTrail(WS2812FX* strip, int16_t pos, IntRocket& r) {
    
    uint8_t width = 1;
    if (pos > r.prev_pos) {
        width = (uint8_t)(pos - r.prev_pos) / LED_FRAC;
    } else {
        width = (uint8_t)(r.prev_pos - pos) / LED_FRAC;
    }
    if (!width) width = 1;
    strip->drawFractionalBar(
        pos,
        width,
        HeatColors_p,
        64,
        196,
        true,
        1
    );
    r.prev_pos = pos;
}

void FireworkRocketEffectInt::drawExplosion(WS2812FX* strip, int16_t pos, IntRocket& r) {
    
    uint8_t width = 6;
    uint16_t segment_start = strip->getSegmentRuntime()->start;
    uint16_t segment_stop = strip->getSegmentRuntime()->stop;
    uint8_t explodeTime = r.explodeTime > MAX_EXPLODE_TIME ? MAX_EXPLODE_TIME : r.explodeTime;
    uint8_t blur = MAX_BLUR;
    if (explodeTime < MAX_EXPLODE_TIME/3) {
        blur = (uint8_t)(MIN_BLUR + ((MAX_BLUR - MIN_BLUR) * explodeTime) / (MAX_EXPLODE_TIME/3));
    }
    int margin = blendWidth;
    int blur_start = max((int)segment_start, (int)r.prev_pos / LED_FRAC - margin);
    int blur_end = min((int)segment_stop, (int)r.prev_pos / LED_FRAC + margin);
    int blur_count = blur_end - blur_start + 1;
    if (explodeTime < MAX_EXPLODE_TIME/3) {
        r.brightness = r.brightness > 8 ? r.brightness - 8 : 0;
    }
    strip->drawFractionalBar(
        pos,
        width,
        *strip->getCurrentPalette(),
        r.color_index,
        r.brightness,
        true,
        0
    );
    if (blur_count > 1) {
        blur1d(strip->leds + blur_start, blur_count, blur);
    }
    CRGB centerColor = strip->leds[r.prev_pos / LED_FRAC] + CRGB(0x202020);
    if (r.brightness > 128 && r.explodeTime > 0) {
        strip->drawFractionalBar(pos, 3, CRGBPalette16(centerColor), 0, 255, true, 0);
    } else {
        strip->drawFractionalBar(pos, 3, CRGBPalette16(centerColor), 0, r.brightness, true, 0);
    }
    r.prev_pos = pos;
    if (r.explodeTime > 0) {
        r.explodeTime--;
    }
}
uint16_t FireworkRocketEffectInt::calculateMaxVelocityInt(int16_t gravity, uint16_t distance, uint16_t margin) {
    // v0 = sqrt(-2 * gravity * distance)
    // gravity is negative, so use abs(gravity)
    if(distance < 2 || gravity == 0) return 0;
    if(margin > distance) margin = distance - 1;
    uint32_t v0sq = 2 * (uint32_t)abs(gravity) * (uint32_t)(distance - margin);
    uint16_t v0 = sqrt32(v0sq)/100; // sqrt32: integer square root
    return 200; //v0;
}

int16_t FireworkRocketEffectInt::getGravity(WS2812FX* strip) {
    // Integer gravity: at beat88=1000, gravity = -((16*60*10)/1000000) = -0.0096 per ms^2 (16xLED/ms^2)
    // We'll use gravity = -((int32_t)9600 * beat88) / 1000000
    uint16_t beat88 = strip->getSegment()->beat88;
    int32_t gravity = -((int32_t)9600 * (int32_t)beat88) / 1000000;
    if (gravity == 0) gravity = -1; // always at least -1
    return (int16_t)gravity;
}

uint16_t FireworkRocketEffectInt::sqrt32(uint32_t x) {
    uint32_t res = 0;
    uint32_t bit = 1UL << 30;
    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return (uint16_t)res;
}

#ifdef DEF_FX_MODE_FIREWORKROCKETEFFECT_INT
// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETS_INT, FireworkRocketEffectInt)
#endif