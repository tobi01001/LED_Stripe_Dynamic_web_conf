#ifndef EFFECT_HELPER_H
#define EFFECT_HELPER_H

#include "FastLED.h"

// Forward declaration
class WS2812FX;

/**
 * @brief Helper class that consolidates common functionality used across multiple effects
 * 
 * This class provides reusable utility methods for:
 * - Common initialization patterns
 * - Timing and animation calculations
 * - Color and palette operations  
 * - Memory management utilities
 * - Mathematical functions
 * 
 * By centralizing these functions, we reduce code duplication and optimize RAM/ROM usage.
 */
class EffectHelper {
public:
    // ===== INITIALIZATION HELPERS =====
    
    /**
     * @brief Standard effect initialization pattern
     * @param strip Pointer to WS2812FX instance
     * @param timebase Reference to effect's timebase variable (will be set to millis())
     * @param initialized Reference to effect's initialized flag (will be set to true)
     * @return true if initialization succeeded
     */
    static bool standardInit(WS2812FX* strip, uint32_t& timebase, bool& initialized);
    
    /**
     * @brief Safe pointer validation for WS2812FX instance
     * @param strip Pointer to validate
     * @return true if pointer is valid and safe to use
     */
    static bool validateStripPointer(WS2812FX* strip);
    
    // ===== TIMING AND ANIMATION HELPERS =====
    
    /**
     * @brief Calculate beat-based position with standard parameters
     * @param strip WS2812FX instance for accessing segment data
     * @param timebase Time reference for consistent calculations
     * @param speedMultiplier Multiplier for beat88 speed (default 1)
     * @return Position value in range 0-65535
     */
    static uint16_t calculateBeatPosition(WS2812FX* strip, uint32_t timebase, uint8_t speedMultiplier = 1);
    
    /**
     * @brief Generate triangular wave for fade effects
     * @param beatPosition Input position (0-65535)
     * @param minBrightness Minimum brightness value (default 32)
     * @param maxBrightness Maximum brightness value (default 255)
     * @return Brightness value with triangular wave pattern
     */
    static uint8_t generateTriangleWave(uint16_t beatPosition, uint8_t minBrightness = 32, uint8_t maxBrightness = 255);
    
    /**
     * @brief Map position across strip length with fractional precision
     * @param strip WS2812FX instance for accessing runtime data
     * @param beatPosition Beat-based position (0-65535)
     * @param speedMultiplier Speed multiplier for position calculation
     * @return Position in 16-bit fixed point format (for sub-pixel accuracy)
     */
    static uint16_t mapPositionToStrip(WS2812FX* strip, uint16_t beatPosition, uint8_t speedMultiplier = 1);
    
    // ===== COLOR AND PALETTE HELPERS =====
    
    /**
     * @brief Calculate color index with hue distribution
     * @param strip WS2812FX instance for accessing segment data
     * @param position Current position on strip
     * @param hueOffset Additional hue offset to apply
     * @return Color index for palette lookup
     */
    static uint8_t calculateColorIndex(WS2812FX* strip, uint16_t position, uint8_t hueOffset = 0);
    
    /**
     * @brief Calculate triangular wave position for smooth back-and-forth motion
     * @param strip WS2812FX instance for accessing segment data
     * @param timebase Time reference for consistent calculations
     * @param speedMultiplier Multiplier for beat88 speed (default 1)
     * @return Position value using triangular wave pattern (0-65535)
     */
    static uint16_t calculateTrianglePosition(WS2812FX* strip, uint32_t timebase, uint8_t speedMultiplier = 1);
    
    /**
     * @brief Clear segment to black color
     * @param strip WS2812FX instance
     */
    static void clearSegment(WS2812FX* strip);
    
    /**
     * @brief Draw a fractional bar with automatic positioning
     * @param strip WS2812FX instance
     * @param relativePosition Position relative to segment start (16-bit fractional)
     * @param width Width of the bar in pixels
     * @param colorIndex Color index for palette lookup
     * @param brightness Brightness value (0-255)
     */
    static void drawBar(WS2812FX* strip, uint16_t relativePosition, uint16_t width, uint8_t colorIndex, uint8_t brightness = 255);
    
