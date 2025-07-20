#ifndef PHONE_RING_EFFECT_H
#define PHONE_RING_EFFECT_H

#include "../Effect.h"

/**
 * @brief Phone Ring effect - simulates a ringing phone with blinking pattern and pauses
 * 
 * This effect creates a phone ring pattern by:
 * - Alternating between on/off states with specific timing
 * - Using a run/pause cycle to simulate realistic phone ringing
 * - Filling the entire strip with palette colors during "on" state
 * - Fading to black during "off" and pause states
 * 
 * Timing pattern:
 * - On time: 50ms (brief flash)
 * - Off time: 100ms (short pause between flashes)
 * - Run time: 1500ms (total duration of ring sequence)
 * - Pause time: 2000ms (pause between ring sequences)
 */
class PhoneRingEffect : public Effect {
public:
    PhoneRingEffect() = default;
    virtual ~PhoneRingEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Internal state variables - moved from segment runtime union
    struct {
        uint32_t nextmillis;    ///< Timestamp for next state change
        uint32_t pausemillis;   ///< Timestamp when pause period started
        uint32_t now;           ///< Current timestamp (updated each frame)
        bool isOn;              ///< Current on/off state during ring sequence
        bool isPause;           ///< Whether currently in pause period between rings
    } state;

    // Timing constants for phone ring pattern
    static const uint16_t ON_TIME = 50;     ///< Duration of "on" state in milliseconds
    static const uint16_t OFF_TIME = 100;   ///< Duration of "off" state in milliseconds
    static const uint16_t RUN_TIME = 1500;  ///< Duration of ring sequence in milliseconds
    static const uint16_t PAUSE_TIME = 2000; ///< Duration of pause between rings in milliseconds
};

#endif // PHONE_RING_EFFECT_H