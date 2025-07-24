#include "FireworkRocketEffect.h"
#include "../WS2812FX_FastLed.h"
#include "../EffectHelper.h"

// Define gravity scaling factor for beat88 to gravity conversion
#define GRAVITY_SCALING_FACTOR -1019367.99184506

bool FireworkRocketEffect::init(WS2812FX* strip) {
    // Call base class standard initialization first
    if (!standardInit(strip)) {
        return false;
    }
    
    // Initialize rocket array and set up initial physics state
    numRockets = min(strip->getSegment()->numBars, (uint8_t)32);
    
    uint32_t currentTime = millis();
    
    // Initialize each rocket with default state (grounded, ready to launch)
    for (uint8_t i = 0; i < numRockets; i++) {
        rockets[i].timebase = currentTime;
        
        // Distribute colors evenly across palette using EffectHelper
        rockets[i].color_index = EffectHelper::calculateColorIndex(strip, i, (256 / numRockets) * i);
        
        // Start at ground position
        rockets[i].pos = 0;
        rockets[i].prev_pos = 0;
        
        // No initial velocity (will be set when launching)
        rockets[i].v0 = 0;
        rockets[i].v = 0;
        
        // Set explosion velocity threshold (percentage of max velocity)
        rockets[i].v_explode = 0; // Will be calculated during launch
        
        // Default explosion timing and brightness
        rockets[i].explodeTime = 0;
        rockets[i].brightness = 0;
    }
    
    initialized = true;
    return true;
}

uint16_t FireworkRocketEffect::update(WS2812FX* strip) {
    // Check if effect needs initialization
    if (!isInitialized()) {
        if (!init(strip)) {
            return 1000; // Return reasonable delay if initialization fails
        }
    }
    
    
    // Apply global fading for smooth trail effects
    // This creates the trailing light effect behind moving rockets
    applyGlobalFade(strip);
    
    // Get current physics parameters that affect all rockets
    // These are recalculated each frame to respond to real-time parameter changes
    const uint32_t currentTime = millis();
    const double gravity = getGravity(strip);                    // Based on beat88 for speed control
    const double segmentLength = calculateSegmentLength(strip);  // Physical LED strip length in mm
    const double maxVelocity = calculateMaxVelocity(strip, gravity, segmentLength);  // Max launch speed
    const uint16_t maxBlendWidth = EffectHelper::calculateProportionalWidth(strip, 2, 1);  // Blur radius
    
    // Process each rocket through its lifecycle: launch -> flight -> explosion -> fade -> relaunch
    for (uint8_t i = 0; i < numRockets; i++) {
        // Check if rocket needs to be launched or re-launched
        // Rockets at ground level (pos <= 0) are candidates for new launch cycles
        if (rockets[i].pos <= 0 && shouldRelaunch(i)) {
            initializeRocketLaunch(i, strip, currentTime, maxVelocity);
        }
        
        // Update and render active rockets (those above ground level)
        if (rockets[i].pos > 0) {
            // Update physics: position, velocity, and handle ground collisions
            updateRocketPhysics(i, strip, currentTime, gravity, segmentLength, maxVelocity);
            
            // Render rocket based on current flight phase
            if (rockets[i].v > rockets[i].v_explode) {
                // Launch phase: rocket is accelerating upward with bright trail
                // Velocity is still above explosion threshold
                renderLaunchPhase(i, strip, segmentLength);
            } else {
                // Explosion phase: rocket has slowed enough to trigger explosion
                // Creates bright burst with blur effects and sparks
                renderExplosionPhase(i, strip, maxBlendWidth);
                
                // Countdown explosion timer to control explosion duration
                // When timer reaches 0, explosion fades and rocket becomes ready for relaunch
                if (rockets[i].explodeTime > 0) {
                    rockets[i].explodeTime--;
                }
            }
        }
    }
    
    return strip->getStripMinDelay();
}

