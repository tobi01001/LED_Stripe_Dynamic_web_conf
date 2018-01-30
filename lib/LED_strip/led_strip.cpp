/**************************************************************
   This work is based on many others.
   If published then under whatever licence free to be used,
   distibuted, changed and whatsoever.
   Work is based on:
   WS2812BFX library by  - see:
   fhem esp8266 implementation by   - see:
   WiFiManager library by - - see:
   ... many others ( see includes)

 **************************************************************/
//#define DEBUG

#ifndef led_strip_h
#include "led_strip.h"
#endif

#include <pahcolor.h>


//globals... here or in the ccp file? - use it here for the moment
/* Globals */
// ToDo: Redefine for effectiveness (static etc)
/*
uint16_t fx_blinker_start_pixel;
uint16_t fx_blinker_end_pixel;
uint8_t fx_blinker_red;
uint8_t fx_blinker_green;
uint8_t fx_blinker_blue;
uint16_t fx_blinker_time_on;
uint16_t fx_blinker_time_off;
*/
// control special effects
bool sunrise_running = false;
bool stripWasOff = true;
bool stripIsOn = true;

//unsigned long last_delay_trigger = 0;

uint8_t currentEffect = FX_NO_FX;
uint8_t previousEffect = FX_NO_FX;

//uint16_t rainbowColor=0;

//uint16_t delay_interval = 50;

mysunriseParam sunriseParam;

pah_color myColor(0,   512, 1024,
                  0,   0, 0,
                  67,  5,  2,
                  127,  31, 0,
                  191, 63, 3,
                  255, 200,  128);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
WS2812FX strip = WS2812FX(1,3, DEFAULT_PIXEL_TYPE); // use no constructor at all? old: = WS2812FX(300, 1, DEFAULT_PIXEL_TYPE); // WS2812FX(strip.getLength(), LEDPIN, NEO_GRB + NEO_KHZ800);

/* obsolete
void stripe_setDelayInterval(uint16_t delay) {
  // new speed in ws2812fx library is 10 to 65535
  // we use the old "delay" but may multiply to get a new speed for ws2812fx
  uint16_t speed = (delay*257);
  if(delay > 255) delay = 255;
  //if(delay < 1) delay = 1;
  if(speed > SPEED_MAX) speed = SPEED_MAX;
  if(speed < SPEED_MIN) speed = SPEED_MIN;
  strip.setSpeed(speed);
  delay_interval = delay;
}

uint16_t stripe_getDelayInterval(){
  return delay_interval;
}
*/ // end obsolete

// set all pixels to 'off'
void stripe_setup(uint16_t LEDCount, uint8_t dataPin, neoPixelType pixelType){
  //initialize the stripe
  strip.setLength(LEDCount);

  strip.setPin(dataPin);

  strip.updateType(pixelType);

  strip.begin();
  strip.clear();
  strip.init();
  strip.setBrightness(150);
  strip.setSpeed(1000);
  strip.setColor(0xff9900);
  strip.start();
  strip.show();
}

void strip_On_Off(bool onOff){
    stripIsOn = onOff;
    stripWasOff = false;
}

