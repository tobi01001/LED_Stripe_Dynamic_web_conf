#include "PopcornEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

// Define millimeters per LED based on LED density (e.g., 60 LEDs/meter)
constexpr double MM_PER_LED = 1000.0 / 60.0; // 16.666... mm per LED for 60 LEDs/m

bool PopcornEffect::init(WS2812FX* strip) {
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return false;
    }
    
    // Initialize kernel array and set up initial physics state
    initialized = false;
    numKernels = min(strip->getSegment()->numBars, (uint8_t)32);
    
    // Physics constants - convert from LEDS_PER_METER to mm scale
    const uint16_t LEDS_PER_METER = 60;
    const double gravity = getGravity(strip);
    const double maxVelocity = calculateMaxVelocity(strip, gravity);
    
    // Initialize each kernel with staggered velocities for visual variety
    for (uint8_t i = 0; i < numKernels; i++) {
        // Stagger initial velocities so kernels don't all pop at same height
        kernels[i].v0 = maxVelocity / (double)(i + 1.1);
        
        // Assign colors distributed across palette
        kernels[i].color_index = EffectHelper::get_random_wheel_index((uint8_t)((255.0 / numKernels) * i), 32);
        
        // Set initial time base to current time
        kernels[i].timebase = millis();
        
        // Full damping initially (100%)
        kernels[i].damp = 100;
        
        // Start at ground position
        kernels[i].prev_pos = 0;
    }
    
    initialized = true;
    return true;
}

uint16_t PopcornEffect::update(WS2812FX* strip) {
    // Validate strip pointer and initialization
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    if (!initialized) {
        init(strip);
    }
    
    // Clear the LED array using helper
    EffectHelper::clearSegment(strip);
    
    // Get current physics parameters
    const double gravity = getGravity(strip);
    const double maxVelocity = calculateMaxVelocity(strip, gravity);
    const uint32_t currentTime = millis();
    
    // Update and render each kernel
    for (uint8_t i = 0; i < numKernels; i++) {
        // Calculate time elapsed since kernel started its trajectory
        double deltaTime = (double)(currentTime - kernels[i].timebase);
        
        // Calculate current position using physics
        double position = calculatePosition(kernels[i], gravity, deltaTime);
        
        // Handle ground collision and bouncing
        if (position < 0) {
            updateKernelState(i, strip, maxVelocity);
            position = 0; // Clamp to ground level
        }
        
        
        // Render kernel with motion blur effect
        renderKernel(position, kernels[i], strip);
        
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* PopcornEffect::getName() const {
    return F("Popcorn");
}

uint8_t PopcornEffect::getModeId() const {
    return FX_MODE_POPCORN;
}

double PopcornEffect::calculateMaxVelocity(WS2812FX* strip, double gravity) const {
    // Length in MM_PER_LED units (approximate LED spacing at 60 LEDs per meter)
    // Physical strip length in millimeters
    const double segmentLength = (double)strip->getSegmentRuntime()->length * MM_PER_LED;
    
    // Calculate velocity needed to reach end of strip using kinematic equation:
    // v² = v₀² + 2as, where final velocity v = 0, acceleration a = gravity
    // Solving for v₀: v₀ = √(-2 * gravity * distance)
    return sqrt(-2.0 * gravity * segmentLength);
}

double PopcornEffect::getGravity(WS2812FX* strip) const {
    // Map beat88 parameter (0-10000) to gravity range
    // Earth gravity: 9.81 m/s² = 9810 mm/s² = 0.00981 mm/ms²
    // Scale to effect range for visual appeal
    const double minGravity = 9.810 / (1000.0 * 1000.0);     // 0.00000981 mm/ms²
    const double maxGravity = 9810.0 / (1000.0 * 1000.0);    // 0.009810 mm/ms²
    
    double beat88 = (double)strip->getSegment()->beat88;
    double gravity = EffectHelper::safeMapdouble(beat88, 0.0, 10000.0, minGravity, maxGravity);
    
    return -gravity; // Negative for downward acceleration
}

double PopcornEffect::calculatePosition(const KernelData& kernel, double gravity, double deltaTime) const {
    // Kinematic equation for position under constant acceleration:
    // s = v₀t + ½at²
    // Where: s = displacement, v₀ = initial velocity, t = time, a = acceleration (gravity)
    return (gravity / 2.0 * deltaTime + kernel.v0) * deltaTime;
}

void PopcornEffect::updateKernelState(uint8_t kernelIndex, WS2812FX* strip, double maxVelocity) {
    KernelData& kernel = kernels[kernelIndex];
    
    // Apply damping if configured (simulates energy loss on bouncing)
    uint8_t dampingPercent = strip->getSegment()->damping;
    if (dampingPercent < 100) {
        // Reduce velocity based on damping factor and kernel's individual damping
        kernel.v0 = kernel.v0 * ((double)kernel.damp / 100.0) - 0.02;
    }
    
    // Reset time base for new trajectory
    kernel.timebase = millis();
    
    // Check if kernel should re-ignite (very low velocity + random chance)
    if (kernel.v0 < 0.01 && random8() < 1) {
        // Re-ignite with new random velocity (80-100% of maximum)
        kernel.v0 = ((double)random8(80, 100) / 100.0) * maxVelocity;
        
        // Select new color for visual variety
        kernel.color_index = EffectHelper::get_random_wheel_index(kernel.color_index, 32);
        
        // Set new damping factor if damping is enabled
        if (dampingPercent < 100) {
            kernel.damp = (uint8_t)((random8(90, 100) * dampingPercent) / 100);
        } else {
            kernel.damp = 100;
        }
        
        // Reset position tracking
        kernel.prev_pos = 0;
    }
}

void PopcornEffect::renderKernel(double position, KernelData& kernel, WS2812FX* strip) {
    // Convert position to LED coordinates
    uint16_t currentPos = (uint16_t)(position);
    uint16_t prevPosition = kernel.prev_pos;
    // Calculate width for motion blur effect
    uint8_t width = 1;
    int myWidth = (uint8_t)((double)(currentPos - prevPosition) / MM_PER_LED);
    if (currentPos > prevPosition) {
        width = max(myWidth, 1);
    } else if (prevPosition > currentPos) {
        width = max(-myWidth, 1);
    }
    
    // Render kernel using the strip's drawing function with current palette
    strip->drawFractionalBar(
        currentPos,                                              // Position
        width,                                                   // Width for motion blur
        *strip->getCurrentPalette(),                             // Current color palette
        kernel.color_index,                                      // Color index in palette
        255,                                                     // Full brightness
        true,                                                    // Additive blending
        1                                                        // Single pixel precision
    );
    // Update previous position for next frame
    kernel.prev_pos = currentPos;
}

// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_POPCORN, PopcornEffect)