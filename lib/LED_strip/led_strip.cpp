/**************************************************************
   This work is based on many others.
   If published then under whatever licence free to be used,
   distibuted, changed and whatsoever.
   This Work is based on many others 
   and heavily modified for my personal use.
   It is basically based on the following great developments:
   - Adafruit Neopixel https://github.com/adafruit/Adafruit_NeoPixel
   - WS2812FX library https://github.com/kitesurfer1404/WS2812FX
   - fhem esp8266 implementation - Idea from https://github.com/sw-home/FHEM-LEDStripe 
   - FastLED library - see http://www.fastLed.io
   - ESPWebserver - see https://github.com/jasoncoon/esp8266-fastled-webserver
  
  My GIT source code storage
  https://github.com/tobi01001/LED_Stripe_Dynamic_web_conf

  Done by tobi01001

  **************************************************************

  MIT License

  Copyright (c) 2018 tobi01001

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.


 **************************************************************/

#ifndef led_strip_h
  #include "led_strip.h"
#endif 

bool stripWasOff = true;
bool stripIsOn = true;
extern bool shouldSaveRuntime;


uint8_t currentEffect = FX_NO_FX;
uint8_t previousEffect = FX_NO_FX;

mysunriseParam sunriseParam;

WS2812FX *strip;  

const String TitleFieldType   = "Title";
const String NumberFieldType  = "Number";
const String BooleanFieldType = "Boolean";
const String SelectFieldType  = "Select";
const String ColorFieldType   = "Color";
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
  return String(strip->getSegment()->autoplay);
}

String getInverse() {
  return String(strip->getInverse());
}

String getMirror() {
  return String(strip->getMirror());
}

String getAutoplayDuration() {
  return String(strip->getSegment()->autoplayDuration);
}

String getAutopal() {
  return String(strip->getSegment()->autoPal);
}