const __FlashStringHelper* FireworkRocketEffect::getName() const {
    return F("Firework Rocket");
}

uint8_t FireworkRocketEffect::getModeId() const {
    return FX_MODE_FIREWORKROCKETS;
}

double FireworkRocketEffect::calculateMaxVelocity(WS2812FX* strip, double gravity, double segmentLength) const {
    // Calculate velocity needed to reach reasonable height using kinematic equation:
    // v² = v₀² + 2as, where final velocity v = 0, acceleration a = gravity
    // Solving for v₀: v₀ = √(-2 * gravity * distance)
    return sqrt(-2.0 * gravity * segmentLength);
}

double FireworkRocketEffect::getGravity(WS2812FX* strip) const {
    // Map beat88 parameter to gravity range for visual appeal
    // 
    // The beat88 parameter controls the overall effect speed and timing:
    // - Lower values (1000-3000): Slower, more majestic rockets with longer flight times
    // - Higher values (4000-6000): Faster, more energetic rockets with quicker cycles
    // 
    // Physics note: We use negative gravity constant for downward acceleration
    // The scaling factor converts beat88 range to reasonable mm/ms² values
    // that produce visually appealing ballistic trajectories
    double beat88 = (double)strip->getSegment()->beat88;
    return beat88 / GRAVITY_SCALING_FACTOR; // Use named constant for clarity and maintainability
}

double FireworkRocketEffect::calculateSegmentLength(WS2812FX* strip) const {
    // Convert LED segment length to physical millimeters
    // Assuming 60 LEDs per meter, with blend width consideration
    const uint16_t maxBlendWidth = min((uint16_t)(strip->getSegmentRuntime()->length / 2), (uint16_t)40);
    const double lengthLeds = (double)(strip->getSegmentRuntime()->length - maxBlendWidth / 2);
    return (lengthLeds / 60.0) * 1000.0; // Convert to millimeters
}

void FireworkRocketEffect::updateRocketPhysics(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, 
                                             double gravity, double segmentLength, double maxVelocity) {
    RocketData& rocket = rockets[rocketIndex];
    
    // Calculate time elapsed since rocket started its trajectory
    // This delta time is crucial for accurate physics simulation
    double deltaTime = (double)(currentTime - rocket.timebase);
    
    // Update position using kinematic equation for constant acceleration:
    // s = v₀t + ½at²
    // Where: s = displacement, v₀ = initial velocity, a = gravity, t = time
    rocket.pos = rocket.v0 * deltaTime + (gravity / 2.0) * deltaTime * deltaTime;
    
    // Update velocity using: v = v₀ + at
    // This gives us current velocity for determining explosion trigger
    rocket.v = rocket.v0 + gravity * deltaTime;
    
    // Handle ground collision and physics state transitions
    if (rocket.pos <= 0) {
        // Handle ground collision or reset after explosion
        if (rocket.v0 <= 0.001) {
            // Very low velocity - ready for potential re-launch, keep at minimal velocity
            rocket.v0 = 0.001;
        } else {
            // Apply damping for bouncing effect on rockets with remaining velocity
            uint8_t dampingPercent = strip->getSegment()->damping;
            float damping = 0.1f / 100.0f; // Default minimal damping
            
            if (dampingPercent > 0) {
                if (dampingPercent <= 100) {
                    damping = (float)dampingPercent / 100.0f;
                } else {
                    damping = 1.0f;
                }
            }
            
            // Apply damping to reduce velocity for next bounce
            rocket.v0 = rocket.v0 * (1.0 - damping);
            
            // Ensure minimum velocity threshold to prevent infinite small bounces
            if (rocket.v0 < 0.001) {
                rocket.v0 = 0.001;
            }
        }
        
        // Reset physics state
        rocket.v = rocket.v0;
        rocket.pos = 0.00001; // Slightly above ground to maintain active state
        rocket.prev_pos = 0;
        rocket.timebase = currentTime;
    }
}

