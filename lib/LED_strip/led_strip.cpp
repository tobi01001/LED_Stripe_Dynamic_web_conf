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
#ifndef led_strip_h
#include "led_strip.h"
#endif

#include <pahcolor.h>


//globals... here or in the ccp file? - use it here for the moment
/* Globals */
// ToDo: Redefine for effectiveness (static etc)
uint16_t fx_blinker_start_pixel;
uint16_t fx_blinker_end_pixel;
uint8_t fx_blinker_red;
uint8_t fx_blinker_green;
uint8_t fx_blinker_blue;
uint16_t fx_blinker_time_on;
uint16_t fx_blinker_time_off;

// control special effects
bool sunrise_running = false;
bool stripWasOff = true;
bool stripIsOn = true;

unsigned long last_delay_trigger = 0;

uint8_t currentEffect = FX_NO_FX;
uint8_t previousEffect = FX_NO_FX;

uint16_t rainbowColor=0;

uint16_t delay_interval = 50;

struct sunriseParam {
  bool isRunning;
  bool isSunrise;
  uint16_t steps;
  uint16_t step;
  uint32_t deltaTime;
  uint32_t lastChange;
} sunriseParam;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
WS2812FX strip = WS2812FX(300, 1, DEFAULT_PIXEL_TYPE); // WS2812FX(strip.getLength(), LEDPIN, NEO_GRB + NEO_KHZ800);


void stripe_setDelayInterval(uint16_t delay)
{
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

uint16_t stripe_getDelayInterval()
{
  return delay_interval;
}

// set all pixels to 'off'
void stripe_setup(uint16_t LEDCount, uint8_t dataPin, neoPixelType pixelType)
{
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

void strip_On_Off(bool onOff)
{
    stripIsOn = onOff;
    stripWasOff = false;
}



void stripe_setBrightness(uint8_t brightness)
{
  strip.setBrightness(brightness);
  strip.show();
}

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b)
{
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
  strip.setColor(r, g, b);
}

void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b)
{
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  strip.setPixelColor(pixel, r, g, b);
  strip.setColor(r, g, b);
  strip.show();
}

