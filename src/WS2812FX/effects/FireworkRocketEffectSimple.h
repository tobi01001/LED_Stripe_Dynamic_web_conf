#pragma once
#include "../Effect.h"
#include "../WS2812FX_FastLed.h"
#include <FastLED.h>

struct SimpleRocket {
    double pos_mm = 0;
    uint16_t prev_fract_pos = 0;
    double vel_mm_per_ms = 0;
    double acc_mm_per_ms2 = 0;
    uint32_t launch_time = 0;
    bool active = false;
    uint8_t color_index = 0;
    uint8_t brightness = 0;
    uint16_t explodeTime = 0;
};

class FireworkRocketEffectSimple : public Effect {
public:
    static constexpr uint8_t MAX_ROCKETS = 8;
    SimpleRocket rockets[MAX_ROCKETS];
    double gravity = -0.00981;

    FireworkRocketEffectSimple() = default;
    virtual ~FireworkRocketEffectSimple() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override { return F("Simple Firework Rocket"); }
    uint8_t getModeId() const override { return 250; } // Example mode id

private:
    inline double getSegmentLengthMM(WS2812FX* strip) const { return strip->getSegmentRuntime()->length * (1000.0 / 60.0); }
    inline double getGravityScaled(WS2812FX* strip) const { return gravity * (strip->getBeat88()/1000.0); } // base speed is 1000 equals gravity
    void drawRocketTrail(WS2812FX* strip, uint16_t fracPos, uint8_t color_index, uint8_t brightness);
    void drawExplosion(WS2812FX* strip, uint16_t fracPos, uint8_t color_index, uint8_t brightness, uint16_t explodeTime);
};
