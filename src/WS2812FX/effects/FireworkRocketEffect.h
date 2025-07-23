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
        double pos;                ///< Current position in millimeters from strip start
        double v0;                 ///< Initial launch velocity (mm/ms)
        double v;                  ///< Current velocity (mm/ms)
        double v_explode;          ///< Velocity threshold for triggering explosion
        uint16_t prev_pos;         ///< Previous LED position for motion effects
        uint16_t explodeTime;      ///< Remaining explosion duration (decrements each frame)
        uint8_t color_index;       ///< Index into current palette for rocket color
        uint8_t brightness;        ///< Current brightness level (0-255)
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
     * @brief Calculate maximum launch velocity for rockets
     * @param strip Pointer to WS2812FX instance for accessing strip parameters
     * @param gravity Gravitational acceleration in mm/ms²
     * @param segmentLength Physical length of LED segment in millimeters
     * @return Maximum initial velocity in mm/ms
     */
    double calculateMaxVelocity(WS2812FX* strip, double gravity, double segmentLength) const;
    
    /**
     * @brief Get gravitational acceleration based on effect parameters
     * @param strip Pointer to WS2812FX instance for accessing beat88 parameter
     * @return Gravity value in mm/ms² (negative for downward acceleration)
     */
    double getGravity(WS2812FX* strip) const;
    
    /**
     * @brief Calculate physical segment length in millimeters
     * @param strip Pointer to WS2812FX instance for accessing segment data
     * @return Segment length in millimeters
     */
    double calculateSegmentLength(WS2812FX* strip) const;
    
    /**
     * @brief Update rocket physics and handle phase transitions
     * @param rocketIndex Index of rocket to update
     * @param strip Pointer to WS2812FX instance for accessing parameters
     * @param currentTime Current time in milliseconds
     * @param gravity Current gravitational acceleration
     * @param segmentLength Physical segment length
     * @param maxVelocity Maximum velocity for re-launch
     */
    void updateRocketPhysics(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, 
                           double gravity, double segmentLength, double maxVelocity);
    
    /**
     * @brief Render rocket in launch phase with motion trail
     * @param rocket Reference to rocket data
     * @param strip Pointer to WS2812FX instance for rendering functions
     * @param segmentLength Physical segment length for position mapping
     */
    void renderLaunchPhase(const RocketData& rocket, WS2812FX* strip, double segmentLength);
    
    /**
     * @brief Render rocket in explosion phase with blur effects
     * @param rocket Reference to rocket data  
     * @param strip Pointer to WS2812FX instance for rendering functions
     * @param maxBlendWidth Maximum width for blur effects
     */
    void renderExplosionPhase(const RocketData& rocket, WS2812FX* strip, uint16_t maxBlendWidth);
    
    /**
     * @brief Apply global fading to create trail effects
     * @param strip Pointer to WS2812FX instance for accessing fade parameters
     */
    void applyGlobalFade(WS2812FX* strip);
    
    /**
     * @brief Check if rocket should be re-launched
     * @param rocket Reference to rocket data to check
     * @return true if rocket should start new launch cycle
     */
    bool shouldRelaunch(const RocketData& rocket) const;
    
    /**
     * @brief Initialize rocket for new launch cycle
     * @param rocketIndex Index of rocket to initialize
     * @param strip Pointer to WS2812FX instance for accessing parameters
     * @param currentTime Current time in milliseconds
     * @param maxVelocity Maximum velocity for launch
     */
    void initializeRocketLaunch(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, double maxVelocity);
};

#endif // FIREWORK_ROCKET_EFFECT_H