// just calls the right effec routine according to the current Effect
void effectHandler(void)
{
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
    case FX_FIRE :
      fireEffect();
      break;
    case FX_RAINBOW :
      rainbowCycle();
      break;
    case FX_BLINKER :
      blinkerEffect();
      break;
    case FX_SPARKS :
      sparksEffect();
      break;
    case FX_WHITESPARKS :
      white_sparksEffect();
      break;
    case FX_KNIGHTRIDER :
      knightriderEffect();
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
void setEffect(uint8_t Effect)
{
  reset();
  //previousEffect = currentEffect;
  currentEffect = Effect;
  strip_On_Off(true);
  if(strip.getBrightness()<128)
  {
    strip.setBrightness(128);
  }
}


// returns the current Effect
uint8_t getEffect(void)
{
  return currentEffect;
}

// return the previous effect
uint8_t getPreviousEffect(void)
{
  return previousEffect;
}



uint32_t strip_color32(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
  return color & 0xFF;
}
// Dims a strip by rightshift
uint32_t DimColor(uint32_t color)
{
  uint32_t dimColor = strip.Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}

// Dim the Pixel to DimColor
void strip_dimPixel(uint16_t pixel, bool dim_default, uint8_t byValue)
{
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
void delaymicro(unsigned int mics)
{
  delayMicroseconds(mics);
  yield();
}

pah_color myColor(0,   500, 1000,
                  0,   0, 0,
                  67,  4,  0,
                  127,  31, 0,
                  191, 63, 3,
                  255, 200,  128);

// sunrise funtionality... may need to do thata bit  different finally
// both for color as for logic and start/stop/running
void mySunriseStart(uint32_t  mytime, uint16_t steps, bool up)
{
  sunriseParam.isRunning = true;
  sunriseParam.steps = steps;
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
  strip.clear();
  strip.show();
  Serial.printf("\nStarted Sunrise with %.3u steps in %u ms which are %u minutes.\n", steps, mytime, (mytime/60000));
}

void mySunriseTrigger(void)
{
  if(!sunriseParam.isRunning) return;
  uint32_t now = (uint32_t)millis();
  if(now > (uint32_t)(sunriseParam.lastChange + sunriseParam.deltaTime))
  {
    uint32_t cColor = myColor.calcColorValue(sunriseParam.step);
    if(sunriseParam.isSunrise)
    {
      sunriseParam.step++;
    }
    else
    {
      sunriseParam.step--;
    }
    for(uint16_t i = 0; i<strip.getLength();i++)
    {
      strip.setPixelColor(i, cColor);
    }
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

// LED flicker fire effect
void fireEffect() {
  for(int x = 0; x <strip.getLength(); x++) {
    int flicker = random(0,55);
    int r1 = 226-flicker;
    int g1 = 121-flicker;
    int b1 = 35-flicker;
    if(g1<0) g1=0;
    if(r1<0) r1=0;
    if(b1<0) b1=0;
    strip.setPixelColor(x, r1, g1, b1);
  }
  strip.show();
  delay(random(10,113));
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i;
  if((millis() - last_delay_trigger) < delay_interval) return;
  last_delay_trigger = millis();
  if (rainbowColor++>255) rainbowColor=0;
  for(i=0; i< strip.getLength(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.getLength()) + rainbowColor) & 255));
  }
  strip.show();
  //delay(delay_interval);
}

void blinkerEffect() {
 for(int i=fx_blinker_start_pixel; i<=fx_blinker_end_pixel; i++) {
    strip.setPixelColor(i, fx_blinker_red, fx_blinker_green, fx_blinker_blue);
  }
  strip.show();
 delay(fx_blinker_time_on);
 for(int i=fx_blinker_start_pixel; i<= fx_blinker_end_pixel; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
delay(fx_blinker_time_off);
}

void sparksEffect() {
  uint16_t i = random(strip.getLength());
  if((millis() - last_delay_trigger) < delay_interval) return;
  last_delay_trigger = millis();
  if (strip.getPixelColor(i)==0) {
    strip.setPixelColor(i,random(256*256*256));
  }

  for(i = 0; i < strip.getLength(); i++) {
    strip_dimPixel(i, true, 0);
  }

  strip.show();
}

void white_sparksEffect() {
  uint16_t i = random(strip.getLength());
  uint16_t rand = random(256);
  if((millis() - last_delay_trigger) < delay_interval) return;
  last_delay_trigger = millis();
  if (strip.getPixelColor(i)==0) {
    strip.setPixelColor(i,rand*256*256+rand*256+rand);
  }

  for(i = 0; i < strip.getLength(); i++) {
    strip_dimPixel(i, true, (uint8_t)(strip.getLength()*3/4));
  }

  strip.show();
  //delay(delay_interval);
}

void knightriderEffect() {

  if((millis() - last_delay_trigger) < delay_interval) return;

  uint16_t i;
  static uint16_t cur_step = 0;

  uint8_t dim_byValue = 0;
  bool dim_default = true;

  if(strip.getLength() > 16)
  {
    dim_default = false;
    dim_byValue = (uint8_t) (128/(strip.getLength()/2));
    if(dim_byValue < 2) dim_byValue = 2;
  }

  dim_default = true;

  last_delay_trigger = millis();

  cur_step+=1;

  if(cur_step>=((strip.getLength())*2)){
    cur_step=0;
  }

  if(cur_step<(strip.getLength())){
    strip.setPixelColor(cur_step, 0x808080);
    for(i=1;i<=(uint8_t)(strip.getLength()/4);i++){
      if((cur_step-i>-1)) {
        strip_dimPixel(cur_step-i, dim_default, dim_byValue);
      }
      if((cur_step+i-1)<strip.getLength()) {
        strip_dimPixel(cur_step+i-1, dim_default, dim_byValue);
      }

    }
  } else {
    strip.setPixelColor((strip.getLength())*2-cur_step-1, 0x808080);
    for(i=1;i<=(uint8_t)(strip.getLength()/4);i++){
      if(((strip.getLength())*2-cur_step-1+i<strip.getLength())) {
        strip_dimPixel((strip.getLength())*2-cur_step-1+i, dim_default, dim_byValue);
      }
      if(((strip.getLength())*2-cur_step-1-i)>-1) {
        strip_dimPixel((strip.getLength())*2-cur_step-1-i, dim_default, dim_byValue);
      }
    }
  }

  strip.show();
  //delay(delay_interval);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip_color32(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip_color32(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip_color32(WheelPos * 3, 255 - WheelPos * 3, 0);
}

int colorVal(char c) {
  int i = (c>='0' && c<='9') ? (c-'0') : (c - 'A' + 10);
  return i*i + i*2;
}
