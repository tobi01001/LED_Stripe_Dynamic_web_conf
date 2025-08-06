#ifndef FIREWORK_ROCKET_EFFECT_H
#define FIREWORK_ROCKET_EFFECT_H

#include "../Effect.h"

/**
 * @brief Firework Rocket effect - simulates rockets launching and exploding with realistic physics
 * 
 * This effect creates a realistic firework rocket display by:
 * 
 * - Simulating rocket launch phase with upward trajectory under gravity
 * - Triggering explosion phase when rockets reach peak or slow down sufficiently
 * - Creating bright explosion effects with blur and fading
 * - Managing multiple simultaneous rockets with independent lifecycles
 * - Using realistic physics for ballistic motion and explosion dynamics
 * - Supporting configurable explosion timing and brightness
 * 
 * The effect has three distinct phases for each rocket:
 * 1. Launch phase: Rocket accelerates upward with bright trail
 * 2. Explosion phase: Bright burst with expanding blur effect
 * 3. Fade phase: Gradual dimming and disappearance
 * 
 * The effect maintains its own state for:
 * - Individual rocket physics (position, velocity, explosion timing)
 * - Color indices for palette-based coloring
 * - Brightness and explosion state tracking
 * - Previous positions for motion trail effects
 * 
 * Physics simulation includes:
 * - Gravitational deceleration during ascent
 * - Variable launch velocities for visual variety
 * - Explosion trigger based on velocity thresholds
 * - Realistic ballistic trajectories
 * 
 * This implementation uses minimal external shared resources by managing
 * all rocket state internally rather than using global runtime unions.
 */
class FireworkRocketEffect : public Effect {
public:
    FireworkRocketEffect() = default;
    virtual ~FireworkRocketEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Structure to hold complete state for each firework rocket
     * Contains all physics, timing, and visual data needed to simulate one rocket
     */
    struct RocketData {
    uint32_t timebase;         ///< Time reference for physics calculations (milliseconds)
    int32_t pos;              ///< Current position in 16xLED units from strip start
    int16_t v0;               ///< Initial launch velocity (16xLED/ms)
    int16_t v;                ///< Current velocity (16xLED/ms)
    int16_t v_explode;        ///< Velocity threshold for triggering explosion (16xLED/ms)
    uint16_t prev_pos;        ///< Previous LED position for motion effects
    uint16_t explodeTime;     ///< Remaining explosion duration (decrements each frame)
    uint8_t color_index;      ///< Index into current palette for rocket color
    uint8_t brightness;       ///< Current brightness level (0-255)
    };
    
    /**
     * @brief Array storing physics and visual data for each active rocket
     * Each element manages the complete lifecycle of one rocket
     */
    RocketData rockets[32];  // Using reasonable limit based on MAX_NUM_BARS
    
    /**
     * @brief Number of active rockets currently being simulated
     * Determined by strip configuration (SEG.numBars)
     */
    uint8_t numRockets;
    
    /**
     * @brief Flag to track initialization state
     * Ensures rocket arrays are properly initialized on first activation
     */
    bool initialized;
    
    /**
     * @brief Calculate maximum launch velocity for rockets
     * @param strip Pointer to WS2812FX instance for accessing strip parameters
     * @param gravity Gravitational acceleration in 16xLED/ms²
     * @param segmentLength Physical length of LED segment in LED units
     * @return Maximum initial velocity in 16xLED/ms
     */
    int16_t calculateMaxVelocity(WS2812FX* strip, int16_t gravity, uint16_t segmentLength) const;
    
    /**
     * @brief Get gravitational acceleration based on effect parameters
     * @param strip Pointer to WS2812FX instance for accessing beat88 parameter
     * @return Gravity value in 16xLED units/ms² (negative for downward acceleration)
     */
    int16_t getGravity(WS2812FX* strip) const;
    
    /**
     * @brief Calculate physical segment length in 16xLED units
     * @param strip Pointer to WS2812FX instance for accessing segment data
     * @return Segment length in 16xLED units
     */
    uint16_t calculateSegmentLength(WS2812FX* strip) const;
    
    /**
     * @brief Update rocket physics and handle phase transitions
     * @param rocketIndex Index of rocket to update
     * @param strip Pointer to WS2812FX instance for accessing parameters
     * @param currentTime Current time in milliseconds
     * @param gravity Current gravitational acceleration (16xLED units/ms²)
     * @param segmentLength Physical segment length (16xLED units)
     * @param maxVelocity Maximum velocity for re-launch (16xLED units/ms)
     */
    void updateRocketPhysics(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, 
                           int16_t gravity, uint16_t segmentLength, int16_t maxVelocity);
    
    /**
     * @brief Render rocket in launch phase with motion trail
     * @param rocketIndex Index of rocket to render
     * @param strip Pointer to WS2812FX instance for rendering functions
     * @param segmentLength Physical segment length for position mapping (16xLED units)
     */
    void renderLaunchPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t segmentLength);
    
    /**
     * @brief Render rocket in explosion phase with blur effects
     * @param rocketIndex Index of rocket to render
     * @param strip Pointer to WS2812FX instance for rendering functions
     * @param maxBlendWidth Maximum width for blur effects
     */
    void renderExplosionPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t maxBlendWidth);
    
    /**
     * @brief Apply global fading to create trail effects
     * @param strip Pointer to WS2812FX instance for accessing fade parameters
     */
    void applyGlobalFade(WS2812FX* strip);
    
    /**
     * @brief Check if rocket should be re-launched
     * @param rocketIndex Index of rocket to check
     * @return true if rocket should start new launch cycle
     */
    bool shouldRelaunch(uint8_t rocketIndex) const;
    
    /**
     * @brief Initialize rocket for new launch cycle
     * @param rocketIndex Index of rocket to initialize
     * @param strip Pointer to WS2812FX instance for accessing parameters
     * @param currentTime Current time in milliseconds
     * @param maxVelocity Maximum velocity for launch
     */
    void initializeRocketLaunch(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, int16_t maxVelocity);
    
    /**
     * @brief Add sparkling effects around explosion for visual enhancement
     * @param rocketIndex Index of the exploding rocket
     * @param strip Pointer to WS2812FX instance for rendering functions
     * @param explosionPos Position of the explosion in 16ths of pixels
     * @param sparkRadius Maximum radius for spark placement
     */
    void addExplosionSparks(uint8_t rocketIndex, WS2812FX* strip, uint16_t explosionPos, uint16_t sparkRadius);
};

#endif // FIREWORK_ROCKET_EFFECT_H