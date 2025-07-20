#ifndef LARSON_SCANNER_EFFECT_H
#define LARSON_SCANNER_EFFECT_H

#include "../Effect.h"

/**
 * @brief Larson Scanner effect - creates a bouncing bar (like Kitt's car in Knight Rider)
 * 
 * This effect creates a smooth bouncing bar that moves back and forth across the LED strip.
 * The bar width is proportional to the strip length and uses a triangular wave function
 * to create the smooth bouncing motion. The effect uses fade_out to create trailing effects.
 * 
 * Key features:
 * - Smooth bidirectional movement using triangular wave
 * - Proportional bar width based on strip length  
 * - Color cycling based on position and base hue
 * - Fade trailing effect for smooth visual appeal
 * - Minimal external dependencies - only requires timebase tracking
 */
class LarsonScannerEffect : public Effect {
private:
    uint32_t timebase;  ///< Time reference for consistent animation timing
    
    /**
     * @brief 16-bit triangle wave function for smooth bouncing motion
     * @param in Input value (0-65535) 
     * @return Triangle wave output creating smooth back-and-forth motion
     */
    static inline uint16_t triwave16(uint16_t in) {
        if (in & 0x8000) {
            in = 65535 - in;
        }
        return in << 1;
    }
    
public:
    LarsonScannerEffect() = default;
    virtual ~LarsonScannerEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
};

#endif // LARSON_SCANNER_EFFECT_H