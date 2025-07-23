#ifndef DOT_BEAT_EFFECT_H
#define DOT_BEAT_EFFECT_H

#include "../Effect.h"

/**
 * @brief Dot Beat effect - creates multiple moving dots with different wave patterns
 * 
 * This effect creates several moving bars (dots) that move using different 
 * mathematical wave functions (triangular, quadratic, cubic). Each bar has
 * its own speed, timing, and color. The bars fade background and create
 * dynamic movement patterns across the strip.
 */
class DotBeatEffect : public Effect {
public:
    DotBeatEffect() = default;
    virtual ~DotBeatEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Per-bar state tracking
    struct BarState {
        uint16_t beat;        ///< Beat speed for this bar
        uint32_t timebase;    ///< Time reference for this bar  
        uint8_t colorOffset;  ///< Color offset for this bar
        bool newBase;         ///< Flag for timing reset
    };
    
    uint32_t timebase = 0;       ///< Main time reference
    uint16_t lastBeat88 = 0;     ///< Last beat88 value for change detection
    BarState* barStates = nullptr; ///< Dynamic array of bar states
    uint8_t numBars = 0;         ///< Number of active bars
    bool initialized = false;    ///< Initialization flag
    
    // Constants
    static const uint8_t FADE_AMOUNT = 64;    ///< Background fade amount
    static const uint8_t BAR_WIDTH = 3;       ///< Width of each bar
    static const uint8_t MIN_BEAT = 1;        ///< Minimum beat divisor
    static const uint8_t MAX_BEAT_MULTIPLIER = 3; ///< Maximum beat multiplier
    static const uint8_t MIN_SPEED_MULTIPLIER = 3; ///< Minimum speed multiplier  
    static const uint8_t MAX_SPEED_MULTIPLIER = 6; ///< Maximum speed multiplier
    
    // Helper methods
    void initializeBars(WS2812FX* strip);
    void updateBarSpeeds(WS2812FX* strip);
    uint16_t calculateBarPosition(uint8_t barIndex, uint8_t waveType, WS2812FX* strip);
    void updateBarColor(uint8_t barIndex, uint16_t position, WS2812FX* strip);
    void cleanup() override;
};

#endif // DOT_BEAT_EFFECT_H