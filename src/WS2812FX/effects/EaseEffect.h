#ifndef EASE_EFFECT_H
#define EASE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Ease effect - two moving "comets" with antialiasing
 * 
 * Creates smooth moving points of light that ease in and out from the center,
 * with optional random sparkles.
 */
class EaseEffect : public Effect {
public:
    EaseEffect() = default;
    virtual ~EaseEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Effect-specific constants
    static const uint8_t WIDTH = 3; // Number of pixels for antialiased bar
    
    // Effect-specific state variables (instead of using shared mode variables)
    bool initialized = false;
    uint32_t timebase = 0;
    uint16_t beat = 0;
    uint16_t oldbeat = 0;
    uint16_t p_lerp = 0;
    bool trigger = false;
};

#endif // EASE_EFFECT_H