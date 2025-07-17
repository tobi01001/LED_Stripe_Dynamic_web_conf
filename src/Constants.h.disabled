/*************************************************************
 * Constants.h - Centralized application constants
 * 
 * This file consolidates magic numbers and configuration values
 * scattered throughout the original codebase to improve maintainability.
 *************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "defaults.h"

// Network Configuration
namespace NetworkConstants {
    static const uint16_t HTTP_PORT = 80;
    static const uint16_t OTA_PORT = 8266;
    static const uint16_t WIFI_TIMEOUT_MS = 5000;
    static const uint16_t WIFI_SETUP_TIMEOUT_SEC = 240;
    static const uint8_t MAX_WEBSOCKET_CLIENTS = 10;
    static const uint32_t WEBSOCKET_PING_INTERVAL_MS = 2000;
    static const uint8_t WIFI_ERROR_THRESHOLD = 20;
    static const uint16_t WIFI_DISCONNECT_PENALTY = 50;
    static const uint32_t WIFI_RECONNECT_TIMEOUT_MS = 60000;
}

// Display Configuration (for knob control)
namespace DisplayConstants {
    static const uint32_t DISPLAY_TIMEOUT_MS = 30000;
    static const uint32_t OPERATION_TIMEOUT_MS = 10000;
    static const uint32_t BUTTON_DEBOUNCE_MS = 200;
    static const uint32_t ROTATION_DEBOUNCE_MS = 50;
    static const uint8_t DISPLAY_FPS = 10;
    static const uint16_t CURSOR_BLINK_RATE_MS = 500;
    static const uint32_t BOOT_DELAY_MS = 2000;
    
    // Display dimensions
    static const uint8_t DISPLAY_WIDTH = 128;
    static const uint8_t DISPLAY_HEIGHT = 64;
    static const uint8_t FONT_HEIGHT_10 = 10;
    static const uint8_t FONT_HEIGHT_16 = 16;
}

// LED Configuration
namespace LEDConstants {
    static const uint8_t LED_PIN = 3; // Must be 3 for ESP8266 DMA
    static const uint8_t LED_MAX_CURRENT_MA = 37; // Per LED at full brightness
    static const uint16_t DEFAULT_PS_MAX_CURRENT_MA = 4000;
    static const uint16_t DEFAULT_CURRENT_LIMIT_MA = 2800;
    
    // Timing constants
    static const uint32_t FRAME_CALC_WAIT_MICROSEC = 400;
    static const uint32_t MIN_LED_WRITE_CYCLE_MICROSEC = (10 * LED_COUNT + 50 + FRAME_CALC_WAIT_MICROSEC);
    static const uint8_t STRIP_MIN_FPS = 10;
    static const uint8_t STRIP_MAX_FPS = 120;
    
    // Information LEDs (for status display)
    static const uint8_t NUM_INFO_LEDS = (LED_COUNT > 10 ? 10 : LED_COUNT);
}

// EEPROM and Storage
namespace StorageConstants {
    static const uint32_t EEPROM_SAVE_INTERVAL_MS = 5000;
    static const uint16_t CRC_SEED = 0x5a5a;
    static const char CONFIG_FILE_PATH[] = "/config_all.json";
    static const char RESET_REASON_FILE[] = "/lastReset.txt";
}

// Effect and Animation Constants
namespace EffectConstants {
    // Sunrise/Sunset configuration
    static const float SUNRISE_START_R = 0.0f;
    static const float SUNRISE_START_G = 0.0f;
    static const float SUNRISE_START_B = 0.0f;
    
    static const float SUNRISE_MID1_R = 67.0f;
    static const float SUNRISE_MID1_G = 4.0f;
    static const float SUNRISE_MID1_B = 0.0f;
    
    static const float SUNRISE_MID2_R = 127.0f;
    static const float SUNRISE_MID2_G = 31.0f;
    static const float SUNRISE_MID2_B = 0.0f;
    
    static const float SUNRISE_MID3_R = 191.0f;
    static const float SUNRISE_MID3_G = 63.0f;
    static const float SUNRISE_MID3_B = 3.0f;
    
    static const float SUNRISE_END_R = 255.0f;
    static const float SUNRISE_END_G = 255.0f;
    static const float SUNRISE_END_B = 255.0f;
    
    static const uint16_t DEFAULT_SUNRISE_STEPS = 1024;
    
    // Speed and brightness limits
    static const uint16_t BEAT88_MIN = 1;
    static const uint16_t BEAT88_MAX = 10000;
    static const uint8_t BRIGHTNESS_MIN = 0;
    static const uint8_t BRIGHTNESS_MAX = 255;
}

// System Constants
namespace SystemConstants {
    static const uint32_t INIT_DELAY_MS = 50;
    static const uint32_t DEBUG_INIT_DELAY_MS = 500;
    static const uint32_t SYSTEM_CHECK_INTERVAL_MS = 100;
    static const uint32_t RUNTIME_UPDATE_INTERVAL_MS = 1000;
    static const uint16_t MAX_FILENAME_LENGTH = 64;
    
    // Reset reasons
    static const char RESET_REASON_WIFI_TIMEOUT[] = "WiFi disconnect Timeout";
    static const char RESET_REASON_OTA_SUCCESS[] = "OTA success";
    static const char RESET_REASON_FACTORY_RESET[] = "Factory Reset";
    static const char RESET_REASON_DEFAULT_RESET[] = "Reset Default Values";
    static const char RESET_REASON_WIFI_TOGGLE[] = "WiFi disabled toggle";
}

// Hardware Pin Assignments (for knob control)
#ifdef HAS_KNOB_CONTROL
namespace HardwarePins {
    static const uint8_t KNOB_ENCODER_A = KNOB_C_PNA;
    static const uint8_t KNOB_ENCODER_B = KNOB_C_PNB;
    static const uint8_t KNOB_BUTTON = KNOB_C_BTN;
    static const uint8_t DISPLAY_I2C_ADDR = KNOB_C_I2C;
    static const uint8_t DISPLAY_SDA = KNOB_C_SDA;
    static const uint8_t DISPLAY_SCL = KNOB_C_SCL;
}
#endif

// Color Constants
namespace ColorConstants {
    static const uint32_t COLOR_BLACK = 0x000000;
    static const uint32_t COLOR_RED = 0xFF0000;
    static const uint32_t COLOR_GREEN = 0x00FF00;
    static const uint32_t COLOR_BLUE = 0x0000FF;
    static const uint32_t COLOR_WHITE = 0xFFFFFF;
    static const uint32_t COLOR_YELLOW = 0xFFFF00;
    static const uint32_t COLOR_ORANGE = 0xFF8000;
    
    // Debug colors (dimmed for power safety)
    static const uint32_t DEBUG_COLOR_MASK = 0x202020;
}

#endif // CONSTANTS_H