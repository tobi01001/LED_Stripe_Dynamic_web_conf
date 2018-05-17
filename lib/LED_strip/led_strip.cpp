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

// control special effects
//bool sunrise_running = false;
bool stripWasOff = true;
bool stripIsOn = true;
extern bool shouldSaveRuntime;


uint8_t currentEffect = FX_NO_FX;
uint8_t previousEffect = FX_NO_FX;

mysunriseParam sunriseParam;

pah_color myColor(0,   512, 1024,
                  0,   0, 0,
                  67,  5,  2,
                  127,  31, 0,
                  191, 63, 3,
                  255, 200,  128);


WS2812FX *strip;  

// set all pixels to 'off'
void stripe_setup(  const uint16_t LEDCount, 
                    const uint8_t FPS = 60, 
                    const uint8_t volt = 5, 
                    const uint16_t milliamps = 500, 
                    const CRGBPalette16 pal = Rainbow_gp, 
                    const String Name = "Custom",
                    const LEDColorCorrection colc = TypicalLEDStrip ){
  strip = new WS2812FX(LEDCount, FPS, volt, milliamps, pal, Name, colc);
  //initialize the stripe
  //strip->setLength(LEDCount);

  //strip->setPin(dataPin);

  //strip->updateType(pixelType);

  //strip->begin();
  //strip->clear();
  strip->init();
  strip->setBrightness(DEFAULT_BRIGHTNESS);
  strip->setSpeed(DEFAULT_BEAT88);
  //strip->setColor(0xff9900);
  strip->start();
  strip->show();
}

void strip_On_Off(bool onOff){
    stripIsOn = onOff;
    stripWasOff = false;
}

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b) {
  if(start >= strip->getLength() || stop >= strip->getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip->leds[i] = CRGB(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
  }
  //strip->setColor(r, g, b);
  strip->show();
}

void set_Range(uint16_t start, uint16_t stop, uint32_t color) {
  if(start >= strip->getLength() || stop >= strip->getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip->leds[i] = CRGB(color);
  }
  //strip->setColor(color);
  FastLED.show();
}

void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  if(pixel >= strip->getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  //strip->setPixelColor(pixel, r, g, b);
  strip->leds[pixel] = CRGB(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
  //strip->setColor(r, g, b);
  strip->show();
}

void strip_setpixelcolor(uint16_t pixel, uint32_t color) {
  if(pixel >= strip->getLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  strip->leds[pixel] = CRGB(color);
  //strip->setPixelColor(pixel, color);
  //strip->setColor(color);
  strip->show();
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
      strip->service();
      break;
    default:
      reset();
  }
}

// Sets a new Effect to be called
void setEffect(uint8_t Effect){
  //if(Effect != FX_WS2812) reset(); // Only reset (with fade) for non-WS2812FX as we have the fading build-in
  //previousEffect = currentEffect;
  currentEffect = Effect;
  strip_On_Off(true);
  if(strip->getBrightness()<1)
  {
    strip->setBrightness(10);
  }
  if(Effect == FX_WS2812) {
    strip->start();
    strip->trigger();
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
  uint32_t dimColor = strip_color32(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}

// Dim the Pixel to DimColor
void strip_dimPixel(uint16_t pixel, bool dim_default, uint8_t byValue){
  if(dim_default)
    strip->leds[pixel].fadeToBlackBy(2);
  else
  {
    strip->leds[pixel].subtractFromRGB(byValue);
    /*
    uint32_t color = strip->leds[pixel].raw;
    uint8_t r = Red(color);
    uint8_t g = Green(color);
    uint8_t b = Blue(color);
    if(r < byValue) r=0;
    else r -= byValue;
    if(g < byValue) g=0;
    else g -= byValue;
    if(b < byValue) b=0;
    else b -= byValue;
    strip->setPixelColor(pixel, r, g, b);
    */
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
  //strip->clear();
  strip->setBrightness(BRIGHTNESS_MAX);
  strip->show();
  shouldSaveRuntime = true;
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
    // ToDo: We change to palette and move along the palette...
    uint32_t cColor = myColor.calcColorValue(sunriseParam.step);
    for(uint16_t i = 0; i<strip->getLength();i++)
    {
      //strip->setPixelColor(i, cColor);
      strip->leds[i] = CRGB(cColor);
    }
    //strip->setColor(cColor);
    strip->show();
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
  /*uint8_t max = 0;
  uint32_t color = 0;
  for(uint8_t i = 0; i<strip->getLength(); i++) {
    if(strip->leds[i].r >max ) max = strip->leds[i].r;
    if(strip->leds[i].g >max ) max = strip->leds[i].g;
    if(strip->leds[i].b >max ) max = strip->leds[i].b;
  }
  uint8_t r,g,b;

  // ToDo: Fade To Black!

  for(uint8_t i = 0; i<max; i++) {
    for(uint16_t p=0; p<strip->getLength(); p++)
    {
        r = Red(strip->getPixelColor(p));
        g = Green(strip->getPixelColor(p));
        b = Blue(strip->getPixelColor(p));
        if(r>0) r--;
        if(g>0) g--;
        if(b>0) b--;
        strip->leds[p].subtractFromRGB(2);
        //strip->setPixelColor(p, r, g, b);
    }
    strip->show();
    //delay(1);
  }
  */
  bool isBlack = true;
  do {
    isBlack = true;
    fadeToBlackBy(strip->leds, strip->getLength(), 16);
    for(uint16_t i = 0; i < strip->getLength(); i++)
    {
      if(strip->leds[i]) isBlack = false;
    }
    FastLED.show();
  } while (!isBlack);
}
