#pragma once
#include "../Effect.h"
#include "../WS2812FX_FastLed.h"
#include <FastLED.h>

class FireworkRocketEffectInt : public Effect {
public:
    FireworkRocketEffectInt() = default;
    virtual ~FireworkRocketEffectInt() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override { return F("Firework Rocket Int"); }
    uint8_t getModeId() const override { 
        #ifdef DEF_FX_MODE_FIREWORKROCKETEFFECT_INT
        return FX_MODE_FIREWORKROCKETS_INT; }
        #else
        return 255; }
        #endif

private:
    static constexpr uint8_t MAX_ROCKETS = 8;
    static constexpr uint8_t MAX_EXPLODE_TIME = 240;
    static constexpr uint8_t MIN_BLUR = 32;
    static constexpr uint8_t MAX_BLUR = 128;
    static constexpr uint16_t LEDS_PER_METER = 60;
    static constexpr uint16_t LED_FRAC = 16; // 16x fractional LED positions
    static constexpr uint32_t MM_PER_LED_FRAC = 1000UL * LED_FRAC / LEDS_PER_METER;

    struct IntRocket {
        bool active = false;
        uint8_t color_index = 0;
        uint8_t brightness = 0;
        uint16_t explodeTime = 0;
        int16_t prev_pos = 0; // Previous position for motion blur (16xLED)
        uint32_t launch_time = 0;
        int16_t pos = 0; // 16xLED units
        int16_t vel = 0; // 16xLED/ms
        uint16_t explosionTrigger = 0; // 16xLED/ms
    };

    uint8_t maxRockets = MAX_ROCKETS;
    uint16_t blendWidth = 16;
    IntRocket rockets[MAX_ROCKETS];
    int16_t gravity = -10; // 16xLED/ms^2 (negative for downward)
    uint16_t maxVelocity = 0; // 16xLED/ms

    void drawRocketTrail(WS2812FX* strip, int16_t pos, IntRocket& r);
    void drawExplosion(WS2812FX* strip, int16_t pos, IntRocket& r);
    void initializeRocket(WS2812FX* strip, IntRocket& rocket);
    uint16_t calculateMaxVelocityInt(int16_t gravity, uint16_t distance, uint16_t margin = 0);
    int16_t getGravity(WS2812FX* strip);
    uint16_t sqrt32(uint32_t x);
};
