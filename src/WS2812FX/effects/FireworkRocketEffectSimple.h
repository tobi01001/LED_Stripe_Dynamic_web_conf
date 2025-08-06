#pragma once
#include "../Effect.h"
#include "../WS2812FX_FastLed.h"
#include <FastLED.h>

struct SimpleRocket {
    bool active = false;
    uint8_t color_index = 0;
    uint8_t brightness = 0;
    uint16_t explodeTime = 0;
    uint16_t prev_pos = 0; // Previous position for motion blur
    uint32_t launch_time = 0;
    double pos_mm = 0;
    double vel_mm_per_ms = 0;
    double explosionTrigger = 0;
};

class FireworkRocketEffectSimple : public Effect {
public:
    
    

    FireworkRocketEffectSimple() = default;
    virtual ~FireworkRocketEffectSimple() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override { return F("Simple Firework Rocket"); }
    uint8_t getModeId() const override { return FX_MODE_FIREWORKROCKETSIMPLE; } // Example mode id

private:
    static constexpr uint8_t MAX_ROCKETS = 8;
    static constexpr uint8_t MAX_EXPLODE_TIME = 240;
    uint16_t blendWidth = 16; ///< Millimeters per LED
    uint8_t maxRockets = MAX_ROCKETS;
    SimpleRocket rockets[MAX_ROCKETS];
    double gravity = -0.00981; ///< Gravitational acceleration in mm/ms² (negative for downward)
    double maxVelocity = 0.0;   ///< Maximum velocity for rockets (mm/ms)
    inline double getSegmentLengthMM(WS2812FX* strip) const { return strip->getSegmentRuntime()->length * (1000.0 / 60.0); }
    void drawRocketTrail(WS2812FX* strip, double pos, SimpleRocket& r);
    void drawExplosion(WS2812FX* strip, double pos, SimpleRocket& r);
    void initializeRocket(SimpleRocket& rocket);
};
