#ifndef POPCORN_EFFECT_H
#define POPCORN_EFFECT_H

#include "../Effect.h"

/**
 * @brief Popcorn effect - simulates popcorn kernels popping with realistic physics
 * 
 * This effect creates a realistic popcorn popping animation by:
 * 
 * - Simulating each kernel as a physical object with gravity and velocity
 * - Using real physics calculations for parabolic motion under gravity
 * - Supporting multiple simultaneous kernels popping at different heights
 * - Implementing damping/bouncing when kernels hit the ground
 * - Using palette colors for varied kernel colors
 * - Automatically re-igniting kernels after they lose momentum
 * 
 * The effect maintains its own state for:
 * - Individual kernel physics (position, velocity, time base)
 * - Color indices for palette-based coloring
 * - Damping factors for realistic bouncing behavior
 * - Previous positions for motion blur effects
 * 
 * Physics simulation includes:
 * - Gravitational acceleration (configurable via beat88 parameter)
 * - Initial velocity calculations to reach strip ends
 * - Realistic ballistic motion with parabolic trajectories
 * - Energy loss on bouncing for natural behavior
 * 
 * This implementation uses minimal external shared resources by managing
 * all kernel state internally rather than using global runtime unions.
 */
class PopcornEffect : public Effect {
public:
    PopcornEffect() = default;
    virtual ~PopcornEffect() = default;

    bool init(WS2812FX* strip) override;
    uint16_t update(WS2812FX* strip) override;
    const __FlashStringHelper* getName() const override;
    uint8_t getModeId() const override;

private:
    /**
     * @brief Structure to hold physics state for each popcorn kernel
     * Contains all the physics and visual data needed to simulate one kernel
     */
    struct KernelData {
        uint32_t timebase;      ///< Time reference for physics calculations (milliseconds)
        double v0;              ///< Initial velocity (mm/ms)
        uint8_t color_index;    ///< Index into current palette for kernel color
        uint8_t damp;           ///< Damping factor for bouncing (percentage, 0-100)
        uint16_t prev_pos;      ///< Previous LED position for motion effects
    };
    
    /**
     * @brief Array storing physics data for each active kernel
     * Each element manages the complete state of one popping kernel
     */
    KernelData kernels[32];  // Using reasonable limit based on MAX_NUM_BARS
    
    /**
     * @brief Number of active kernels currently being simulated
     * Determined by strip configuration (SEG.numBars)
     */
    uint8_t numKernels;
    
    /**
     * @brief Flag to track initialization state
     * Ensures kernel arrays are properly initialized on first activation
     */
    bool initialized;
    
    /**
     * @brief Calculate maximum velocity needed to reach strip end
     * @param strip Pointer to WS2812FX instance for accessing strip parameters
     * @param gravity Gravitational acceleration in mm/ms²
     * @return Maximum initial velocity in mm/ms
     */
    double calculateMaxVelocity(WS2812FX* strip, double gravity) const;
    
    /**
     * @brief Get gravitational acceleration based on effect parameters
     * @param strip Pointer to WS2812FX instance for accessing beat88 parameter
     * @return Gravity value in mm/ms² (negative for downward acceleration)
     */
    double getGravity(WS2812FX* strip) const;
    
    /**
     * @brief Calculate current position using physics simulation
     * @param kernel Reference to kernel data containing velocity and time
     * @param gravity Current gravitational acceleration
     * @param deltaTime Time elapsed since kernel time base
     * @return Current position in millimeters from strip start
     */
    double calculatePosition(const KernelData& kernel, double gravity, double deltaTime) const;
    
    /**
     * @brief Handle kernel bouncing and re-ignition logic
     * @param kernelIndex Index of kernel to update
     * @param strip Pointer to WS2812FX instance for accessing parameters
     * @param maxVelocity Maximum velocity for re-ignition
     */
    void updateKernelState(uint8_t kernelIndex, WS2812FX* strip, double maxVelocity);
    
    /**
     * @brief Render a kernel at its current position with motion blur
     * @param position Current position in millimeters
     * @param prevPosition Previous position for calculating motion width
     * @param kernel Kernel data containing color information
     * @param strip Pointer to WS2812FX instance for rendering functions
     */
    void renderKernel(double position, KernelData& kernel, WS2812FX* strip);
};

#endif // POPCORN_EFFECT_H