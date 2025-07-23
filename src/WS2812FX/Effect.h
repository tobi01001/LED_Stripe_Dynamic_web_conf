#ifndef WS2812FX_EFFECT_H
#define WS2812FX_EFFECT_H

#include "FastLED.h"
#include "../include/defaults.h" // Ensure LED_COUNT is defined before use

// Forward declarations - full definitions will be available in implementation files
class WS2812FX;

/**
 * @brief Base class for all LED effects
 * 
 * This abstract base class defines the interface that all effects must implement.
 * Effects are responsible for:
 * - Initializing their state when first activated
 * - Rendering one frame of the effect
 * - Managing their own internal state variables
 * - Returning appropriate timing delays for smooth animation
 */
class Effect {
public:
    Effect() : initialized(false) {}
    virtual ~Effect() = default;

    /**
     * @brief Initialize the effect when it first becomes active
     * @param strip Pointer to the WS2812FX instance
     * @return true if initialization was successful
     */
    virtual bool init(WS2812FX* strip) = 0;

    /**
     * @brief Render one frame of the effect
     * @param strip Pointer to the WS2812FX instance
     * @return Delay in milliseconds until the next frame should be rendered
     */
    virtual uint16_t update(WS2812FX* strip) = 0;

    /**
     * @brief Get the name of this effect
     * @return Flash string containing the effect name
     */
    virtual const __FlashStringHelper* getName() const = 0;

    /**
     * @brief Get the mode ID for this effect
     * @return Mode ID from MODES enum
     */
    virtual uint8_t getModeId() const = 0;

    /**
     * @brief Clean up when the effect is being deactivated
     * Called before switching to a different effect
     */
    virtual void cleanup() {}

    /**
     * @brief Check if this effect supports smooth transitions
     * @return true if effect can blend smoothly with others
     */
    virtual bool supportsTransition() const { return true; }

    bool isInitialized() const {
        return initialized;
    }
    void setInitialized(bool init) {
        initialized = init;
    }
    void resetInitialized() {
        initialized = false;
    }

protected:
    bool initialized; ///< Flag to track if effect has been initialized
};

/**
 * @brief Factory class for creating effect instances
 */
class EffectFactory {
public:
    /**
     * @brief Create an effect instance by mode ID
     * @param modeId Mode ID from MODES enum
     * @return Pointer to new effect instance, or nullptr if not found
     */
    static Effect* createEffect(uint8_t modeId);

    /**
     * @brief Register an effect creator function
     * @param modeId Mode ID from MODES enum
     * @param creator Function that creates the effect
     */
    static void registerEffect(uint8_t modeId, Effect* (*creator)());

private:
    // Creator function type
    typedef Effect* (*EffectCreator)();
    
    // Maximum number of effects
    static const uint8_t MAX_EFFECTS = 64;
    
    // Registration storage
    static uint8_t registeredModes[MAX_EFFECTS];
    static EffectCreator creators[MAX_EFFECTS];
    static uint8_t numRegistered;
};

/**
 * @brief Macro to register an effect class
 * Usage: REGISTER_EFFECT(FX_MODE_STATIC, StaticEffect)
 */
#define REGISTER_EFFECT(mode_id, effect_class) \
    namespace { \
        Effect* create##effect_class() { return new effect_class(); } \
        struct effect_class##Registrar { \
            effect_class##Registrar() { \
                EffectFactory::registerEffect(mode_id, create##effect_class); \
            } \
        }; \
        static effect_class##Registrar effect_class##_registrar; \
    }

#endif // WS2812FX_EFFECT_H