#ifndef FIRE2012_EFFECT_H
#define FIRE2012_EFFECT_H

#include "../Effect.h"

/**
 * @brief Fire 2012 effect - realistic fire simulation with heat diffusion
 * 
 * This effect creates a highly realistic fire animation by simulating:
 * 
 * - Heat generation at the "base" of the fire (bottom LEDs)
 * - Heat diffusion upward through the LED strip
 * - Cooling effects that make heat dissipate over time
 * - Random sparking to create realistic fire dynamics
 * - Mapping heat values to fire colors using the heat palette
 * 
 * The simulation works by:
 * 1. Cooling each heat cell by a random amount based on cooling parameter
 * 2. Diffusing heat upward (each cell influenced by cells below it)
 * 3. Randomly igniting new sparks at the base with sparking parameter
 * 4. Mapping heat values to colors using a heat-based color palette
 * 
 * This implementation maintains its own heat array rather than using
 * shared segment runtime memory, minimizing external resource dependencies.
 */
class Fire2012Effect : public Effect {
public:
    Fire2012Effect() = default;
    virtual ~Fire2012Effect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Heat array for fire simulation
     * Each element represents the heat level at a specific LED position.
     * Values range from 0 (cool/black) to 255 (hot/bright).
     * Array size matches LED_COUNT for maximum compatibility.
     */
    byte* heatArray = nullptr;
    
    /**
     * @brief Current allocated size of heat array
     * Tracks the actual allocated size to handle dynamic allocation safely
     */
    uint16_t heatArraySize = 0;
    
    
    /**
     * @brief Time reference for consistent animation timing
     */
    uint32_t timebase = 0;
    
    /**
     * @brief Allocate heat array for the current strip segment
     * @param strip Pointer to WS2812FX instance for accessing segment info
     * @return true if allocation successful, false otherwise
     */
    bool allocateHeatArray(WS2812FX* strip);
    
    /**
     * @brief Free the allocated heat array
     * Called during cleanup to prevent memory leaks
     */
    void freeHeatArray();
    
    /**
     * @brief Perform cooling step of fire simulation
     * Reduces heat in each cell by a random amount based on cooling parameter
     * @param strip Pointer to WS2812FX instance for accessing parameters
     */
    void performCooling(WS2812FX* strip);
    
    /**
     * @brief Perform heat diffusion step
     * Each cell's heat is influenced by the cells below it, creating upward heat flow
     * @param strip Pointer to WS2812FX instance for accessing segment info
     */
    void performHeatDiffusion(WS2812FX* strip);
    
    /**
     * @brief Perform sparking step
     * Randomly ignites new heat sources at the base based on sparking parameter
     * @param strip Pointer to WS2812FX instance for accessing parameters
     */
    void performSparking(WS2812FX* strip);
    
    /**
     * @brief Map heat values to LED colors
     * Converts heat array values to visible colors using heat palette
     * @param strip Pointer to WS2812FX instance for accessing LED array and parameters
     */
    void mapHeatToColors(WS2812FX* strip);

public:
    /**
     * @brief Clean up when the effect is being deactivated
     * Frees allocated memory to prevent memory leaks
     */
    void cleanup() override;
};

#endif // FIRE2012_EFFECT_H