#ifndef WS2812FX_DEFAULTS_h
#define WS2812FX_DEFAULTS_h

// The delay being used for several init phases.
#ifdef DEBUG
#define INITDELAY 500
#else
#define INITDELAY 50
#endif

#ifndef BUILD_VERSION
  #error "Build Version unknown"
#endif
//in case of "major upgrades affecting e.g. the structure of field, set this to > 0
#define RESET_DEFAULTS 1

#ifndef PIO_SRC_REV
  #define PIO_SRC_REV "no_git_rev"
#endif
#ifndef PIO_SRC_BRANCH
  #define PIO_SRC_BRANCH "no_git_branch"
#endif

#define BUILD_GITREV PIO_SRC_REV

/* use build flags to define these */
#ifndef LED_NAME
  #error "You need to give your LED a unique Name (build flag e.g. '-DLED_NAME=\"My_LED\"')!"
#endif

#ifdef LED_COUNT_TOT
#ifndef LED_OFFSET
#warning "You did Specify a total LED Count (LED_COUNT_TOT) but no LED_OFFSET. We will use 0 as offset and LED_COUNT equals LED_COUNT_TOT!"
#define LED_OFFSET 0
#endif
#define LED_COUNT (LED_COUNT_TOT-LED_OFFSET)
#else
#define LED_COUNT_TOT LED_COUNT
#define LED_OFFSET 0
#endif

#ifndef LED_COUNT
#error "You need to define the number of Leds by LED_COUNT (build flag e.g. -DLED_COUNT=50)"
#elif LED_COUNT < 11
#error "You need to define more than 10 LEDs by LED_COUNT (build flag e.g. -DLED_COUNT=50)"
#elif LED_COUNT > 300
#warning "This Code was written for up to 300 leds and was not tested for anything beyond"
#endif

// Other parameters being used
#define SRSS_StartR 0.0
#define SRSS_StartG 0.0
#define SRSS_StartB 0.0

#define SRSS_Mid1R  67.0
#define SRSS_Mid1G  4.0
#define SRSS_Mid1B  0.0

#define SRSS_Mid2R  127.0
#define SRSS_Mid2G  31.0
#define SRSS_Mid2B  0.0

#define SRSS_Mid3R  191.0
#define SRSS_Mid3G  63.0
#define SRSS_Mid3B  3.0

#define SRSS_EndR   255.0
#define SRSS_EndG   255.0//200.0
#define SRSS_EndB   255.0//128.0

#define SRSS_StartValue 0.0
#define SRSS_MidValue 512.0
#define SRSS_Endvalue 1023.0
#define DEFAULT_SUNRISE_STEPS   1024

#define EEPROM_SAVE_INTERVAL_MS 5000
#define WIFI_TIMEOUT            5000 // changed for issue #13 
#define MAX_NUM_SEGMENTS        ((LED_COUNT / 15)>10?10:(LED_COUNT / 15))
#define MAX_NUM_BARS_FACTOR     20                    // Segment divided by this defines the maximum number of "bars"
#define MAX_NUM_BARS            ((LED_COUNT / MAX_NUM_BARS_FACTOR)>10?10:(LED_COUNT / MAX_NUM_BARS_FACTOR))

// default strip / segment values below (before being stored / after "reset")


#define BEAT88_MIN 1
#define BEAT88_MAX 10000

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255

#define LED_PIN 3 // Needs to be 3 (raw value) for ESP8266 because of DMA
#define LED_MAX_CURRENT 37 // the current one (RGB) LED is drawing at full brightness incl. controller
#ifndef DEFAULT_PS_MAX_CURRENT
  #define DEFAULT_PS_MAX_CURRENT  4000 // the maximum rated current of the power supply inc. cabling to the leds
#endif 
#define DEFAULT_CURRENT_MAX ((LED_COUNT * LED_MAX_CURRENT) < DEFAULT_PS_MAX_CURRENT ? (LED_COUNT * LED_MAX_CURRENT) : DEFAULT_PS_MAX_CURRENT)
#define DEFAULT_CURRENT ((LED_COUNT * LED_MAX_CURRENT) < 2800 ? (LED_COUNT * LED_MAX_CURRENT) : 2800)
// the value of 300 microseconds is the average between two service routine calls.... 400 seems to be a good interval.
#define FRAME_CALC_WAIT_MICROINTERVAL ((uint32_t)400)
#define MIN_LED_WRITE_CYCLE (10 * LED_COUNT + 50 + FRAME_CALC_WAIT_MICROINTERVAL)
#define STRIP_MIN_FPS  (10)
#define STRIP_MAX_FPS  (((((1000*1000)/MIN_LED_WRITE_CYCLE))) < 120 ? ((((1000*1000)/MIN_LED_WRITE_CYCLE))) : 120)        // Depends on LED count...
#define DEFAULT_WIFI_ENABLED    (true)
#define STRIP_VOLTAGE 5            // fixed to 5 volts
#define STRIP_MILLIAMPS (DEFAULT_CURRENT) // can be changed during runtime
#define NUM_INFORMATION_LEDS (10<LED_COUNT?10:LED_COUNT)



#define DEFAULT_RUNNING 1
#ifndef DEFAULT_POWER     // can be set via compile flag to somewhat address #15
  #define DEFAULT_POWER 0 // starts being switched off
