#ifndef HEART_BEAT_EFFECT_H
#define HEART_BEAT_EFFECT_H

#include "../Effect.h"

/**
 * @brief Heart Beat effect - simulates a realistic heartbeat pattern with double pulse
 * 
 * This effect creates a heartbeat pattern by:
 * - Generating a primary pulse followed by a secondary pulse
 * - Using timing based on configurable beats per minute (derived from speed setting)
 * - Creating a spreading pulse effect from the center of the strip
 * - Maintaining continuous fade-out for smooth pulse decay
 * - Adapting pulse size based on strip length
 * 
 * The effect uses a dual-beat pattern typical of a real heartbeat (lub-dub rhythm).
 * The pulse originates from the center and creates a bright spot that fades outward.
 * 
 * Based on: https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Heartbeat.h
 */
class HeartBeatEffect : public Effect {
public:
    HeartBeatEffect() = default;
    virtual ~HeartBeatEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Internal state variables - moved from segment runtime union
    struct {
        uint32_t beatTimer;         ///< Time elapsed since last primary beat
        uint32_t lastBeat;          ///< Timestamp of last primary beat
        uint16_t centerOffset;      ///< Center position of the strip
        uint16_t pCount;            ///< Number of pixels to process for shifting effect
        uint16_t msPerBeat;         ///< Milliseconds per heartbeat (calculated from speed)
        uint16_t secondBeat;        ///< Timing offset for secondary beat
        uint8_t size;               ///< Size of the pulse effect (based on strip length)
        bool secondBeatActive;      ///< Whether secondary beat has been triggered
    } state;

    /**
     * @brief Calculate heartbeat timing based on speed setting
     * @param speed Speed value from segment configuration
     * @return Milliseconds per beat
     */
    uint16_t calculateBeatsPerMinute(uint16_t speed);
    
    /**
     * @brief Calculate appropriate pulse size based on strip length
     * @param stripLength Total length of the LED strip
     * @return Pulse size in pixels
     */
    uint8_t calculatePulseSize(uint16_t stripLength);
};

#endif // HEART_BEAT_EFFECT_H