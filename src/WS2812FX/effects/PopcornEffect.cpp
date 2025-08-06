#include "PopcornEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

// Define millimeters per LED based on LED density (e.g., 60 LEDs/meter)
constexpr double MM_PER_LED = 1000.0 / 60.0; // 16.666... mm per LED for 60 LEDs/m

bool PopcornEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize kernel array and set up initial physics state
    numKernels = min(strip->getSegment()->numBars, (uint8_t)MAX_POP_KERNELS);
    
    // Physics constants - convert from LEDS_PER_METER to mm scale
    gravity = EffectHelper::getGravity(strip);
    maxVelocity = EffectHelper::calculateMaxVelocity(strip, gravity);

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
    
    return true;
}

uint16_t PopcornEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    // Validate strip pointer
    if (!EffectHelper::validateStripPointer(strip)) {
        return strip->getStripMinDelay();
    }
    
    // Clear the LED array using helper
    EffectHelper::clearSegment(strip);
    
    // Get current physics parameters
    const uint32_t currentTime = millis();
    
    // Update and render each kernel
    for (uint8_t i = 0; i < numKernels; i++) {
        // Calculate time elapsed since kernel started its trajectory
        double deltaTime = (double)(currentTime - kernels[i].timebase);
        
        // Calculate current position using physics
        double position =  (gravity / 2.0 * deltaTime + kernels[i].v0) * deltaTime;
        
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
    if (currentPos > prevPosition) {
        width = (uint8_t)((double)(currentPos - prevPosition) / MM_PER_LED);
    } else {
        width = (uint8_t)((double)(prevPosition - currentPos) / MM_PER_LED);
    }
    if (!width)
    {
        width = 1;
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