#elif DEFAULT_POWER < 0
  #define DEFAULT_POWER 0
#endif
#ifndef DEFAULT_MODE     // can be set via compile flag to somewhat address #15
  #define DEFAULT_MODE 0 // Static
#elif DEFAULT_MODE < 0
  #define DEFAULT_MODE 0
#endif
#define DEFAULT_BRIGHTNESS 200        // 0 to 255

#define DEFAULT_SPEED 1000            // fair value
#define DEFAULT_BLEND ((TBlendType)1) // equals LinearBlend - would need Fastled.h included to have access to the enum
#define DEFAULT_BLENDING 255          // no blend
#define DEFAULT_REVERSE 0
#ifndef DEFAULT_NUM_SEGS
  #define DEFAULT_NUM_SEGS (LED_COUNT<100?1:2) // Strips with less than 100 leds will default to 1 segment at start, others to 2.
#elif DEFAULT_NUM_SEGS < 1
  #define DEFAULT_NUM_SEGS (LED_COUNT<100?1:2) // Strips with less than 100 leds will default to 1 segment at start, others to 2.
#elif DEFAULT_NUM_SEGS > MAX_NUM_SEGMENTS
  #define DEFAULT_NUM_SEGS MAX_NUM_SEGMENTS
#endif
#define DEFAULT_MIRRORED 1             // mirrored - effect with more than one seg only...
#define DEFAULT_INVERTED 0             // invert all the colors (makes it pretty bright) FIXME: reconsider this option
#define DEFAULT_HUE_INT 500              // Hue does change over time
#define DEFAULT_HUE_OFFSET 0           // Hue default value. No offset
#define DEFAULT_AUTOMODE AUTO_MODE_OFF // auto mode change off
#define DEFAULT_T_AUTOMODE 60          // auto mode change every 60 seconds
#define DEFAULT_T_AUTOMODE_MIN 5
#define DEFAULT_T_AUTOMODE_MAX 1000


// TODO: Random Mode change? - done / Selectable list - open?
#define DEFAULT_AUTOCOLOR AUTO_MODE_OFF // auto palette change off
#define DEFAULT_T_AUTOCOLOR 60          // auto palette change every 60 seconds
#define DEFAULT_T_AUTOCOLOR_MIN 5
#define DEFAULT_T_AUTOCOLOR_MAX 1000
#define DEFAULT_COOLING 128
#define DEFAULT_COOLING_MIN 20
#define DEFAULT_COOLING_MAX 100
#define DEFAULT_SPARKING 128
#define DEFAULT_SPARKING_MIN 50
#define DEFAULT_SPARKING_MAX 200
#define DEFAULT_TWINKLE_S 4
#define DEFAULT_TWINKLE_S_MIN 0
#define DEFAULT_TWINKLE_S_MAX 8
#define DEFAULT_TWINKLE_NUM 4
#define DEFAULT_TWINKLE_NUM_MIN 0
#define DEFAULT_TWINKLE_NUM_MAX 8
#define DEFAULT_LED_BARS ((LED_COUNT / 40) > 0 ? LED_COUNT / 40 : 1) // half of the possible bars will be used
#define DEFAULT_DAMPING 90
#define DEFAULT_DAMPING_MIN 1 
#define DEFAULT_DAMPING_MAX 100
#define DEFAULT_SUNRISETIME 15
#define DEFAULT_SUNRISETIME_MIN 1
#define DEFAULT_SUNRISETIME_MAX 120
#define DEFAULT_DITHER 0
#ifndef DEFAULT_PALETTE     // can be set via compile flag
  #define DEFAULT_PALETTE 0
#endif
#define DEFAULT_COLOR_TEMP 19
#define DEFAULT_GLITTER_ADD 0
#define DEFAULT_GLITTER_WHITE 1
#define DEFAULT_GLITTER_ONBLACK 0
#define DEFAULT_GLITTER_SYNC 0

//#define RND_PAL_MIN_SAT 224
#define RND_PAL_CHANGE_INT 200 
#define RND_PAL_MIN_BRIGHT 128

// Background Color
#define DEFAULT_BCKND_HUE 0
#define DEFAULT_BCKND_SAT 255
#define DEFAULT_BCKND_BRI 0 //means OFF
#define BCKND_MIN_BRI     0
#define BCKND_MAX_BRI     80

// KNOB CONTROL
#ifdef HAS_KNOB_CONTROL
  #define DEFAULT_WIFI_DISABLED    (false)
  #define KNOB_C_SDA 4
  #define KNOB_C_SCL 5
  #define KNOB_C_BTN 2
  #define KNOB_C_PNA 12
  #define KNOB_C_PNB 13
  #define KNOB_C_I2C 0x3c
  #define KNOB_BTN_DEBOUNCE 200
  #define KNOB_ROT_DEBOUNCE 20
  #define KNOB_BOOT_DELAY 1000
  #define KNOB_TIMEOUT_OPERATION 15000 //0
  #define KNOB_TIMEOUT_DISPLAY   240000 //0
  #define KNOB_DISPLAY_FPS       25
  #define KNOB_CURSOR_BLINK      500
#endif

#endif