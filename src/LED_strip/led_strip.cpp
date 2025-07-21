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

WS2812FX *strip;


/*
*
* get Values
* 
*/
inline uint32_t getPower() {
  return (uint32_t)(strip->getPower());
}
inline uint32_t getIsRunning(void) {
  return (uint32_t)(strip->isRunning());
}
inline uint32_t getBrightness() {
  return (uint32_t)(strip->getBrightness());
}
inline uint32_t getPattern() {
  return (uint32_t)(strip->getMode());
}
inline uint32_t getPalette() {
  return (uint32_t)(strip->getTargetPaletteNumber());
}
inline uint32_t getPaletteDistribution() {
  return (uint32_t)(strip->getPaletteDistribution());
}
inline uint32_t getSpeed() {
  return (uint32_t)(strip->getBeat88());
}
inline uint32_t getBlendType() {
  return (uint32_t)(strip->getBlendType());
}
inline uint32_t getColorTemp() {
  return (uint32_t)(strip->getColorTemp());
}
inline uint32_t getBlurValue(void) {
  return (uint32_t)(strip->getBlurValue());
}
inline uint32_t getReverse() {
  return (uint32_t)(strip->getReverse());
}
inline uint32_t getSegments(void) {
  return (uint32_t)(strip->getSegments());
}
inline uint32_t getMirror() {
  return (uint32_t)(strip->getMirror());
}
inline uint32_t getAddGlitter(void) {
  return (uint32_t)(strip->getAddGlitter());
}
inline uint32_t getWhiteOnly(void) {
  return (uint32_t)(strip->getWhiteGlitter());
}
inline uint32_t getOnBlackOnly(void) {
  return (uint32_t)(strip->getOnBlackOnly());
}
inline uint32_t getSynched(void) {
  return (uint32_t)(strip->getSynchronous());
}
inline uint32_t getHueTime() {
  return (uint32_t)(strip->getHueTime());
}
inline uint32_t getDeltaHue()
{
  return (uint32_t)(strip->getDeltaHue());
}
inline uint32_t getAutoplay() {
  return (uint32_t)(strip->getAutoplay());
}
inline uint32_t getAutoplayDuration() {
  return (uint32_t)(strip->getAutoplayDuration());
}
inline uint32_t getAutopal() {
  return (uint32_t)(strip->getAutopal());
}
inline uint32_t getAutopalDuration(){
  return (uint32_t)(strip->getAutopalDuration());
}
inline uint32_t getSolidColor() {
  CRGB solidColor = strip->getSolidColor();
  return ((solidColor.r << 16 | solidColor.g << 8 | solidColor.b << 0) & 0x00ffffff);
}
inline uint32_t getCooling() {
  return (uint32_t)(strip->getCooling());
}
inline uint32_t getSparking() {
  return (uint32_t)(strip->getSparking());
}
inline uint32_t getTwinkleSpeed() {
  return (uint32_t)(strip->getTwinkleSpeed());
}
inline uint32_t getTwinkleDensity() {
  return (uint32_t)(strip->getTwinkleDensity());
}
inline uint32_t getNumBars() {
  return (uint32_t)(strip->getNumBars());
}
inline uint32_t getDamping() {
  return (uint32_t)(strip->getDamping());
}
inline uint32_t getSunRiseTime(void) {
  return (uint32_t)(strip->getSunriseTime());
}
inline uint32_t getMilliamps(void) {
  return (uint32_t)(strip->getMilliamps());
}
inline uint32_t getFPSValue(void) {
  return (uint32_t)(strip->getMaxFPS());
}
inline uint32_t getDithering(void) {
  return (uint32_t)(strip->getDithering());
}
inline uint32_t getResetDefaults(void) {
  return (uint32_t)(0);
}
inline uint32_t getBckndHue() {
  return (uint32_t)(strip->getBckndHue());
}
inline uint32_t getBckndSat() {
  return (uint32_t)(strip->getBckndSat());
}
inline uint32_t getBckndBri() {
  return (uint32_t)(strip->getBckndBri());
}
inline uint32_t getColCor() {
  return (uint32_t)(strip->getColorCorrectionEnum());
}


