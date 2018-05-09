/**************************************************************
   This work is based on many others.
   If published then under whatever licence free to be used,
   distibuted, changed and whatsoever.
   Work is based on:
   WS2812BFX library by  - see:
   fhem esp8266 implementation by   - see:
   WiFiManager library by - - see:
   ... many others ( see includes)

   ToDo: Make this a class (encapsulation)

 **************************************************************/

#ifndef led_strip_h
#define led_strip_h

#include "WS2812FX.h"
//#include <Adafruit_NeoPixel.h>

//ToDo: Rework for ColorPalettes
#include "pahcolor.h"

/* should be in library now....
// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
*/

//#define DEFAULT_PIXEL_TYPE (NEO_GRB + NEO_KHZ800)

#define FX_NO_FX        0
#define FX_SUNRISE      1
#define FX_SUNSET       2
#define FX_WS2812       3



// initialize the strip during boot (or at changes)
// strip can be any neopixel arrangement
// but is currently limited to NEO_GRB
void stripe_setup(  const uint16_t LEDCount, 
                    const uint8_t FPS, 
                    const uint8_t volt , 
                    const uint16_t milliamps , 
                    const CRGBPalette16 pal , 
                    const String Name ,
                    const LEDColorCorrection colc);

// resets the current effects and stops the strip
void reset(void);

// just calls the right effec routine according to the current Effect
void effectHandler(void);

// Sets a new Effect to be called
void setEffect(uint8_t Effect);

// returns the current Effect
uint8_t getEffect(void);

// return the previous effect
uint8_t getPreviousEffect(void);

// 32 Bit Color out of 3 Color values....
uint32_t strip_color32(uint8_t r, uint8_t g, uint8_t b);

// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color);

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color);

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color);

// helper for long delays (prevents watchdog reboot)
void delaymicro(unsigned int mics);

// set color for a single pixel (deactivates effects)
// but also sets the color for the effect library
void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);
void strip_setpixelcolor(uint16_t pixel, uint32_t color);

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b);
void set_Range(uint16_t start, uint16_t stop, uint32_t color);

void stripe_setDelayInterval(uint16_t delay);

uint16_t stripe_getDelayInterval();

void strip_off(void);

void strip_On_Off(bool onOff);

void stripe_setBrightness(uint8_t brightness);

void mySunriseStart(uint32_t  mytime, uint16_t steps, bool up);

void mySunriseTrigger(void);

// ToDo: To be reqorked for the FastLED approach
typedef struct sunriseParam {
  bool isRunning;
  bool isSunrise;
  uint16_t steps;
  uint16_t step;
  uint32_t deltaTime;
  uint32_t lastChange;
} mysunriseParam;


extern bool sunrise_running;
extern bool stripWasOff;
extern bool stripIsOn;


//extern unsigned long last_delay_trigger;

extern uint8_t currentEffect;
extern uint8_t previousEffect;

extern pah_color myColor;

extern mysunriseParam sunriseParam;


extern WS2812FX *strip; // = WS2812FX(1, 1, DEFAULT_PIXEL_TYPE); // WS2812FX(strip.getLength(), LEDPIN, NEO_GRB + NEO_KHZ800);


#endif
