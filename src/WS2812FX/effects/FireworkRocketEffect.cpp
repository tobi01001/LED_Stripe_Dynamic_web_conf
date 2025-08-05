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
        rockets[i].color_index = map(i,
                            (uint16_t)0, (uint16_t)(numRockets-1),
                            (uint16_t)0, (uint16_t)255);

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
    // Map beat88 to gravity in mm/ms^2
    // beat88=1000 -> 9.81 m/s^2 = 0.00981 mm/ms^2
    // beat88=6000 -> much higher gravity
    double beat88 = (double)strip->getSegment()->beat88;
    double g_earth = 0.00981; // mm/ms^2
    double gravity = g_earth * (beat88 / 1000.0); // scale gravity with beat88
    return -gravity; // negative for downward acceleration
}

double FireworkRocketEffect::calculateSegmentLength(WS2812FX* strip) const {
    // Calculate physical length in mm
    // Assume 60 LEDs per meter
    double leds = (double)strip->getSegmentRuntime()->length;
    return leds * (1000.0 / 60.0); // mm
}

void FireworkRocketEffect::updateRocketPhysics(uint8_t rocketIndex, WS2812FX* strip, uint32_t currentTime, 
                                             double gravity, double segmentLength, double maxVelocity) {
    RocketData& rocket = rockets[rocketIndex];
    
    // Calculate time elapsed since rocket started its trajectory (milliseconds)
    double deltaTime = (double)(currentTime - rocket.timebase); // ms
    // Physics in mm and mm/ms
    // s = v₀t + ½at²
    rocket.pos = rocket.v0 * deltaTime + (gravity / 2.0) * deltaTime * deltaTime;
    rocket.v = rocket.v0 + gravity * deltaTime;
    auto segment_start = strip->getSegmentRuntime()->start;
    auto segment_stop = strip->getSegmentRuntime()->stop;
    // Map mm position to fractional LED position
    double segmentLength_mm = calculateSegmentLength(strip);
    // Map physics position to render position for drawing, but keep rocket.pos for physics
    double renderPos = map(rocket.pos, 0.0, segmentLength_mm, segment_start * 16.0, segment_stop * 16.0);
    rocket.render_pos = renderPos; // Store for rendering functions (add render_pos to RocketData struct)
    
    // Handle ground collision and physics state transitions
    if (rocket.pos <= 0) {
        // Rocket has hit the ground - bound it there and prepare for relaunch
        rocket.pos = 0;           // Bound position to ground level
        rocket.v0 = 0.0;          // Stop all velocity
        rocket.v = 0.0;           // Current velocity also zero
        rocket.prev_pos = 0;      // Reset previous position
        rocket.timebase = currentTime; // Reset time base
        rocket.explodeTime = 0;   // Reset explosion timer
        rocket.render_pos = 0;    // Also reset render position
    }
}

void FireworkRocketEffect::renderLaunchPhase(uint8_t rocketIndex, WS2812FX* strip, double segmentLength) {
    RocketData& rocket = rockets[rocketIndex];
    
    // Use fractional LED position for smooth movement
    uint16_t fracPos = (uint16_t)rocket.pos;
    uint16_t width = 1;
    if (rocket.prev_pos != 0) {
        uint16_t prevFrac = (uint16_t)rocket.prev_pos;
        width = max(abs(fracPos - prevFrac) / 16, 1);
        width = min(width, (uint16_t)5);
    }
    rocket.prev_pos = fracPos;
    strip->drawFractionalBar(
        fracPos,                 // Fractional LED position
        width,                   // Width for motion blur
        HeatColors_p,            // Heat palette for rocket trail
        64,                      // Color index in heat palette
        rocket.brightness,       // Full brightness for rocket
        true,                    // Additive blending
        1                        // Single pixel precision
    );
}

