#ifndef VOID_EFFECT_H
#define VOID_EFFECT_H

#include "../Effect.h"

/**
 * @brief Void effect - does nothing, turns off autoplay
 * 
 * This effect is designed to be a null/empty effect that essentially
 * does nothing except disable autoplay mode. It serves as a "pause" 
 * or "idle" state for the LED strip without any visual effects.
 * 
 * The void effect:
 * - Disables autoplay mode during initialization
 * - Returns minimum delay to avoid unnecessary processing
 * - Does not modify LED colors or states
 * - Uses minimal external shared resources
 */
class VoidEffect : public Effect {
public:
    VoidEffect() = default;
    virtual ~VoidEffect() = default;

    /**
     * @brief Initialize the void effect
     * @param strip Pointer to the WS2812FX instance
     * @return Always returns true (initialization cannot fail)
     */
    bool init(WS2812FX* strip) override;

    /**
     * @brief Update the void effect (does nothing)
     * @param strip Pointer to the WS2812FX instance
     * @return Minimum delay since no processing is needed
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the name of this effect
     * @return Flash string containing "Void DOES NOTHING"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_VOID
     */
    uint8_t getModeId() const override;
};

#endif // VOID_EFFECT_H