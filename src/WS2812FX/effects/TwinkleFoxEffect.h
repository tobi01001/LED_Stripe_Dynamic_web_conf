#ifndef TWINKLE_FOX_EFFECT_H
#define TWINKLE_FOX_EFFECT_H

#include "../Effect.h"

/**
 * @brief Twinkle Fox Effect - Creates complex organic-looking twinkles with stable PRNG
 * 
 * This effect creates sophisticated twinkling patterns that look more organic and
 * natural than simple random twinkles. It uses a deterministic pseudo-random number
 * generator (PRNG) to ensure that each LED's twinkling pattern is stable and
 * repeatable, creating the illusion of individual "fireflies" or "stars" with
 * their own unique behaviors.
 * 
 * Key features:
 * - Deterministic PRNG for stable per-pixel behavior
 * - Individual timing offsets for each LED
 * - Speed multipliers create varied twinkle rates
 * - Attack/decay wave function for natural brightness curves
 * - Background color blending for smooth transitions
 * - Palette-based color selection
 * 
 * The algorithm is based on the FastLED TwinkleFox example by Mark Kriegsman.
 */
class TwinkleFoxEffect : public Effect {
private:
    /**
     * @brief Compute the color and brightness for one LED's twinkle
     * 
     * This function implements the core TwinkleFox algorithm for a single pixel.
     * It uses the provided time and salt values to determine:
     * - Whether the pixel should be lit during this cycle
     * - The brightness using an attack/decay wave function
     * - The color from the current palette
     * 
     * @param timeMs Pointer to adjusted time in milliseconds for this pixel
     * @param salt Pointer to unique salt value for this pixel
     * @param strip Pointer to WS2812FX instance for palette access
     * @return CRGB color for this pixel at this time
     */
    CRGB computeOneTwinkle(uint32_t* timeMs, uint8_t* salt, WS2812FX* strip);

public:
    TwinkleFoxEffect() = default;
    virtual ~TwinkleFoxEffect() = default;

    /**
     * @brief Update the twinkle fox effect for one frame
     * 
     * This method implements the main TwinkleFox algorithm:
     * 1. Resets the PRNG to a fixed seed for deterministic behavior
     * 2. Gets current time for animation base
     * 3. Sets up background color (currently black)
     * 4. For each LED in the segment:
     *    - Advances PRNG to get unique parameters for this LED
     *    - Calculates time offset and speed multiplier
     *    - Computes adjusted time for this pixel's twinkle cycle
     *    - Calls computeOneTwinkle to get the pixel color
     *    - Blends with background based on brightness difference
     * 
     * @param strip Pointer to the WS2812FX instance providing LED control
     * @return Delay in milliseconds until next frame update
     */
    uint16_t update(WS2812FX* strip) override;

    /**
     * @brief Get the human-readable name of this effect
     * @return Flash string containing "Twinkle Fox"
     */
    const __FlashStringHelper* getName() const override;

    /**
     * @brief Get the mode ID for this effect
     * @return FX_MODE_TWINKLE_FOX constant
     */
    uint8_t getModeId() const override;
};

#endif // TWINKLE_FOX_EFFECT_H