#ifdef HAS_KNOB_CONTROL
inline uint32_t getWiFiDisabled(void) {
  return (uint32_t)(strip->getWiFiDisabled());
}
#endif

/*
* Options
* 
*/
void getPatterns(JsonArray jArr) {
  const uint8_t count = strip->getModeCount();
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getModeName(i));
  }
}
void getPalettes(JsonArray jArr) {
  const uint8_t count = strip->getPalCount();
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getPalName(i));
  }

  jArr.add(F("Custom"));
}

void getAutoplayModes(JsonArray jArr) {
  jArr.add(F("Off"));
  jArr.add(F("Up"));
  jArr.add(F("Down"));
  jArr.add(F("Random"));
}
void getBlendTypes(JsonArray jArr)
{
  jArr.add(F("NoBlend"));
  jArr.add(F("LinearBlend"));
}
void getColorTemps(JsonArray jArr)
{
  const uint8_t count = 10;
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getColorTempName(i));
  }
}
void getColCorValues(JsonArray jArr) {
  jArr.add(F("TypicalLEDStrip"));
  jArr.add(F("TypicalPixelString"));
  jArr.add(F("UncorrectedColor"));
}

/*
* Single option getters - memory efficient versions
* 
*/
String getPatternAtIndex(uint32_t index) {
  if (index < strip->getModeCount()) {
    return String(strip->getModeName(index));
  }
  return String("");
}

String getPaletteAtIndex(uint32_t index) {
  const uint8_t count = strip->getPalCount();
  if (index < count) {
    return String(strip->getPalName(index));
  } else if (index == count) {
    return String("Custom");
  }
  return String("");
}

String getAutoplayModeAtIndex(uint32_t index) {
  switch(index) {
    case 0: return String("Off");
    case 1: return String("Up");
    case 2: return String("Down");
    case 3: return String("Random");
    default: return String("");
  }
}

String getBlendTypeAtIndex(uint32_t index) {
  switch(index) {
    case 0: return String("NoBlend");
    case 1: return String("LinearBlend");
    default: return String("");
  }
}

String getColorTempAtIndex(uint32_t index) {
  if (index < 10) {
    return String(strip->getColorTempName(index));
  }
  return String("");
}

String getColCorValueAtIndex(uint32_t index) {
  switch(index) {
    case 0: return String("TypicalLEDStrip");
    case 1: return String("TypicalPixelString");
    case 2: return String("UncorrectedColor");
    default: return String("");
  }
}

/*
* Setters
* 
*/

