#ifndef METEOR_SHOWER_EFFECT_H
#define METEOR_SHOWER_EFFECT_H

#include "../Effect.h"

/**
 * @brief Meteor Shower effect - creates falling meteor-like streaks across the LED strip
 * 
 * This effect simulates a meteor shower by:
 * - Creating multiple meteor streaks that fall from top to bottom of the strip
 * - Each meteor has its own timing, color, and lifecycle
 * - Meteors start at random intervals to create natural variation
 * - Background continuously fades to create trailing effects
 * - Speed setting controls both fade rate and meteor spawn frequency
 * - Number of simultaneous meteors is configurable via numBars setting
 * 
 * The meteors are drawn as short bars that move down the strip using fractional positioning
 * for smooth movement. Each meteor has a random color offset from the base hue to create
 * visual variety in the shower.
 */
class MeteorShowerEffect : public Effect {
public:
    MeteorShowerEffect() = default;
    virtual ~MeteorShowerEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    // Internal state variables - moved from segment runtime union
    // Arrays sized for maximum possible meteors (MAX_NUM_BARS)
    struct {
        uint32_t timebase[10];    ///< Individual start times for each meteor (MAX_NUM_BARS limit)
        uint8_t actives[10];      ///< Active status for each meteor slot (boolean array)
        uint8_t cind[10];         ///< Color index offset for each meteor (for color variation)
    } state;
    
    uint32_t lastFadeTime;        ///< Timestamp for fade timing control
    uint32_t lastSpawnTime;       ///< Timestamp for meteor spawn timing control
    
    
    /**
     * @brief Check if there's sufficient distance from the end for a new meteor
     * @param strip Pointer to the WS2812FX instance
     * @param minDistance Minimum distance required in pixels
     * @return true if area is clear for new meteor
     */
    bool isSpawnAreaClear(WS2812FX* strip, uint16_t minDistance);
    
    /**
     * @brief Find an inactive meteor slot for spawning a new meteor
     * @param maxMeteors Maximum number of meteors to check
     * @return Index of available slot, or 255 if none available
     */
    uint8_t findAvailableSlot(uint8_t maxMeteors);

    // Timing constants
    static const uint32_t FADE_INTERVAL = 20;    ///< Milliseconds between fade operations
    static const uint32_t SPAWN_INTERVAL = 100;  ///< Milliseconds between spawn checks
    static const uint8_t METEOR_WIDTH = 4;       ///< Width of meteor trail in pixels
};

#endif // METEOR_SHOWER_EFFECT_H