void FireworkRocketEffect::renderLaunchPhase(uint8_t rocketIndex, WS2812FX* strip, double segmentLength) {
    const RocketData& rocket = rockets[rocketIndex];
    // Map position from millimeters to LED coordinates
    uint16_t pos = map(rocket.pos, 0.0, segmentLength, 
                      strip->getSegmentRuntime()->start * 16.0, 
                      strip->getSegmentRuntime()->stop * 16.0);
    
    // Calculate motion blur width
    uint16_t width = 2; // Default width
    if (pos != rocket.prev_pos) {
        if (pos > rocket.prev_pos) {
            width = max((pos - rocket.prev_pos) / 16, 2);
        } else {
            width = max((rocket.prev_pos - pos) / 16, 2);
        }
    }
    
    // Render rocket trail using heat colors for launch phase
    strip->drawFractionalBar(
        rocket.prev_pos,     // Start position
        width,               // Width for motion blur
        HeatColors_p,        // Heat palette for rocket trail
        64,                  // Color index in heat palette
        rocket.brightness / 2, // Reduced brightness for trail
        true,                // Additive blending
        1                    // Single pixel precision
    );
}

void FireworkRocketEffect::renderExplosionPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t maxBlendWidth) {
    const RocketData& rocket = rockets[rocketIndex];
    // Map position to LED coordinates
    uint16_t pos = map(rocket.pos, 0.0, calculateSegmentLength(strip),
                      strip->getSegmentRuntime()->start * 16.0,
                      strip->getSegmentRuntime()->stop * 16.0);
    
    // Calculate blur width based on position and strip bounds
    uint16_t blendWidth = maxBlendWidth - 3;
    if ((strip->getSegmentRuntime()->stop - pos / 16) < blendWidth / 2 + 3) {
        blendWidth = (strip->getSegmentRuntime()->stop - pos / 16) * 2;
    }
    if (pos / 16 < blendWidth / 2) {
        blendWidth = (pos / 16) * 2;
    }
    
    // Scale brightness for rendering
    uint8_t renderBrightness = map(rocket.brightness, 0, 48, 0, 255);
    
    if (rocket.explodeTime > 10) {
        // Main explosion phase - bright burst with blur
        
        // Draw main explosion point with palette color
        strip->drawFractionalBar(pos, 3, *strip->getCurrentPalette(), rocket.color_index, 255, true, 0);
        
        // Add white hot center
        CRGB centerColor = strip->leds[pos / 16] + CRGB(0x202020);
        strip->drawFractionalBar(pos, 3, CRGBPalette16(centerColor), 0, 255, true, 0);
        
        // Add sparks around the explosion for visual flair
        addExplosionSparks(rocketIndex, strip, pos, blendWidth);
        
        // Apply blur for explosion spread
        uint16_t ledArraySize = strip->getSegmentRuntime()->length; // Use segment length as the LED array size
        uint16_t startIndex = pos / 16 + 1;
        if (startIndex + blendWidth <= ledArraySize) {
            blur1d(&strip->leds[startIndex], blendWidth, 172);
        } else {
            // Adjust blendWidth to fit within bounds or skip blur
            uint16_t adjustedBlendWidth = ledArraySize - startIndex;
            if (adjustedBlendWidth > 0) {
                blur1d(&strip->leds[startIndex], adjustedBlendWidth, 172);
            }
        }
        
    } else if (rocket.explodeTime > 0) {
        // Fade phase - dimming explosion
        
        // Draw fading explosion with reduced brightness
        strip->drawFractionalBar(pos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(pos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);
        
        // Apply reduced blur
        blur1d(&strip->leds[pos / 16], blendWidth, 128);
        
    } else {
        // Final fade phase - minimal brightness
        
        strip->drawFractionalBar(pos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(pos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);
        
        // Minimal blur
        blur1d(&strip->leds[pos / 16], blendWidth, 64);
    }
}

void FireworkRocketEffect::applyGlobalFade(WS2812FX* strip) {
    // Apply global fading using EffectHelper with beat88-based calculation
    uint8_t fadeAmount = EffectHelper::safeMapuint16_t(strip->getSegment()->beat88, 0, 6000, 24, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
}

bool FireworkRocketEffect::shouldRelaunch(uint8_t rocketIndex) const {
    const RocketData& rocket = rockets[rocketIndex];
    // Relaunch if velocity is very low and random chance triggers
    // Increased probability from 2/256 (~0.78%) to 12/256 (~4.7%) for more frequent launches
    return (rocket.v0 <= 0.01 && random8() < 2);
}

void FireworkRocketEffect::initializeRocketLaunch(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, double maxVelocity) {
    RocketData& rocket = rockets[rocketIndex];
    
    // Set random launch velocity for visual variety
    // Use 85-99% of maximum to ensure rockets reach different heights
    // This creates a more natural, staggered explosion pattern
    rocket.v0 = (double)random((long)(maxVelocity * 850), (long)(maxVelocity * 990)) / 1000.0;
    rocket.v = rocket.v0;
    
    // Start just above ground level to avoid immediate collision detection
    rocket.pos = 0.00001;
    rocket.prev_pos = 0;
    
    // Reset timing base for accurate physics calculations
    rocket.timebase = currentTime;
    
    // Select new color for this launch cycle to add visual variety
    rocket.color_index = EffectHelper::get_random_wheel_index(rocket.color_index, 32);
    
    // Set brightness and explosion timing parameters
    rocket.brightness = random8(12, 48);
    
    // Map beat88 to explosion duration - faster beat88 = shorter explosions
    // This ensures the explosion timing scales with the overall effect speed
    rocket.explodeTime = map(strip->getSegment()->beat88, 1000, 6000, 80, 180);
    
    // Set explosion velocity threshold as percentage of launch velocity
    // When rocket slows to this speed, it transitions to explosion phase
    // Random range (1-50%) creates varied explosion heights
    rocket.v_explode = rocket.v0 * ((double)random8(1, 50) / 100.0);
}

void FireworkRocketEffect::addExplosionSparks(uint8_t rocketIndex, WS2812FX* strip, uint16_t explosionPos, uint16_t sparkRadius) {
    const RocketData& rocket = rockets[rocketIndex];
    
    // Generate 3-6 sparks around the explosion
    uint8_t numSparks = random8(3, 7);
    
    for (uint8_t i = 0; i < numSparks; i++) {
        // Random offset from explosion center (within spark radius)
        int16_t sparkOffset = random16(sparkRadius * 2) - sparkRadius;
        uint16_t sparkPos = explosionPos + (sparkOffset * 16);
        
        // Ensure spark is within strip bounds
        uint16_t stripStart = strip->getSegmentRuntime()->start * 16;
        uint16_t stripEnd = strip->getSegmentRuntime()->stop * 16;
        
        if (sparkPos >= stripStart && sparkPos <= stripEnd) {
            // Random spark brightness (30-80% of main explosion)
            uint8_t sparkBrightness = random8(76, 206); // 30-80% of 255
            
            // Random spark color variation (within 64 steps of main color)
            uint8_t sparkColorIndex = rocket.color_index + random8(128) - 64;
            
            // Get the actual LED index and create spark color
            uint16_t ledIndex = sparkPos / 16;
            CRGB sparkColor = ColorFromPalette(*strip->getCurrentPalette(), sparkColorIndex, sparkBrightness);
            
            // Add spark to existing LED color (single frame spark)
            strip->leds[ledIndex] |= sparkColor;
            
            // Occasionally add a white hot spark for extra sparkle
            if (random8() < 64) { // 25% chance
                CRGB whiteSparkColor = CRGB::White;
                whiteSparkColor.nscale8(sparkBrightness / 2);
                strip->leds[ledIndex] |= whiteSparkColor;
            }
        }
    }
}

// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETS, FireworkRocketEffect)