void setPower(uint32_t val) {
  strip->setPower(val);
  if(val)
    strip->setIsRunning(val);
}
void setIsRunning(uint32_t val) {
  strip->setIsRunning(val); 
}
void setBrightness(uint32_t val) {
  strip->setBrightness(val);
}
void setPattern(uint32_t val) {
  strip->setMode(val);
}
void setPalette(uint32_t val) {
  strip->setTargetPalette(val);
}
void setPaletteDistribution(uint32_t val) {
  strip->setPaletteDistribution(val);
}
void setSpeed(uint32_t val) {
  strip->setBeat88(val);
}
void setBlendType(uint32_t val) {
  strip->setBlendType((TBlendType)val);
}
void setColorTemp(uint32_t val) {
  strip->setColorTemp(val);
}
void setBlurValue(uint32_t val) {
  strip->setBlur(val);
}
void setReverse(uint32_t val) {
  strip->setReverse(val);
}
void setSegments(uint32_t val) {
  strip->setSegments(val);
}
void setMirror(uint32_t val) {
  strip->setMirror(val);
}
void setAddGlitter(uint32_t val) {
  strip->setAddGlitter(val);
}
void setWhiteOnly(uint32_t val) {
  strip->setWhiteGlitter(val);
}
void setOnBlackOnly(uint32_t val) {
  strip->setOnBlackOnly(val);
}
void setSynched(uint32_t val) {
  strip->setSynchronous(val);
}
void setHueTime(uint32_t val) {
  strip->setHuetime(val);
}
void setDeltaHue(uint32_t val) {
  strip->setDeltaHue(val);
}
void setAutoplayMode(uint32_t val) {
  strip->setAutoplay((AUTOPLAYMODES)val);
}
void setAutoplayDuration(uint32_t val) {
  strip->setAutoplayDuration(val);
}
void setAutopal(uint32_t val) {
  strip->setAutopal((AUTOPLAYMODES)val);
}
void setAutopalDuration(uint32_t val) {
  strip->setAutopalDuration(val);
}
void setSolidColor(uint32_t val) {
  strip->setSolidColor(val);
}
void setCooling(uint32_t val) {
  strip->setCooling(val);
}
void setSparking(uint32_t val) {
  strip->setSparking(val);
}
void setTwinkleSpeed(uint32_t val) {
  strip->setTwinkleSpeed(val);
}
void setTwinkleDensity(uint32_t val) {
  strip->setTwinkleDensity(val);
}
void setNumBars(uint32_t val) {
  strip->setNumBars(val);
}
void setDamping(uint32_t val) {
  strip->setDamping(val);
}
void setSunRiseTime(uint32_t val) {
  strip->setSunriseTime(val);
}
void setMilliamps(uint32_t val) {
  strip->setMilliamps(val);
}
void setFPSValue(uint32_t val) {
  strip->setMaxFPS(val);
}
void setDithering(uint32_t val) {
  strip->setDithering(val);
}
void setResetDefaults(uint32_t val) {
  if(val)
  {
    strip->resetDefaults();
  }
}
void setBckndHue(uint32_t val) {
  strip->setBckndHue(val);
}
void setBckndSat(uint32_t val) {
  strip->setBckndSat(val);
}
void setBckndBri(uint32_t val) {
  strip->setBckndBri(val);
}
void setColCor(uint32_t val) {
  strip->setColCor((COLORCORRECTIONS)val);
}

#ifdef HAS_KNOB_CONTROL
void setWiFiDisabled(uint32_t val) {
  if(val)
    strip->setWiFiDisabled(true);
  else
    strip->setWiFiDisabled(false);
}
#endif

#ifdef DEBUG
uint32_t getReset() {
  return 0; // Return current reset type index
}

void getResets(JsonArray jArr) {
  jArr.add(F("No Reset"));
  jArr.add(F("Reset Function"));
  jArr.add(F("Restart Function"));
  jArr.add(F("HW Watchdog"));
  jArr.add(F("SW Watchdog"));
  jArr.add(F("Exception"));
}

String getResetAtIndex(uint32_t index) {
  switch(index) {
    case 0: return String("No Reset");
    case 1: return String("Reset Function");
    case 2: return String("Restart Function");
    case 3: return String("HW Watchdog");
    case 4: return String("SW Watchdog");
    case 5: return String("Exception");
    default: return String("");
  }
}
#endif

