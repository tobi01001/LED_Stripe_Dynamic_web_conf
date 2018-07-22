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

//#include <pahcolor.h>

bool stripWasOff = true;
bool stripIsOn = true;
extern bool shouldSaveRuntime;


uint8_t currentEffect = FX_NO_FX;
uint8_t previousEffect = FX_NO_FX;

mysunriseParam sunriseParam;

WS2812FX *strip;  

const String NumberFieldType = "Number";
const String BooleanFieldType = "Boolean";
const String SelectFieldType = "Select";
const String ColorFieldType = "Color";
const String SectionFieldType = "Section";

Field getField(String name, FieldList fields, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    Field field = fields[i];
    if (field.name == name) {
      return field;
    }
  }
  return Field();
}

String getFieldValue(String name, FieldList fields, uint8_t count) {
  Field field = getField(name, fields, count);
  if (field.getValue) {
    return field.getValue();
  }
  return String();
}

String setFieldValue(String name, String value, FieldList fields, uint8_t count) {
  Field field = getField(name, fields, count);
  if (field.setValue) {
    return field.setValue(value);
  }
  return String();
}

String getFieldsJson(FieldList fields, uint8_t count) {
  String json = "[";

  for (uint8_t i = 0; i < count; i++) {
    Field field = fields[i];

    json += "{\"name\":\"" + field.name + "\",\"label\":\"" + field.label + "\",\"type\":\"" + field.type + "\"";

    if(field.getValue) {
      if (field.type == ColorFieldType || field.type == "String") {
        json += ",\"value\":\"" + field.getValue() + "\"";
      }
      else {
        json += ",\"value\":" + field.getValue();
      }
    }

    if (field.type == NumberFieldType) {
      json += ",\"min\":" + String(field.min);
      json += ",\"max\":" + String(field.max);
    }

    if (field.getOptions) {
      json += ",\"options\":[";
      json += field.getOptions();
      json += "]";
    }

    json += "}";

    if (i < count - 1)
      json += ",";
  }

  json += "]";

  return json;
}

String getPower() {
  return String(stripIsOn);
}


String getBrightness() {
  return String(strip->getBrightness());
}

String getPattern() {
  return String(strip->getMode());
}

String getPatterns() {
  String json = "";

  for (uint8_t i = 0; i < strip->getModeCount(); i++) {
    json += "\"" + String(strip->getModeName(i)) + "\"";
    if (i < strip->getModeCount() - 1)
      json += ",";
  }

  return json;
}

String getPalette() {
  return String(strip->getTargetPaletteNumber());
}

String getPalettes() {
  String json = "";

  for (uint8_t i = 0; i < strip->getPalCount(); i++) {
    json += "\"" + String(strip->getPalName(i)) + "\"";
    //if (i < strip->getPalCount() - 1)
      json += ",";
  }
  json += "\"Custom\"";

  return json;
}

String getAutoplay() {
  return String(strip->getSegments()[0].autoplay);
}

String getAutoplayDuration() {
  return String(strip->getSegments()[0].autoplayDuration);
}

String getAutopal() {
  return String(strip->getSegments()[0].autoPal);
}

String getAutopalDuration() {
  return String(strip->getSegments()[0].autoPalDuration);
}

String getSolidColor() {
  CRGB solidColor = strip->getTargetPalette().entries[0];
  return String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
}

String getCooling() {
  return String(strip->getCooling());
}

String getSparking() {
  return String(strip->getSparking());
}

String getSpeed() {
  return String(strip->getBeat88());
}

String getTwinkleSpeed() {
  return String(strip->getTwinkleSpeed());
}

String getTwinkleDensity() {
  return String(strip->getTwinkleDensity());
}

String getHueTime() {
  return String(strip->getSegments()[0].hueTime);
}

String getDeltaHue() {
  return String(strip->getSegments()[0].deltaHue);
}

String getBlendType() {
  return String(strip->getSegments()[0].blendType);
}

String getBlendTypes() {
  return "\"NoBlend\",\"LinearBlend\"";
}

String getReverse() {
  return String(strip->getSegments()[0].reverse);
}

String getMilliamps(void) {
  return String(strip->getMilliamps());
}

String getBlurValue(void) {
  return String(strip->getBlurValue());
}

