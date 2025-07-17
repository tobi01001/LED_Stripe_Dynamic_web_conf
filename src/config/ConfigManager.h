/*************************************************************
 * ConfigManager.h - Configuration and EEPROM management
 * 
 * Extracted from httpledstripe_esp.cpp to improve code organization
 * and separation of concerns.
 * 
 * This class encapsulates configuration persistence:
 * - EEPROM read/write operations
 * - CRC validation
 * - Default value management
 * - Runtime settings tracking
 *************************************************************/

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <EEPROM_Rotate.h>
#include <LittleFS.h>

// Forward declaration of WS2812FX segment structure
struct WS2812FXSegment;

/**
 * Manages configuration persistence and validation
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Core configuration management
    bool initialize(size_t eepromSize);
    bool loadConfiguration();
    bool saveConfiguration();
    void resetToDefaults();
    void factoryReset();

    // Runtime settings management
    void markForSave() { shouldSave = true; }
    bool needsSaving() const { return shouldSave; }
    void periodicSave(); // Call this regularly to handle auto-saving

    // File system operations
    bool deleteConfigFile();
    bool updateConfigFile();
    String readLastResetReason();
    void writeLastResetReason(const String& reason);

    // CRC operations
    void clearCRC();
    bool validateCRC(const void* data, size_t size, uint16_t storedCRC) const;
    uint16_t calculateCRC(const void* data, size_t size) const;

    // Reset management
    enum ResetType {
        NO_RESET,
        SAVE_AND_RESTART,
        RESET_DEFAULTS,
        FACTORY_RESET
    };
    
    void requestReset(ResetType type) { pendingReset = type; }
    ResetType getPendingReset() const { return pendingReset; }
    void clearPendingReset() { pendingReset = NO_RESET; }

private:
    EEPROM_Rotate eeprom;
    bool shouldSave;
    uint32_t lastSaveTime;
    ResetType pendingReset;
    
    static const uint32_t SAVE_INTERVAL_MS = 5000;
    static const uint16_t CRC_SEED = 0x5a5a;

    // Non-copyable
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

#endif // CONFIG_MANAGER_H