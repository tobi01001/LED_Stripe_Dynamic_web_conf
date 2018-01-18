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

#include <WS2812FX.h>
#include <Adafruit_NeoPixel.h>

/* should be in library now....
// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
*/

#define DEFAULT_PIXEL_TYPE (NEO_GRB + NEO_KHZ800)

#define FX_NO_FX        0
#define FX_FIRE         1
#define FX_RAINBOW      2
#define FX_BLINKER      3
#define FX_SPARKS       4
#define FX_WHITESPARKS  5
#define FX_KNIGHTRIDER  6
#define FX_SUNRISE      7
#define FX_SUNSET       8
#define FX_WS2812       9



// initialize the strip during boot (or at changes)
// strip can be any neopixel arrangement
// but is currently limited to NEO_GRB
void stripe_setup(uint16_t LEDCount, uint8_t dataPin, neoPixelType pixelType);

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


// fire Effect (not WS2812BFX)
void fireEffect(void) ;
// Rainbow  (not WS2812BFX)
void rainbowCycle(void);
// Blinker Effect (not WS2812BFX)
void blinkerEffect(void);
// Sparks Effect (not WS2812BFX)
void sparksEffect(void);
// WhiteSparks(not WS2812BFX)
void white_sparksEffect(void);
// Knightrider (not WS2812BFX)
void knightriderEffect(void);

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos);

//noch benutzt?
int colorVal(char c);

// dims a single pixel either by right shift (division by 2)
//  or by a certain value (enables smoother but also slower dims)
void strip_dimPixel(uint16_t pixel, bool dim_default, uint8_t byValue);

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

void stripe_setDelayInterval(uint16_t delay);

uint16_t stripe_getDelayInterval();

void strip_off(void);

void strip_On_Off(bool onOff);

void stripe_setBrightness(uint8_t brightness);

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b);

void mySunriseStart(uint32_t  mytime, uint16_t steps, bool up);

void mySunriseTrigger(void);


//globals... here or in the ccp file? - use it here for the moment
/* Globals */
// ToDo: Redefine for effectiveness (static etc)
extern uint16_t fx_blinker_start_pixel;
extern uint16_t fx_blinker_end_pixel;
extern uint8_t fx_blinker_red;
extern uint8_t fx_blinker_green;
extern uint8_t fx_blinker_blue;
extern uint16_t fx_blinker_time_on;
extern uint16_t fx_blinker_time_off;

// control special effects
extern bool sunrise_running;
extern bool stripWasOff;
extern bool stripIsOn;


extern unsigned long last_delay_trigger;

extern uint8_t currentEffect;
extern uint8_t previousEffect;

extern uint16_t rainbowColor;

extern uint16_t delay_interval;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
extern WS2812FX strip; // = WS2812FX(1, 1, DEFAULT_PIXEL_TYPE); // WS2812FX(strip.getLength(), LEDPIN, NEO_GRB + NEO_KHZ800);


#endif
