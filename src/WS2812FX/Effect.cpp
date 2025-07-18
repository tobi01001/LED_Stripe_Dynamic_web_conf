#include "Effect.h"

// Static member definitions for EffectFactory
uint8_t EffectFactory::registeredModes[EffectFactory::MAX_EFFECTS];
EffectFactory::EffectCreator EffectFactory::creators[EffectFactory::MAX_EFFECTS];
uint8_t EffectFactory::numRegistered = 0;

Effect* EffectFactory::createEffect(uint8_t modeId) {
    for (uint8_t i = 0; i < numRegistered; i++) {
        if (registeredModes[i] == modeId) {
            return creators[i]();
        }
    }
    return nullptr;
}

void EffectFactory::registerEffect(uint8_t modeId, EffectCreator creator) {
    if (numRegistered < MAX_EFFECTS) {
        registeredModes[numRegistered] = modeId;
        creators[numRegistered] = creator;
        numRegistered++;
    }
}