String getAutopalDuration() {
  return String(strip->getSegment()->autoPalDuration);
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

String getNumBars() {
  return String(strip->getNumBars());
}

String getHueTime() {
  return String(strip->getSegment()->hueTime);
}

String getDeltaHue() {
  return String(strip->getSegment()->deltaHue);
}

String getBlendType() {
  return String(strip->getSegment()->blendType);
}

String getBlendTypes() {
  return "\"NoBlend\",\"LinearBlend\"";
}

String getColorTemp() {
  return String(strip->getColorTemp());
}

String getColorTemps() {
  String json = "";

  for (uint8_t i = 0; i < 19; i++) {
    json += "\"" + String(strip->getColorTempName(i)) + "\"";
    json += ",";
  }
  json += "\"" + String(strip->getColorTempName(19)) + "\"";

  return json;
  //return "\"Candle\",\"Kelvin\",\"Tungsten40W\",\"Tungsten100W\",\"Halogen\",\"CarbonArc\",\"HighNoonSun\",\"DirectSunlight\",\"OvercastSky\",\"ClearBlueSky\",\"WarmFluorescent\",\"StandardFluorescent\",\"CoolWhiteFluorescent\",\"FullSpectrumFluorescent\",\"GrowLightFluorescent\",\"BlackLightFluorescent\",\"MercuryVapor\",\"SodiumVapor\",\"MetalHalide\",\"HighPressureSodium\",\"UncorrectedTemperature\"";
}

String getReverse() {
  return String(strip->getSegment()->reverse);
}

String getMilliamps(void) {
  return String(strip->getMilliamps());
}

String getBlurValue(void) {
  return String(strip->getBlurValue());
}

String getFPSValue(void) {
  return String(strip->getMaxFPS());
}

String getSegments(void) {
  return String(strip->getSegment()->segments);
}

FieldList fields = {
  { "title",            LED_NAME,                           TitleFieldType                                                                          },
  { "power",            LED_NAME,                           SectionFieldType                                                                        },
  { "power",            "LED Schalter",                     BooleanFieldType,   0,              1,                      getPower                    },
  { "basicControl",     "Basic control",                    SectionFieldType                                                                        },
  { "br",               "Helligkeit",                       NumberFieldType,    BRIGHTNESS_MIN, BRIGHTNESS_MAX,         getBrightness               },
  { "mo",               "Lichteffekt",                      SelectFieldType,    0,              strip->getModeCount(),  getPattern, getPatterns     },
  { "pa",               "Farbpalette",                      SelectFieldType,    0,   (uint16_t)(strip->getPalCount()+1),getPalette, getPalettes     },
  { "sp",               "Geschwindigkeit",                  NumberFieldType,    BEAT88_MIN,     BEAT88_MAX,             getSpeed                    },
  { "blendType",        "Blendmodus",                       SelectFieldType,    NOBLEND,        LINEARBLEND,            getBlendType, getBlendTypes },
  { "ColorTemperature", "Farbtemperatur",                   SelectFieldType,    0,              20,                     getColorTemp, getColorTemps },
  { "LEDblur",          "LED / Effect Blending",            NumberFieldType,    0,              255,                    getBlurValue                },
  { "reverse",          "Rückwärts",                        BooleanFieldType,   0,              1,                      getReverse                  },
  { "segments",         "Segmente",                         NumberFieldType,    1,              max(LED_COUNT/50,1),    getSegments                 },
  { "mirror",           "gespiegelt",                       BooleanFieldType,   0,              1,                      getMirror                   },
  { "inverse",          "Invertiert",                       BooleanFieldType,   0,              1,                      getInverse                  },
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
  { "ledBars",          "LED Balken für Effekte",           SectionFieldType,                                                                       },
  { "numBars",          "Anzahl LED Balken",                NumberFieldType,    1,              max(LED_COUNT/20,1),    getNumBars                  },
  { "Settings",         "Einstellungen",                    SectionFieldType                                                                        },
  { "current",          "max Strom",                        NumberFieldType,    100,            10000,                  getMilliamps                },
  { "fps",              "Wiederholrate (FPS)",              NumberFieldType,    5,              255,                    getFPSValue                 },
};

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

uint8_t fieldCount = ARRAY_SIZE(fields);

// set all pixels to 'off'
void stripe_setup(  const uint16_t LEDCount, 
                    const uint8_t FPS = (uint8_t) 60, 
                    const uint8_t volt = (uint8_t) 5, 
                    const uint16_t milliamps = (uint16_t) 500, 
                    const CRGBPalette16 pal = Rainbow_gp, 
                    const String Name = "Rainbow Colors",
                    const LEDColorCorrection colc = UncorrectedColor /*TypicalLEDStrip*/ ){
  strip = new WS2812FX(LEDCount, FPS, volt, milliamps, pal, Name, colc);
  strip->setColorTemperature(19);
  strip->init();
  strip->setBrightness(DEFAULT_BRIGHTNESS);
  strip->setSpeed(DEFAULT_BEAT88);

  strip->start();
  strip->show();
}

void strip_On_Off(bool onOff){
    stripIsOn = onOff;
    //stripWasOff = false;
}

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b) {
  if(start >= strip->getStripLength() || stop >= strip->getStripLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip->leds[i] = CRGB(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
  }
  //strip->setColor(r, g, b);
  strip->show();
}

void set_Range(uint16_t start, uint16_t stop, uint32_t color) {
  if(start >= strip->getStripLength() || stop >= strip->getStripLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  for(uint16_t i=start; i<=stop; i++) {
    strip->leds[i] = CRGB(color);
  }
  //strip->setColor(color);
  strip->show();
}

void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  if(pixel >= strip->getStripLength()) return;
  strip_On_Off(true);
  setEffect(FX_NO_FX);
  //strip->setPixelColor(pixel, r, g, b);
  strip->leds[pixel] = CRGB(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
  //strip->setColor(r, g, b);
  strip->show();
}

void strip_setpixelcolor(uint16_t pixel, uint32_t color) {
  if(pixel >= strip->getStripLength()) return;
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
    fill_solid(strip->leds, strip->getStripLength(), CRGB::Black);
  }
  else
  {
    sunriseParam.step = steps;
    sunriseParam.isSunrise = false;
    fill_solid(strip->leds, strip->getStripLength(), HeatColor(255)); //ColorFromPalette(HeatColors_p, 240, BRIGHTNESS_MAX, LINEARBLEND));
  }
  strip->setColor(CRGBPalette16(HeatColor(255)));
  strip->setCurrentPalette(CRGBPalette16(HeatColor(255)), "Custom");
  sunriseParam.deltaTime = (mytime/steps);
  sunriseParam.lastChange = millis() - sunriseParam.deltaTime;

  shouldSaveRuntime = true;
  
  setEffect(FX_SUNRISE);
  
  #ifdef DEBUG
  Serial.printf("\nStarted Sunrise/Sunset with %.3u steps in %u ms which are %u minutes.\n", steps, mytime, (mytime/60000));
  #endif

}

void mySunriseTrigger(void) {
  if(!sunriseParam.isRunning) return;
  
  uint32_t now = (uint32_t)millis();
  static uint8_t step = 0;
  if(now > (uint32_t)(sunriseParam.lastChange + sunriseParam.deltaTime))
  {  
    
    #ifdef DEBUG
    if(sunriseParam.isSunrise)
      Serial.printf("\nSunrise running at step %u of %.3u steps in %u s of %u seconds.\n", 
                  sunriseParam.step, sunriseParam.steps, (sunriseParam.step*sunriseParam.deltaTime)/1000, (sunriseParam.deltaTime*sunriseParam.steps)/1000);
    else
      Serial.printf("\nSunset running at step %u of %.3u steps in %u s of %u seconds.\n", 
                  sunriseParam.step, sunriseParam.steps, (sunriseParam.step*sunriseParam.deltaTime)/1000, (sunriseParam.deltaTime*sunriseParam.steps)/1000);
    #endif
    if(sunriseParam.isSunrise)
    {
      sunriseParam.step++;
    }
    else
    {
      sunriseParam.step--;
    }
    step = (uint8_t)map(sunriseParam.step, 0, sunriseParam.steps, 20, 255);
    if((sunriseParam.step >= sunriseParam.steps))
    {
      sunriseParam.isRunning = false;
      
      CRGB co = CRGB::Black;

      for(uint8_t i = 0; i<strip->getStripLength() / 4; i++)
      {
        if(strip->leds[i]>co)
        {
          co = strip->leds[i];
        }
      }
      strip->setColor(CRGBPalette16(co));
      strip->setCurrentPalette(CRGBPalette16(co), "Custom");
      strip->setMode(FX_MODE_STATIC);
      setEffect(FX_WS2812);
      strip_On_Off(true);
      strip->setTransition();
      shouldSaveRuntime = true;
      #ifdef DEBUG
      Serial.println("\nSunrise finished!");
      #endif
      //return;
    }
    else if (sunriseParam.step == 0)
    {
      sunriseParam.isRunning = false;
      
      strip->setTargetPalette(0);
      strip->setMode(FX_MODE_TWINKLE_FOX);
      setEffect(FX_WS2812);
      //reset();
      strip_On_Off(false);
      strip->setTransition();
      shouldSaveRuntime = true;
      #ifdef DEBUG
      Serial.println("\nSunset finished!");
      #endif
      //return;
    }
    
    sunriseParam.lastChange = (uint32_t)millis();
  }

  fill_solid(strip->leds, strip->getStripLength(), HeatColor(step));
  
  uint8_t br = (step<96)?(uint8_t)map(step, 0, 96, 15, BRIGHTNESS_MAX):BRIGHTNESS_MAX;
  nscale8_video(strip->leds, strip->getStripLength(), br);

  CRGB nc = 0x0;
  for(uint16_t i=0; i<strip->getStripLength(); i++)
  {
    nc = strip->leds[i];
    nc.nscale8_video(random8(step<172?(step/2):(255-step)));
    strip->leds[i] = nblend(strip->leds[i], nc, 222);
  }
  
  nblend(strip->_bleds, strip->leds, strip->getStripLength(), 48);
  
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
  for(uint8_t i = 0; i<strip->getStripLength(); i++) {
    if(strip->leds[i].r >max ) max = strip->leds[i].r;
    if(strip->leds[i].g >max ) max = strip->leds[i].g;
    if(strip->leds[i].b >max ) max = strip->leds[i].b;
  }
  uint8_t r,g,b;

  // ToDo: Fade To Black!

  for(uint8_t i = 0; i<max; i++) {
    for(uint16_t p=0; p<strip->getStripLength(); p++)
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
    fadeToBlackBy(strip->leds, strip->getStripLength(), 16);
    for(uint16_t i = 0; i < strip->getStripLength(); i++)
    {
      if(strip->leds[i]) isBlack = false;
    }
    strip->show();
  } while (!isBlack);
  //strip->stop();
}