FieldList fields = {
  { "power",            LED_NAME,                           SectionFieldType                                                                        },
  { "power",            "LED Schalter",                     BooleanFieldType,   0,              1,                      getPower                    },
  { "basicControl",     "Basic control",                    SectionFieldType                                                                        },
  { "br",               "Helligkeit",                       NumberFieldType,    BRIGHTNESS_MIN, BRIGHTNESS_MAX,         getBrightness               },
  { "mo",               "Lichteffekt",                      SelectFieldType,    0,              strip->getModeCount(),  getPattern, getPatterns     },
  { "pa",               "Farbpalette",                      SelectFieldType,    0,   (uint16_t)(strip->getPalCount()+1),getPalette, getPalettes     },
  { "sp",               "Geschwindigkeit",                  NumberFieldType,    BEAT88_MIN,     BEAT88_MAX,             getSpeed                    },
  { "blendType",        "Blendmodus",                       SelectFieldType,    NOBLEND,        LINEARBLEND,            getBlendType, getBlendTypes },
  { "LEDblur",          "LED / Effect Blending",            NumberFieldType,    0,              255,                    getBlurValue                },
  { "reverse",          "Rückwärts",                        BooleanFieldType,   0,              1,                      getReverse                  },
  { "hue",              "Farbwechsel",                      SectionFieldType                                                                        },
  { "huetime",          "Hue Wechselintervall",             NumberFieldType,    0,              10000,                  getHueTime                  },
  { "deltahue",         "Hue Offset",                       NumberFieldType,    0,              255,                    getDeltaHue                 },
  { "autoplay",         "Mode Autoplay",                    SectionFieldType                                                                        },
  { "autoplay",         "Mode Automatisch wechseln",        BooleanFieldType,   0,              1,                      getAutoplay                 },
  { "autoplayDuration", "Mode Wechselzeit",                 NumberFieldType,    5,              1000,                   getAutoplayDuration         },
  { "autopal",          "Farbpalette Autoplay",             SectionFieldType                                                                        },
  { "autopal",          "Farbpalette Automatisch wechseln", BooleanFieldType,   0,              1,                      getAutopal                  },
  { "autopalDuration",  "Farbpalette Wechselzeit",          NumberFieldType,    5,              1000,                   getAutopalDuration          },
  { "solidColor",       "Feste Farbe",                      SectionFieldType                                                                        },
  { "solidColor",       "Farbe",                            ColorFieldType,     0,              255,                    getSolidColor               },
  { "fire",             "Feuer und Wasser",                 SectionFieldType                                                                        },
  { "cooling",          "Kühlung",                          NumberFieldType,    0,              255,                    getCooling                  },
  { "sparking",         "Funken",                           NumberFieldType,    0,              255,                    getSparking                 },
  { "twinkles",         "Funkeln",                          SectionFieldType                                                                        },
  { "twinkleSpeed",     "Funkelgeschwindigkeit",            NumberFieldType,    0,              8,                      getTwinkleSpeed             },
  { "twinkleDensity",   "Wieviel Funkellichter",            NumberFieldType,    0,              8,                      getTwinkleDensity           },
  { "Settings",         "Einstellungen",                    SectionFieldType                                                                        },
  { "current",          "max Strom",                        NumberFieldType,    100,            20000,                  getMilliamps                },
};

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

uint8_t fieldCount = ARRAY_SIZE(fields);

// set all pixels to 'off'
void stripe_setup(  const uint16_t LEDCount, 
                    const uint8_t FPS = 60, 
                    const uint8_t volt = 5, 
                    const uint16_t milliamps = 500, 
                    const CRGBPalette16 pal = Rainbow_gp, 
                    const String Name = "Rainbow Colors",
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
    //stripWasOff = false;
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
  strip->show();
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
    strip->start();
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
        EVERY_N_MILLISECONDS(20);
        {
          mySunriseTrigger();
        }
      }
      break;
    case FX_WS2812 :
      strip->service();
      break;
    default:
      break;
      //reset();
  }
}

// Sets a new Effect to be called
void setEffect(uint8_t Effect){
  //if(Effect != FX_WS2812) reset(); // Only reset (with fade) for non-WS2812FX as we have the fading build-in
  //previousEffect = currentEffect;
  currentEffect = Effect;
  strip_On_Off(true);
  stripWasOff = false;
  if(strip->getBrightness()<1)
  {
    strip->setBrightness(10);
  }
  if(Effect == FX_WS2812) {
    strip->start();
    //strip->trigger();
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
  strip->setBrightness(BRIGHTNESS_MAX);
  if(up)
  {
    sunriseParam.step = 0;
    sunriseParam.isSunrise = true;
    fill_solid(strip->leds, strip->getLength(), CRGB::Black);
  }
  else
  {
    sunriseParam.step = steps;
    sunriseParam.isSunrise = false;
    fill_solid(strip->leds, strip->getLength(), HeatColor(255)); //ColorFromPalette(HeatColors_p, 240, BRIGHTNESS_MAX, LINEARBLEND));
  }
  sunriseParam.deltaTime = (mytime/steps);
  sunriseParam.lastChange = millis() - sunriseParam.deltaTime;

  shouldSaveRuntime = true;
  
  setEffect(FX_SUNRISE);
  
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
    sunriseParam.lastChange = (uint32_t)millis();
    
    if((sunriseParam.step >= sunriseParam.steps))
    {
      sunriseParam.isRunning = false;
      strip->setColor(CRGBPalette16(HeatColor(255)));
      strip->setMode(0);
      setEffect(FX_WS2812);
      strip_On_Off(true);
      return;
    }
    else if (sunriseParam.step == 0)
    {
      sunriseParam.isRunning = false;
      strip->setTargetPalette(0);
      strip->setMode(FX_MODE_TWINKLE_FOX);
      setEffect(FX_WS2812);
      //reset();
      strip_On_Off(false);
      return;
    }
  }
  uint8_t step = (uint8_t)map(sunriseParam.step, 0, sunriseParam.steps, 0, 255);

  fill_solid(strip->leds, strip->getLength(), HeatColor(step));
  
  uint8_t br = step<64?(uint8_t)map(step, 0, 64, BRIGHTNESS_MIN+1, BRIGHTNESS_MAX):BRIGHTNESS_MAX;
  nscale8_video(strip->leds, strip->getLength(), br);

  if(step>20)
  {
    CRGB nc = 0x0;
    for(uint16_t i=0; i<strip->getLength(); i++)
    {
      nc = strip->leds[i];
      if(step < 192)
      {
        nc.nscale8_video(random8(step/2));
      }
      else
      {
        nc.nscale8_video(random8(270-step));
      }
      strip->leds[i] = nblend(strip->leds[i], nc, 192);
    }
  }
  nblend(strip->_bleds, strip->leds, strip->getLength(), 48);
  //strip->show();
  FastLED.show();
  //FastLED.delay(1);
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
    strip->show();
  } while (!isBlack);
  //strip->stop();
}
