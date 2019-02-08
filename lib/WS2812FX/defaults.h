#ifndef WS2812FX_DEFAULTS_h
#define WS2812FX_DEFAULTS_h

// The delay being used for several init phases.
#ifdef DEBUG
#define INITDELAY 500
#else
#define INITDELAY 2
#endif

/* use build flags to define these */
#ifndef LED_NAME
#error "You need to give your LED a Name (build flag e.g. '-DLED_NAME=\"My LED\"')!"
#endif

#ifndef LED_COUNT
#error "You need to define the number of Leds by LED_COUNT (build flag e.g. -DLED_COUNT=50)"
#endif

// Other parameters being used

#define DEFAULT_SUNRISE_STEPS   1024
#define EEPROM_SAVE_INTERVAL_MS 5000
#define WIFI_TIMEOUT            5000

// default strip / segment values below (before being stored / after "reset")


#define BEAT88_MIN 1
#define BEAT88_MAX 10000

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255

#define LED_PIN 3 // Needs to be 3 (raw value) for ESP8266 because of DMA
#define DEFAULT_MAX_CURRENT 2800
#define STRIP_FPS 111              // set to 111 as this is the max we can get out of 300 WS2812 leds....
#define STRIP_VOLTAGE 5            // fixed to 5 volts
#define STRIP_MILLIAMPS ((LED_COUNT * 60) < DEFAULT_MAX_CURRENT ? LED_COUNT * 60 : DEFAULT_MAX_CURRENT) // can be changed during runtime

#define DEFAULT_RUNNING 1
#define DEFAULT_POWER 0 // starts being switched off
#define DEFAULT_MODE 0
#define DEFAULT_BRIGHTNESS 255        // 0 to 255
#define DEFAULT_EFFECT 0              // 0 to modecount
#define DEFAULT_PALETTE 0             // 0 is rainbow colors
#define DEFAULT_SPEED 1000            // fair value
#define DEFAULT_BLEND ((TBlendType)1) // equals LinearBlend - would need Fastled.h included to have access to the enum
#define DEFAULT_BLENDING 255          // no blend
#define DEFAULT_REVERSE 0
#define DEFAULT_NUM_SEGS 1             // one segment only
#define DEFAULT_MIRRORED 1             // mirrored - effect with more than one seg only...
#define DEFAULT_INVERTED 0             // invert all the colors (makes it pretty bright) FIXME: reconsider this option
#define DEFAULT_HUE_INT 0              // Hue does not change over time
#define DEFAULT_HUE_OFFSET 0           // Hue default value. No offset
#define DEFAULT_AUTOMODE AUTO_MODE_OFF // auto mode change off
#define DEFAULT_T_AUTOMODE 60          // auto mode change every 60 seconds

// TODO: Random Mode change? - done / Selectable list - open?
#define DEFAULT_AUTOCOLOR AUTO_MODE_OFF // auto palette change off
#define DEFAULT_T_AUTOCOLOR 60          // auto palette change every 60 seconds
#define DEFAULT_COOLING 128             //
#define DEFAULT_SPAKRS 128
#define DEFAULT_TWINKLE_S 4
#define DEFAULT_TWINKLE_NUM 4
#define DEFAULT_LED_BARS ((LED_COUNT / 40) > 0 ? LED_COUNT / 40 : 1) // half of the possible bars will be used
#define DEFAULT_DAMPING 90
#define DEFAULT_SUNRISETIME 15
#define DEFAULT_DITHER 1
#define DEFAULT_PALETTE 0
#define DEFAULT_COLOR_TEMP 19


#endif