/*************************************************************
 * ConfigManager.cpp - Configuration and EEPROM management implementation
 *************************************************************/

#include "ConfigManager.h"
#include "../../include/defaults.h"
#include "../LED_strip/led_strip.h"

ConfigManager::ConfigManager() 
    : shouldSave(false)
    , lastSaveTime(0)
    , pendingReset(NO_RESET)
{
}

ConfigManager::~ConfigManager() {
    if (shouldSave) {
        saveConfiguration();
    }
}

bool ConfigManager::initialize(size_t eepromSize) {
    eeprom.begin(eepromSize);
    return true;
}

bool ConfigManager::loadConfiguration() {
    if (!strip) {
        return false;
    }

    // Read segment data from EEPROM
    WS2812FX::segment seg;
    eeprom.get(0, seg);

    // Calculate CRC for validation
    uint16_t calculatedCRC = calculateCRC((unsigned char*)&seg + 2, sizeof(seg) - 2);

    if (seg.CRC == calculatedCRC) {
        // Valid configuration found, apply it
        *(strip->getSegment()) = seg;
        strip->init();
        return true;
    } else {
        // Invalid CRC, use defaults
        resetToDefaults();
        return false;
    }
}

bool ConfigManager::saveConfiguration() {
    if (!strip || !shouldSave) {
        return false;
    }

    shouldSave = false;

    // Get current segment data
    WS2812FX::segment seg = *(strip->getSegment());
    
    // Calculate and store CRC
    seg.CRC = calculateCRC((unsigned char*)&seg + 2, sizeof(seg) - 2);
    strip->setCRC(seg.CRC);

    // Write to EEPROM
    eeprom.put(0, seg);
    eeprom.commit();

    lastSaveTime = millis();
    return true;
}

void ConfigManager::resetToDefaults() {
    if (strip) {
        strip->resetDefaults();
    }
    shouldSave = true;
}

void ConfigManager::factoryReset() {
    // Clear EEPROM
    for (uint32_t i = 0; i < eeprom.length(); i++) {
        eeprom.write(i, 0);
    }
    eeprom.commit();
    
    // Delete config files
    deleteConfigFile();
    
    // Clear WiFi settings would need to be handled by NetworkManager
    writeLastResetReason(F("Factory Reset"));
}

void ConfigManager::periodicSave() {
    uint32_t now = millis();
    
    if (shouldSave && (now - lastSaveTime) >= SAVE_INTERVAL_MS) {
        saveConfiguration();
    }
}

bool ConfigManager::deleteConfigFile() {
    if (LittleFS.exists(F("/config_all.json"))) {
        return LittleFS.remove(F("/config_all.json"));
    }
    return true;
}

bool ConfigManager::updateConfigFile() {
    DynamicJsonDocument doc(1024);
    JsonArray root = doc.to<JsonArray>();
    
    // This would need access to field definitions
    // getAllJSON(root);

    File configFile = LittleFS.open(F("/config_all.json"), "w");
    if (!configFile) {
        return false;
    }

    serializeJson(doc, configFile);
    configFile.close();
    return true;
}

String ConfigManager::readLastResetReason() {
    File f = LittleFS.open(F("/lastReset.txt"), "r");
    if (!f) {
        return F("FS upload? File not found");
    }
    
    String reason = f.readStringUntil((char)13);
    f.close();
    return reason;
}

void ConfigManager::writeLastResetReason(const String& reason) {
    File f = LittleFS.open(F("/lastReset.txt"), "w");
    if (!f) {
        return;
    }
    
    // Add some entropy to the reason
    uint16_t entropy = random(65536);
    f.println(reason + " " + String(entropy));
    f.close();
}

void ConfigManager::clearCRC() {
    eeprom.begin(sizeof(uint16_t));
    eeprom.put(0, (uint16_t)0);
    eeprom.commit();
    eeprom.end();
}

bool ConfigManager::validateCRC(const void* data, size_t size, uint16_t storedCRC) const {
    uint16_t calculatedCRC = calculateCRC(data, size);
    return calculatedCRC == storedCRC;
}

uint16_t ConfigManager::calculateCRC(const void* data, size_t size) const {
    if (!strip) {
        return 0;
    }
    return (uint16_t)strip->calc_CRC16(CRC_SEED, (unsigned char*)data, size);
}