const Field fields [] = {
  {"title",             LED_NAME,                       TitleFieldType,     0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr          },
  {"s_powerSection",    "Power / Effects",              SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr          },
  {"power",             "On/Off",                       BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getPower,           nullptr,           nullptr,        setPower      },
  {"effect",            "Effect",                       SelectFieldType,    (uint16_t)0,                            (uint16_t)strip->getModeCount(),                  getPattern,         getPatterns,       getPatternAtIndex, setPattern    },
  {"brightness",        "Brightness",                   NumberFieldType,    (uint16_t)BRIGHTNESS_MIN,               (uint16_t)BRIGHTNESS_MAX,                         getBrightness,      nullptr,           nullptr,        setBrightness },
  {"speed",             "Speed",                        NumberFieldType,    (uint16_t)BEAT88_MIN,                   (uint16_t)BEAT88_MAX,                             getSpeed,           nullptr,           nullptr,        setSpeed      },
  {"colorPalette",      "Color palette",                SelectFieldType,    (uint16_t)0,                            (uint16_t)(strip->getPalCount() + 1),             getPalette,         getPalettes,       getPaletteAtIndex, setPalette    },
  {"paletteDistribution", "Palette distribution (%)",    NumberFieldType,    (uint16_t)25,                           (uint16_t)400,                                    getPaletteDistribution, nullptr,       nullptr,        setPaletteDistribution },
  {"running",           "Pause",                        BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getIsRunning,       nullptr,           nullptr,        setIsRunning  },
  {"s_stripeStruture",  "Structure",                    SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr          },
  {"segments",          "Segments",                     NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_SEGMENTS, 1),               getSegments,        nullptr,           nullptr,        setSegments   },
  {"numEffectBars",     "# LED bars",                   NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_BARS, 1),                   getNumBars,         nullptr,           nullptr,        setNumBars                    },
  {"reversed",          "Reverse",                      BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getReverse,         nullptr,           nullptr,        setReverse    },
  {"mirrored",          "Mirror",                       BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getMirror,          nullptr,           nullptr,        setMirror     },
  {"s_Autoplay",        "Autoplay",                     SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr          },
  {"autoPlay",          "Auto mode change",             SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutoplay,        getAutoplayModes,  getAutoplayModeAtIndex, setAutoplayMode             },
  {"autoPlayInterval",  "Auto mode interval (s)",       NumberFieldType,    (uint16_t)DEFAULT_T_AUTOMODE_MIN,       (uint16_t)DEFAULT_T_AUTOMODE_MAX,                 getAutoplayDuration, nullptr,          nullptr,        setAutoplayDuration           },
  {"autoPalette",       "Auto palette change",          SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutopal,         getAutoplayModes,  getAutoplayModeAtIndex, setAutopal                  },
  {"autoPalInterval",   "Auto palette interval (s)",    NumberFieldType,    (uint16_t)DEFAULT_T_AUTOCOLOR_MIN,      (uint16_t)DEFAULT_T_AUTOCOLOR_MAX,                                   getAutopalDuration, nullptr,           nullptr,        setAutopalDuration            },
  {"s_BackGroundColor", "Bcknd Color",                  SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"backgroundHue",     "Bcknd Hue",                    NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndHue,        nullptr,           nullptr,        setBckndHue                   },
  {"backgroundSat",     "Bcknd Sat",                    NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndSat,        nullptr,           nullptr,        setBckndSat                   },
  {"backgroundBri",     "Bcknd Bri",                    NumberFieldType,    (uint16_t)BCKND_MIN_BRI,                (uint16_t)BCKND_MAX_BRI,                          getBckndBri,        nullptr,           nullptr,        setBckndBri                   },
  {"s_advancedControl", "Advanced",                     SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"blendType",         "Color blend",                  SelectFieldType,    (uint16_t)NOBLEND,                      (uint16_t)LINEARBLEND,                            getBlendType,       getBlendTypes,     getBlendTypeAtIndex, setBlendType                  },
  {"colorTemperature",  "Color temperature",            SelectFieldType,    (uint16_t)0,                            (uint16_t)20,                                     getColorTemp,       getColorTemps,     getColorTempAtIndex, setColorTemp                  },
  {"ledBlur",           "Effect blur / blending",       NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBlurValue,       nullptr,           nullptr,        setBlurValue                  },
  {"s_solidColor",      "Solid color",                  SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"solidColor",        "Color",                        ColorFieldType,     (uint16_t)0,                            (uint16_t)55,                                     getSolidColor,      nullptr,           nullptr,        setSolidColor                 },
  {"s_glitter",         "Glitter",                      SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"addGlitter",        "Add Glitter",                  BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getAddGlitter,      nullptr,           nullptr,        setAddGlitter                 },
  {"whiteGlitter",      "White Glitter",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWhiteOnly,       nullptr,           nullptr,        setWhiteOnly                  },
  {"onBlackOnly",       "On Black",                     BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getOnBlackOnly,     nullptr,           nullptr,        setOnBlackOnly                },
  {"syncGlitter",       "Sync Segments",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getSynched,         nullptr,           nullptr,        setSynched             },
  {"s_basicHue",        "Hue Change",                   SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"hueTime",           "Hue interval (ms)",            NumberFieldType,    (uint16_t)0,                            (uint16_t)5000,                                   getHueTime,         nullptr,           nullptr,        setHueTime                    },
  {"deltaHue",          "Hue Offset",                   NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getDeltaHue,        nullptr,           nullptr,        setDeltaHue                   },    
  {"s_effectSettings",  "Effect Settings",              SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"cooling",           "Cooling",                      NumberFieldType,    (uint16_t)DEFAULT_COOLING_MIN,          (uint16_t)DEFAULT_COOLING_MAX,                    getCooling,         nullptr,           nullptr,        setCooling},
  {"sparking",          "Sparking",                     NumberFieldType,    (uint16_t)DEFAULT_SPARKING_MIN,         (uint16_t)DEFAULT_SPARKING_MAX,                   getSparking,        nullptr,           nullptr,        setSparking                   },
  {"twinkleSpeed",      "Twinkle speed",                NumberFieldType,    (uint16_t)DEFAULT_TWINKLE_S_MIN,        (uint16_t)DEFAULT_TWINKLE_S_MAX,                  getTwinkleSpeed,    nullptr,           nullptr,        setTwinkleSpeed               },
  {"twinkleDensity",    "Twinkle density",              NumberFieldType,    (uint16_t)DEFAULT_TWINKLE_NUM_MIN,      (uint16_t)DEFAULT_TWINKLE_NUM_MAX,                getTwinkleDensity,  nullptr,           nullptr,        setTwinkleDensity             },
  {"damping",           "damping for bounce",           NumberFieldType,    (uint16_t)DEFAULT_DAMPING_MIN,          (uint16_t)DEFAULT_DAMPING_MAX,                    getDamping,         nullptr,           nullptr,        setDamping                    },
  // time provided in Minutes and capped at 60 minutes actually.
  {"sunriseset",        "sunrise, sunset time in min",  NumberFieldType,    (uint16_t)DEFAULT_SUNRISETIME_MIN,      (uint16_t)DEFAULT_SUNRISETIME_MAX,                getSunRiseTime,     nullptr,           nullptr,        setSunRiseTime                }, 
  {"s_otherSettings",   "Other settings",               SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  #ifdef HAS_KNOB_CONTROL
  {"wifiDisabled",      "WiFi Disabled",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWiFiDisabled,    nullptr,           nullptr,        setWiFiDisabled               },  
  #endif
  {"currentLimit",      "Current limit",                NumberFieldType,    (uint16_t)100,                          (uint16_t)DEFAULT_CURRENT_MAX,                    getMilliamps,       nullptr,           nullptr,        setMilliamps                  },
  {"colorCorrection",   "ColorCorrection",              SelectFieldType,    (uint16_t)0,                            (uint16_t)(COR_NUMCORRECTIONS-1),                 getColCor,          getColCorValues,   getColCorValueAtIndex, setColCor                    },
  // 111 max equals the minimum update time required for 300 pixels
  // this is the minimal delay being used anyway, so no use in being faster
  {"fps",               "max FPS",                      NumberFieldType,    (uint16_t)STRIP_MIN_FPS,                (uint16_t)(STRIP_MAX_FPS),                        getFPSValue,        nullptr,           nullptr,        setFPSValue                   },                                                                           
  {"dithering",         "Dithering",                    BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getDithering,       nullptr,           nullptr,        setDithering                  },
  {"resetdefaults",     "Reset default",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getResetDefaults,   nullptr,           nullptr,        setResetDefaults              },
#ifdef DEBUG
  // With the DEBUG flag enabled we can provoke some resets (SOFT WDT, HW WDT, Exception...)
  {"S_Debug",           "DEBUG only - not for production",        SectionFieldType,   0,                                   0,                                             nullptr,               nullptr,           nullptr,        nullptr                          },
  {"resets",            "Resets (DEV Debug)",                     SelectFieldType,    (uint16_t)0,                            (uint16_t)5,                                      getReset,         getResets,         getResetAtIndex,        nullptr   },
#endif
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

uint8_t fieldCount = ARRAY_SIZE(fields);

bool getAllValuesJSONArray(JsonArray arr)
{
  bool ret = false; 
  for(uint8_t i=0; i<getFieldCount(); i++)
  {
    if(fields[i].type < TitleFieldType)
    {
      JsonObject obj = arr.createNestedObject();
      obj[F("name")] =  fields[i].name;
      obj[F("value")] = fields[i].getValue();          
      ret = true;
    }
  }
  return ret;
}


void getAllJSON(JsonArray arr)
{
  for (uint8_t i = 0; i < getFieldCount(); i++)
  {
    Field field = fields[i];
    JsonObject obj = arr.createNestedObject();
    obj[F("name")]  = field.name;
    obj[F("label")] = field.label;
    obj[F("type")]  = (int)field.type;
    
    if (field.type == NumberFieldType) 
    {
      obj[F("min")] = field.min;
      obj[F("max")] = field.max;
    }

    if (field.getOptions)
    {
      JsonArray ar = obj.createNestedArray(F("options"));
      field.getOptions(ar);
    }
  }
}


bool getAllValuesJSON(JsonObject obj)
{
  bool ret = false;
  for(uint8_t i=0; i<getFieldCount(); i++)
  {
    if(fields[i].getValue)
    {
      ret = true;
      obj[fields[i].name] = getFieldValue(fields[i].name);
    }
  }
  return ret;
}

bool isField(const char * name)
{
  for (uint8_t i = 0; i < fieldCount; i++)
  {
    if (strcmp(fields[i].name, name) == 0)
    {
      return true;
    }
  }
  return false;
}

Field getField(const char * name)
{
  for (uint8_t i = 0; i < fieldCount; i++)
  {
    Field field = fields[i];
    if (strcmp(field.name, name) == 0)
    {
      return field;
    }
  }
  return Field();
}

String getFieldValue(const char * name)
{
  Field field = getField(name);
  if (field.getValue)
  {
    switch (field.type)
    {
    case NumberFieldType:
    case ColorFieldType:
      return String(field.getValue());
      break;
    case BooleanFieldType:
      if(field.getValue())
      {
        return "on";
      }
      return "off";
      break;
    case SelectFieldType:
      // Use memory-efficient single option getter if available
      if (field.getOptionAtIndex)
      {
        return field.getOptionAtIndex(field.getValue());
      }
      else if (field.getOptions)
      {
        // Fallback to original method if single option getter not available
        // Buffer size: Single field value as uint32_t + JSON overhead = 1024 bytes - standard to be safe
        DynamicJsonDocument doc(1024);
        JsonArray a = doc.to<JsonArray>();
        field.getOptions(a);
        return a[field.getValue()].as<const char*>(); 
      }
      break;
    default:
      break;
    }
    return String(field.getValue());
  }
  return String();
}

void setFieldValue(const char * name, uint32_t value)
{
  Field field = getField(name);
  if (field.setValue)
  {
    field.setValue(value);
  }
}

const Field * getFields(void)
{
  return (const Field*)fields;
}


uint8_t getFieldCount(void)
{
  return fieldCount;
}

  

void stripe_setup(CRGB * pleds, CRGB* eleds)
{
  strip = new WS2812FX(pleds, eleds);
  strip->init();
  strip->start();
  strip->show();
}