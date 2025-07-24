#ifndef BUBBLE_SORT_EFFECT_H
#define BUBBLE_SORT_EFFECT_H

#include "../Effect.h"

/**
 * @brief Bubble Sort Effect - Visualizes the bubble sort algorithm with colored elements
 * 
 * This effect demonstrates the bubble sort algorithm by visualizing the sorting process
 * on the LED strip. Each LED position represents an element with a specific hue value,
 * and the effect shows the step-by-step comparison and swapping process of bubble sort.
 * 
 * The algorithm works by:
 * 1. Initializing each LED position with a random hue value
 * 2. Comparing adjacent elements and swapping them if they're out of order
 * 3. Highlighting the elements being compared and moved
 * 4. Continuing until the entire array is sorted
 * 
 * Visual feedback:
 * - All LEDs show their current hue values from the palette
 * - Elements being compared are highlighted
 * - Movement animations show the swapping process
 * 
 * The effect is self-contained and manages its own state without relying on
 * external shared runtime variables.
 */
class BubbleSortEffect : public Effect {
public:
    BubbleSortEffect() = default;
    virtual ~BubbleSortEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;
    void cleanup() override;

private:
    // Effect state variables - encapsulated within the class
    uint8_t* hues;           ///< Array of hue values for each LED position (dynamically allocated)
    bool movedown;           ///< Direction flag for movement animation
    uint16_t ci;             ///< Inner loop counter (comparison index)
    uint16_t co;             ///< Outer loop counter (current position)
    uint16_t cd;             ///< Movement animation counter (countdown)
    uint16_t strip_length;   ///< Current strip length for memory management

    /**
     * @brief Initialize the hue array with random values
     * @param strip Pointer to WS2812FX instance for accessing strip length
     */
    void initializeHues(WS2812FX* strip);

    /**
     * @brief Clean up dynamically allocated memory
     */
    void cleanupMemory();

    /**
     * @brief Update LED display with current hue values
     * @param strip Pointer to WS2812FX instance
     */
    void updateLEDDisplay(WS2812FX* strip);

    /**
     * @brief Calculate frame delay based on current settings
     * @param strip Pointer to WS2812FX instance for accessing speed and length
     * @return Delay in milliseconds for next frame
     */
    uint16_t calculateFrameDelay(WS2812FX* strip) const;
};

#endif // BUBBLE_SORT_EFFECT_H