/*
void stripe_setBrightness(uint8_t brightness){
  strip.setBrightness(brightness);
  strip.show();
}
*/

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b) {
  if(start >= strip.getLength() || stop >= strip.getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.setColor(r, g, b);
  strip.show();
}

void set_Range(uint16_t start, uint16_t stop, uint32_t color) {
  if(start >= strip.getLength() || stop >= strip.getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip.setPixelColor(i, color);
  }
  strip.setColor(color);
  strip.show();
}

void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  if(pixel >= strip.getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  strip.setPixelColor(pixel, r, g, b);
  strip.setColor(r, g, b);
  strip.show();
}

void strip_setpixelcolor(uint16_t pixel, uint32_t color) {
  if(pixel >= strip.getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  strip.setPixelColor(pixel, color);
  strip.setColor(color);
  strip.show();
}

// just calls the right effec routine according to the current Effect
void effectHandler(void){
  //switching on or OFF

  if(stripIsOn && stripWasOff)
  {
    setEffect(getPreviousEffect());
    stripWasOff = false;
  }
  else if (!stripIsOn && !stripWasOff)
  {
    reset();
    stripWasOff = true;
  }
  else
  {
    // noting currently
  }

  switch (currentEffect) {
    case FX_NO_FX :
      break;
    case FX_SUNRISE :
    case FX_SUNSET :
      if(sunriseParam.isRunning)
      {
        mySunriseTrigger();
      }
      break;
    case FX_WS2812 :
      strip.service();
      break;
    default:
      reset();
  }
}

// Sets a new Effect to be called
void setEffect(uint8_t Effect){
  reset();
  //previousEffect = currentEffect;
  currentEffect = Effect;
  strip_On_Off(true);
  if(strip.getBrightness()<1)
  {
    strip.setBrightness(10);
  }
  if(Effect == FX_WS2812) {
    strip.start();
    strip.trigger();
  }
}

// returns the current Effect
uint8_t getEffect(void){
  return currentEffect;
}

// return the previous effect
uint8_t getPreviousEffect(void){
  return previousEffect;
}

uint32_t strip_color32(uint8_t r, uint8_t g, uint8_t b){
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t Red(uint32_t color){
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color){
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color){
  return color & 0xFF;
}
// Dims a strip by rightshift
uint32_t DimColor(uint32_t color){
  uint32_t dimColor = strip.Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}

// Dim the Pixel to DimColor
void strip_dimPixel(uint16_t pixel, bool dim_default, uint8_t byValue){
  if(dim_default)
    strip.setPixelColor(pixel, DimColor(strip.getPixelColor(pixel)));
  else
  {
    uint32_t color = strip.getPixelColor(pixel);
    uint8_t r = Red(color);
    uint8_t g = Green(color);
    uint8_t b = Blue(color);
    if(r < byValue) r=0;
    else r -= byValue;
    if(g < byValue) g=0;
    else g -= byValue;
    if(b < byValue) b=0;
    else b -= byValue;
    strip.setPixelColor(pixel, r, g, b);
  }
}

// helper for long delays (prevents watchdog reboot)
void delaymicro(unsigned int mics){
  delayMicroseconds(mics);
  yield();
}

// sunrise funtionality... may need to do thata bit  different finally
// both for color as for logic and start/stop/running
void mySunriseStart(uint32_t  mytime, uint16_t steps, bool up) {
  sunriseParam.isRunning = true;
  sunriseParam.steps = steps;
  myColor.setStepValues(0, steps/2, steps);
  if(up)
  {
    sunriseParam.step = 0;
    sunriseParam.isSunrise = true;
  }
  else
  {
    sunriseParam.step = steps;
    sunriseParam.isSunrise = false;
  }
  sunriseParam.deltaTime = (mytime/steps);
  sunriseParam.lastChange = millis();
  //reset the stripe
  //strip.clear();
  strip.setBrightness(BRIGHTNESS_MAX);
  strip.show();
  #ifdef DEBUG
  Serial.printf("\nStarted Sunrise with %.3u steps in %u ms which are %u minutes.\n", steps, mytime, (mytime/60000));
  #endif
}

void mySunriseTrigger(void) {
  if(!sunriseParam.isRunning) return;
  uint32_t now = (uint32_t)millis();
  if(now > (uint32_t)(sunriseParam.lastChange + sunriseParam.deltaTime))
  {
    if(sunriseParam.isSunrise)
    {
      sunriseParam.step++;
    }
    else
    {
      sunriseParam.step--;
    }
    uint32_t cColor = myColor.calcColorValue(sunriseParam.step);
    for(uint16_t i = 0; i<strip.getLength();i++)
    {
      strip.setPixelColor(i, cColor);
    }
    strip.setColor(cColor);
    strip.show();
    sunriseParam.lastChange = (uint32_t)millis();
    if((sunriseParam.step >= sunriseParam.steps) || (sunriseParam.step == 0))
    {
      sunriseParam.isRunning = false;
    }
  }
}


// Reset stripe, all LED off and no effects
void reset() {
  previousEffect = currentEffect;
  currentEffect = FX_NO_FX;
  if(strip.getBrightness() < 150)
  {
    strip.setBrightness(150);
  }
  //strip.stop();
}