    // ===== SPECIAL EFFECT UTILITIES =====
    
    /**
     * @brief Generate attack-decay wave for natural looking twinkle effects
     * Creates a brightness curve with sharp attack and slow decay, mimicking fireflies/stars
     * @param phase Input phase (0-255)
     * @return Brightness value (0-255) with attack-decay characteristic
     */
    static uint8_t attackDecayWave8(uint8_t phase);
    
    /**
     * @brief Apply fade effect to LED strip
     * @param strip WS2812FX instance
     * @param fadeAmount Amount to fade (0-255, higher = more fade)
     */
    static void applyFadeEffect(WS2812FX* strip, uint8_t fadeAmount);
    
    /**
     * @brief Fill strip segment with palette colors and brightness
     * @param strip WS2812FX instance
     * @param brightness Overall brightness to apply
     * @param hueDelta Hue increment between LEDs (for color distribution)
     */
    static void fillPaletteWithBrightness(WS2812FX* strip, uint8_t brightness, uint8_t hueDelta = 4);
    
    // ===== MEMORY MANAGEMENT HELPERS =====
    
    /**
     * @brief Safely allocate memory array for effect data
     * @param currentArray Current array pointer (may be null)
     * @param currentSize Current array size
     * @param requiredSize Required size for the array
     * @param elementSize Size of each element in bytes
     * @return Pointer to allocated array, or nullptr if allocation failed
     */
    static void* safeAllocateArray(void* currentArray, size_t& currentSize, size_t requiredSize, size_t elementSize);
    
    /**
     * @brief Safely free allocated memory
     * @param array Pointer to array to free
     * @param size Reference to size variable (will be set to 0)
     */
    static void safeFreeArray(void*& array, size_t& size);
    
    // ===== MATHEMATICAL UTILITIES =====
    
    /**
     * @brief Map uint16_t value from one range to another with bounds checking
     * @param value Input value
     * @param fromMin Input range minimum
     * @param fromMax Input range maximum
     * @param toMin Output range minimum
     * @param toMax Output range maximum
     * @return Mapped value clamped to output range
     */
    static uint16_t safeMapuint16_t(uint16_t value, uint16_t fromMin, uint16_t fromMax, uint16_t toMin, uint16_t toMax);
    
    /**
     * @brief Map double value from one range to another with bounds checking
     * @param value Input value
     * @param fromMin Input range minimum
     * @param fromMax Input range maximum
     * @param toMin Output range minimum
     * @param toMax Output range maximum
     * @return Mapped value clamped to output range
     */
    static double safeMapdouble(double value, double fromMin, double fromMax, double toMin, double toMax);

    



    /**
     * @brief Linear interpolation between two values
     * @param a First value
     * @param b Second value
     * @param fraction Interpolation fraction (0.0 = a, 1.0 = b)
     * @return Interpolated value
     */
    static uint8_t linearInterpolate(uint8_t a, uint8_t b, float fraction);
    
    /**
     * @brief Calculate proportional width based on strip length
     * @param strip WS2812FX instance
     * @param divisor Divisor for calculating width (e.g., 15 for 1/15th width)
     * @param minimum Minimum width to return
     * @return Calculated width, at least minimum value
     */
    static uint16_t calculateProportionalWidth(WS2812FX* strip, uint16_t divisor, uint16_t minimum = 1);
    
    // ===== CONSTANTS =====
    
    // Common fade amounts
    static const uint8_t LIGHT_FADE = 64;
    static const uint8_t MEDIUM_FADE = 96;
    static const uint8_t HEAVY_FADE = 128;
    
    // Common brightness values
    static const uint8_t MIN_BRIGHTNESS = 32;
    static const uint8_t MAX_BRIGHTNESS = 255;
    
    // Common hue delta values
    static const uint8_t DEFAULT_HUE_DELTA = 4;
    static const uint8_t WIDE_HUE_DELTA = 16;
    
    // Common speed multipliers
    static const uint8_t SLOW_SPEED = 1;
    static const uint8_t NORMAL_SPEED = 2;
    static const uint8_t FAST_SPEED = 4;
};

#endif // EFFECT_HELPER_H