void FireworkRocketEffect::renderExplosionPhase(uint8_t rocketIndex, WS2812FX* strip, uint16_t maxBlendWidth) {
    const RocketData& rocket = rockets[rocketIndex];
    
    // Use fractional LED position for smooth movement
    uint16_t fracPos = (uint16_t)rocket.pos;
  
    // Calculate blur width based on position and strip bounds
    uint16_t blendWidth = maxBlendWidth - 3;
    if ((strip->getSegmentRuntime()->stop - fracPos / 16) < blendWidth / 2 + 3) {
        blendWidth = (strip->getSegmentRuntime()->stop - fracPos / 16) * 2;
    }
    if (fracPos / 16 < blendWidth / 2) {
        blendWidth = (fracPos / 16) * 2;
    }
    
    // Scale brightness for rendering
    uint8_t renderBrightness = map(rocket.brightness, 0, 48, 0, 255);
    
    if (rocket.explodeTime > 10) {
        // Main explosion phase - bright burst with blur
        
        // Draw main explosion point with palette color
        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, 255, true, 0);

        // Add white hot center
        CRGB centerColor = strip->leds[fracPos / 16] + CRGB(0x202020);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(centerColor), 0, 255, true, 0);
        
        // Add sparks around the explosion for visual flair
        addExplosionSparks(rocketIndex, strip, fracPos, blendWidth/4);
        
        // Apply blur for explosion spread
        uint16_t ledArraySize = strip->getSegmentRuntime()->length;
        uint16_t startIndex = fracPos / 16 + 1;
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
        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);

        // Apply reduced blur
        blur1d(&strip->leds[fracPos / 16], blendWidth, 128);

    } else {
        // Final fade phase - minimal brightness

        strip->drawFractionalBar(fracPos, 3, *strip->getCurrentPalette(), rocket.color_index, renderBrightness, true, 0);
        strip->drawFractionalBar(fracPos, 3, CRGBPalette16(0x202020), 0, renderBrightness, true, 0);
        
        // Minimal blur
        blur1d(&strip->leds[fracPos / 16], blendWidth, 64);
    }
}

void FireworkRocketEffect::applyGlobalFade(WS2812FX* strip) {
    // Apply global fading using EffectHelper with beat88-based calculation
    uint8_t fadeAmount = EffectHelper::safeMapuint16_t(strip->getSegment()->beat88, 0, 6000, 24, 255);
    EffectHelper::applyFadeEffect(strip, fadeAmount);
}

bool FireworkRocketEffect::shouldRelaunch(uint8_t rocketIndex) const {
    const RocketData& rocket = rockets[rocketIndex];
    // Relaunch if velocity is zero and random chance triggers (higher probability)
    return (rocket.v0 == 0.0 && random8() < 2);
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

    // Generate 1-4 sparks around the explosion
    uint8_t numSparks = random8(1, 3);
    for (uint8_t i = 0; i < numSparks; i++) {
        // Random offset from explosion center (within spark radius)
        int16_t sparkOffset = random16(sparkRadius * 2) - sparkRadius;
        int16_t ledIndexRaw = (int16_t)(explosionPos / 16) + sparkOffset;

        // Ensure spark is within strip bounds and ledIndex is non-negative
        uint16_t stripStart = strip->getSegmentRuntime()->start;
        uint16_t stripEnd = strip->getSegmentRuntime()->stop;
        uint16_t ledIndex = constrain(ledIndexRaw, stripStart, stripEnd);

        // Random spark brightness (30-80% of main explosion)
        uint8_t sparkBrightness = random8(76, 206);

        // Random spark color variation (within 64 steps of main color)
        uint8_t sparkColorIndex = rocket.color_index + random8(128) - 64;

        CRGB sparkColor = ColorFromPalette(*strip->getCurrentPalette(), sparkColorIndex, sparkBrightness);

        // Add spark to existing LED color (single frame spark)
        strip->leds[ledIndex] |= sparkColor;

        // Occasionally add a white hot spark for extra sparkle
        if (random8() < 64) {
            CRGB whiteSparkColor = CRGB::White;
            whiteSparkColor.nscale8(sparkBrightness / 2);
            strip->leds[ledIndex] |= whiteSparkColor;
        }
    }
}

#ifdef DEF_FX_MODE_FIREWORKROCKETS
// Register the effect with the factory system
REGISTER_EFFECT(FX_MODE_FIREWORKROCKETS, FireworkRocketEffect)
#endif