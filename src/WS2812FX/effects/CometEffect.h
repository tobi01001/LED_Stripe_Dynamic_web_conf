#ifndef COMET_EFFECT_H
#define COMET_EFFECT_H

#include "../Effect.h"

/**
 * @brief Comet effect - creates a moving comet with trailing tail
 * 
 * This effect creates a comet-like object that moves linearly across the LED strip
 * from start to end, then repeats. The comet has a bright head with a fading tail
 * created by the fade_out function. The color gradually shifts based on position.
 * 
 * Key features:
 * - Linear movement from start to end of strip
 * - Proportional comet width based on strip length
 * - Color progression based on position along the strip
 * - Fade trailing effect for realistic comet appearance
 * - Minimal external dependencies - only requires timebase tracking
 * - Continuous animation that automatically loops
 */
class CometEffect : public Effect {
private:
    uint32_t timebase;  ///< Time reference for consistent animation timing
    
public:
    CometEffect() = default;
    virtual ~CometEffect() = default;

    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
};

#endif // COMET_EFFECT_H