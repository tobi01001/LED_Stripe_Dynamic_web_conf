#ifndef DOT_COL_WIPE_EFFECT_H
#define DOT_COL_WIPE_EFFECT_H

#include "../Effect.h"

/**
 * @brief Dot Color Wipe effect - moving dots without background fading
 * 
 * This effect creates multiple moving bars (dots) that move using different 
 * mathematical wave functions, similar to DotBeat but without background
 * fading, creating a color wipe pattern where the dots leave trails.
 */
class DotColWipeEffect : public Effect {
public:
    DotColWipeEffect() = default;
    virtual ~DotColWipeEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Per-bar state tracking (same structure as DotBeat)
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
    
    // Constants - no fading for color wipe effect
    static const uint8_t FADE_AMOUNT = 0;     ///< No background fade 
    static const uint8_t BAR_WIDTH = 3;       ///< Width of each bar
    
    // Helper methods
    void initializeBars(WS2812FX* strip);
    void updateBarSpeeds(WS2812FX* strip);
    uint16_t calculateBarPosition(uint8_t barIndex, uint8_t waveType, WS2812FX* strip);
    void updateBarColor(uint8_t barIndex, uint16_t position, WS2812FX* strip);
    void cleanup() override;
};

#endif // DOT_COL_WIPE_EFFECT_H