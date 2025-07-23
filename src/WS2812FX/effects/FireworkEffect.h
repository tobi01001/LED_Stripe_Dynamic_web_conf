#ifndef FIREWORK_EFFECT_H
#define FIREWORK_EFFECT_H

#include "../Effect.h"

/**
 * @brief Firework effect - simulates random firework explosions with fading trails
 * 
 * This effect creates a realistic firework display by:
 * 
 * - Randomly spawning fireworks at different positions on the strip
 * - Using blur effects to create trailing light effects
 * - Managing multiple simultaneous fireworks with independent lifetimes
 * - Preventing fireworks from spawning too close to existing ones
 * - Using palette colors for varied firework colors
 * 
 * The effect maintains its own state for:
 * - Active firework positions and colors
 * - Burning duration for each firework
 * - Color indices for palette-based coloring
 * 
 * This implementation uses minimal external shared resources by managing
 * all firework state internally rather than using global runtime unions.
 */
class FireworkEffect : public Effect {
public:
    FireworkEffect() = default;
    virtual ~FireworkEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Maximum number of simultaneous fireworks
     * Limits memory usage while allowing for rich visual effects
     */
    static const uint8_t MAX_FIREWORKS = 8; // Using smaller value than MAX_NUM_BARS for memory efficiency
    
    /**
     * @brief Array storing LED positions of active fireworks
     * Each element contains the LED index where a firework is currently burning
     */
    uint16_t fireworkPositions[MAX_FIREWORKS];
    
    /**
     * @brief Array storing color indices for each firework
     * Used to select colors from the current palette for each firework
     */
    uint8_t colorIndices[MAX_FIREWORKS];
    
    /**
     * @brief Array storing remaining burn time for each firework
     * Counts down from initial value to 0, determining how long firework stays lit
     */
    uint8_t burnTimeRemaining[MAX_FIREWORKS];
    

    
    /**
     * @brief Calculate minimum distance between fireworks
     * @param strip Pointer to WS2812FX instance for accessing strip length
     * @return Minimum distance in LEDs between firework spawn points
     */
    uint8_t calculateMinDistance(WS2812FX* strip) const;
    
    /**
     * @brief Check if a position is clear for spawning a new firework
     * @param position LED position to check
     * @param minDistance Minimum required distance from existing fireworks
     * @param strip Pointer to WS2812FX instance for accessing LED array
     * @return true if position is clear, false if too close to existing fireworks
     */
    bool isPositionClear(uint16_t position, uint8_t minDistance, WS2812FX* strip) const;
    
    /**
     * @brief Find an available slot for a new firework
     * @return Index of available slot, or 0xff if all slots are occupied
     */
    uint8_t findAvailableSlot() const;
    
    /**
     * @brief Spawn a new firework at the specified position
     * @param position LED position for the new firework
     * @param slotIndex Array index to store the firework data
     * @param strip Pointer to WS2812FX instance for accessing helper functions
     */
    void spawnFirework(uint16_t position, uint8_t slotIndex, WS2812FX* strip);
};

#endif // FIREWORK_EFFECT_H