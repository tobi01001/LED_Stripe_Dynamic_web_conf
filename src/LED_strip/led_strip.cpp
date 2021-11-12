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

bool isField(const char * name, FieldList fields, uint8_t count)
{
  for (uint8_t i = 0; i < count; i++)
  {
    if (strcmp(fields[i].name, name) == 0)
    {
      return true;
    }
  }
  return false;
}

Field getField(const char * name, FieldList fields, uint8_t count)
{
  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (strcmp(field.name, name) == 0)
    {
      return field;
    }
  }
  return Field();
}

String getFieldValue(const char * name, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.getValue)
  {
    DynamicJsonBuffer buffer;
    JsonArray& a = buffer.createArray();
    switch (field.type)
    {
    case NumberFieldType:
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
      field.getOptions(a);
      return a[field.getValue()].as<const char*>(); //.asString();
      break;
    default:
      break;
    }
    return String(field.getValue());
  }
  return String();
}

void setFieldValue(const char * name, uint16_t value, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.setValue)
  {
    field.setValue(value);
  }
}


/*
 *
 * get Values
 * 
 */
inline uint16_t getPower() {
  return (uint16_t)(strip->getPower());
}
inline uint16_t getIsRunning(void) {
  return (uint16_t)(strip->isRunning());
}
inline uint16_t getBrightness() {
  return (uint16_t)(strip->getBrightness());
}
inline uint16_t getPattern() {
  return (uint16_t)(strip->getMode());
}
inline uint16_t getPalette() {
  return (uint16_t)(strip->getTargetPaletteNumber());
}
inline uint16_t getSpeed() {
  return (uint16_t)(strip->getBeat88());
}
inline uint16_t getBlendType() {
  return (uint16_t)(strip->getBlendType());
}
inline uint16_t getColorTemp() {
  return (uint16_t)(strip->getColorTemp());
}
inline uint16_t getBlurValue(void) {
  return (uint16_t)(strip->getBlurValue());
}
inline uint16_t getReverse() {
  return (uint16_t)(strip->getReverse());
}
inline uint16_t getSegments(void) {
  return (uint16_t)(strip->getSegments());
}
inline uint16_t getMirror() {
  return (uint16_t)(strip->getMirror());
}
inline uint16_t getInverse() {
  return (uint16_t)(strip->getInverse());
}
inline uint16_t getAddGlitter(void) {
  return (uint16_t)(strip->getAddGlitter());
}
inline uint16_t getWhiteOnly(void) {
  return (uint16_t)(strip->getWhiteGlitter());
}
inline uint16_t getOnBlackOnly(void) {
  return (uint16_t)(strip->getOnBlackOnly());
}
inline uint16_t getSynched(void) {
  return (uint16_t)(strip->getSynchronous());
}
inline uint16_t getHueTime() {
  return (uint16_t)(strip->getHueTime());
}
inline uint16_t getDeltaHue()
{
  return (uint16_t)(strip->getDeltaHue());
}
inline uint16_t getAutoplay() {
  return (uint16_t)(strip->getAutoplay());
}
inline uint16_t getAutoplayDuration() {
  return (uint16_t)(strip->getAutoplayDuration());
}
inline uint16_t getAutopal() {
  return (uint16_t)(strip->getAutopal());
}
inline uint16_t getAutopalDuration(){
  return (uint16_t)(strip->getAutopalDuration());
}
inline uint16_t getSolidColor() {
  //CRGB solidColor = (*strip->getTargetPalette()).entries[0];
  //return (uint16_t)(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
  return 250; // hmmm what to do with this solid color field? limit t one byte? Change all to uint32 to enable color?
}
inline uint16_t getCooling() {
  return (uint16_t)(strip->getCooling());
}
inline uint16_t getSparking() {
  return (uint16_t)(strip->getSparking());
}
inline uint16_t getTwinkleSpeed() {
  return (uint16_t)(strip->getTwinkleSpeed());
}
inline uint16_t getTwinkleDensity() {
  return (uint16_t)(strip->getTwinkleDensity());
}
inline uint16_t getNumBars() {
  return (uint16_t)(strip->getNumBars());
}
inline uint16_t getDamping() {
  return (uint16_t)(strip->getDamping());
}
inline uint16_t getSunRiseTime(void) {
  return (uint16_t)(strip->getSunriseTime());
}
inline uint16_t getMilliamps(void) {
  return (uint16_t)(strip->getMilliamps());
}
inline uint16_t getFPSValue(void) {
  return (uint16_t)(strip->getMaxFPS());
}
inline uint16_t getDithering(void) {
  return (uint16_t)(strip->getDithering());
}
inline uint16_t getResetDefaults(void) {
  return (uint16_t)(0);
}
inline uint16_t getBckndHue() {
  return (uint16_t)(strip->getBckndHue());
}
inline uint16_t getBckndSat() {
  return (uint16_t)(strip->getBckndSat());
}
inline uint16_t getBckndBri() {
  return (uint16_t)(strip->getBckndBri());
}



#ifdef HAS_KNOB_CONTROL
inline uint16_t getWiFiDisabled(void) {
  return (uint16_t)(strip->getWiFiDisabled());
}
#endif

/*
 * Options
 * 
 */
void getPatterns(JsonArray &jArr) {
  const uint8_t count = strip->getModeCount();
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getModeName(i));
  }
}
void getPalettes(JsonArray &jArr) {
  const uint8_t count = strip->getPalCount();
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getPalName(i));
  }

  jArr.add("Custom");
}

void getAutoplayModes(JsonArray &jArr) {
  jArr.add("Off");
  jArr.add("Up");
  jArr.add("Down");
  jArr.add("Random");
}
void getBlendTypes(JsonArray &jArr)
{
  jArr.add("NoBlend");
  jArr.add("LinearBlend");
}
void getColorTemps(JsonArray &jArr)
{
  const uint8_t count = 10;
  for (uint8_t i = 0; i < count; i++)
  {
    jArr.add(strip->getColorTempName(i));
  }
}

/*
 * Setters
 * 
 */

void setPower(uint16_t val) {
  strip->setPower(val);
  if(val)
    strip->setIsRunning(val);
}
void setIsRunning(uint16_t val) {
  strip->setIsRunning(val); 
}
void setBrightness(uint16_t val) {
  strip->setBrightness(val);
}
void setPattern(uint16_t val) {
  strip->setMode(val);
}
void setPalette(uint16_t val) {
  strip->setTargetPalette(val);
}
void setSpeed(uint16_t val) {
  strip->setBeat88(val);
}
void setBlendType(uint16_t val) {
  strip->setBlendType((TBlendType)val);
}
void setColorTemp(uint16_t val) {
  strip->setColorTemp(val);
}
void setBlurValue(uint16_t val) {
  strip->setBlur(val);
}
void setReverse(uint16_t val) {
  strip->setReverse(val);
}
void setSegments(uint16_t val) {
  strip->setSegments(val);
}
void setMirror(uint16_t val) {
  strip->setMirror(val);
}
void setInverse(uint16_t val) {
  strip->setInverse(val);
}
void setAddGlitter(uint16_t val) {
  strip->setAddGlitter(val);
}
void setWhiteOnly(uint16_t val) {
  strip->setWhiteGlitter(val);
}
void setOnBlackOnly(uint16_t val) {
  strip->setOnBlackOnly(val);
}
void setSynched(uint16_t val) {
  strip->setSynchronous(val);
}
void setHueTime(uint16_t val) {
  strip->setHuetime(val);
}
void setDeltaHue(uint16_t val) {
  strip->setDeltaHue(val);
}
void setAutoplayMode(uint16_t val) {
  strip->setAutoplay((AUTOPLAYMODES)val);
}
void setAutoplayDuration(uint16_t val) {
  strip->setAutoplayDuration(val);
}
void setAutopal(uint16_t val) {
  strip->setAutopal((AUTOPLAYMODES)val);
}
void setAutopalDuration(uint16_t val) {
strip->setAutopalDuration(val);
}
/*
// Currently no valuable solution for this
void setSolidColor(uint16_t val) {
strip->set... 
}
*/
void setCooling(uint16_t val) {
  strip->setCooling(val);
}
void setSparking(uint16_t val) {
  strip->setSparking(val);
}
void setTwinkleSpeed(uint16_t val) {
  strip->setTwinkleSpeed(val);
}
void setTwinkleDensity(uint16_t val) {
  strip->setTwinkleDensity(val);
}
void setNumBars(uint16_t val) {
  strip->setNumBars(val);
}
void setDamping(uint16_t val) {
  strip->setDamping(val);
}
void setSunRiseTime(uint16_t val) {
  strip->setSunriseTime(val);
}
void setMilliamps(uint16_t val) {
  strip->setMilliamps(val);
}
void setFPSValue(uint16_t val) {
  strip->setMaxFPS(val);
}
void setDithering(uint16_t val) {
  strip->setDithering(val);
}
void setResetDefaults(uint16_t val) {
  if(val)
  {
    strip->resetDefaults();
  }
}
void setBckndHue(uint16_t val) {
  strip->setBckndHue(val);
}
void setBckndSat(uint16_t val) {
  strip->setBckndSat(val);
}
void setBckndBri(uint16_t val) {
  strip->setBckndBri(val);
}

#ifdef HAS_KNOB_CONTROL
void setWiFiDisabled(uint16_t val) {
  if(val)
    strip->setWiFiDisabled(true);
  else
    strip->setWiFiDisabled(false);
}
#endif

#ifdef DEBUG
String getReset() {
  return String(0);
}
String getResets() {
  String json = "";
  json += "\"No Reset\",";
  json += "\"Reset Function\",";
  json += "\"Restart Function\",";
  json += "\"HW Watchdog\",";
  json += "\"SW Watchdog\",";
  json += "\"Exception\"";
  return json;
}
#endif

FieldList fields = {
  {"title",             LED_NAME,                       TitleFieldType,     0,                                   0,                                             NULL,               NULL,           NULL          },
  {"s_powerSection",    "Power / Effects",              SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL          },
  {"power",             "On/Off",                       BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getPower,           NULL,           setPower      },
  {"effect",            "Effect",                       SelectFieldType,    (uint16_t)0,                            (uint16_t)strip->getModeCount(),                  getPattern,         getPatterns,    setPattern    },
  {"brightness",        "Brightness",                   NumberFieldType,    (uint16_t)BRIGHTNESS_MIN,               (uint16_t)BRIGHTNESS_MAX,                         getBrightness,      NULL,           setBrightness },
  {"speed",             "Speed",                        NumberFieldType,    (uint16_t)BEAT88_MIN,                   (uint16_t)BEAT88_MAX,                             getSpeed,           NULL,           setSpeed      },
  {"colorPalette",      "Color palette",                SelectFieldType,    (uint16_t)0,                            (uint16_t)(strip->getPalCount() + 1),             getPalette,         getPalettes,    setPalette    },
  {"running",           "Pause",                        BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getIsRunning,       NULL,           setIsRunning  },
  {"s_stripeStruture",  "Structure",                    SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL          },
  {"segments",          "Segments",                     NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_SEGMENTS, 1),               getSegments,        NULL,           setSegments   },
  {"numEffectBars",     "# LED bars",                   NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_BARS, 1),                   getNumBars,         NULL,           setNumBars                    },
  {"reversed",          "Reverse",                      BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getReverse,         NULL,           setReverse    },
  {"mirrored",          "Mirror",                       BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getMirror,          NULL,           setMirror     },
  {"s_Autoplay",        "Autoplay",                     SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL          },
  {"autoPlay",          "Auto mode change",             SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutoplay,        getAutoplayModes, setAutoplayMode             },
  {"autoPlayInterval",  "Auto mode interval (s)",       NumberFieldType,    (uint16_t)DEFAULT_T_AUTOMODE_MIN,       (uint16_t)DEFAULT_T_AUTOMODE_MAX,                 getAutoplayDuration, NULL,          setAutoplayDuration           },
  {"autoPalette",       "Auto palette change",          SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutopal,         getAutoplayModes, setAutopal                  },
  {"autoPalInterval",   "Auto palette interval (s)",    NumberFieldType,    (uint16_t)DEFAULT_T_AUTOCOLOR_MIN,      (uint16_t)DEFAULT_T_AUTOCOLOR_MAX,                                   getAutopalDuration, NULL,           setAutopalDuration            },
  {"s_BackGroundColor", "Bcknd Color",                  SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"backgroundHue",     "Bcknd Hue",                    NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndHue,        NULL,           setBckndHue                   },
  {"backgroundSat",     "Bcknd Sat",                    NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndSat,        NULL,           setBckndSat                   },
  {"backgroundBri",     "Bcknd Bri",                    NumberFieldType,    (uint16_t)BCKND_MIN_BRI,                (uint16_t)BCKND_MAX_BRI,                          getBckndBri,        NULL,           setBckndBri                   },
  {"s_advancedControl", "Advanced",                     SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"blendType",         "Color blend",                  SelectFieldType,    (uint16_t)NOBLEND,                      (uint16_t)LINEARBLEND,                            getBlendType,       getBlendTypes,  setBlendType                  },
  {"colorTemperature",  "Color temperature",            SelectFieldType,    (uint16_t)0,                            (uint16_t)20,                                     getColorTemp,       getColorTemps,  setColorTemp                  },
  {"ledBlur",           "Effect blur / blending",       NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBlurValue,       NULL,           setBlurValue                  },
  {"s_solidColor",      "Solid color",                  SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"solidColor",        "Color",                        ColorFieldType,     (uint16_t)0,                            (uint16_t)55,                                     getSolidColor,      NULL,           NULL                          },
  {"s_glitter",         "Glitter",                      SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"addGlitter",        "Add Glitter",                  BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getAddGlitter,      NULL,           setAddGlitter                 },
  {"whiteGlitter",      "White Glitter",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWhiteOnly,       NULL,           setWhiteOnly                  },
  {"onBlackOnly",       "On Black",                     BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getOnBlackOnly,     NULL,           setOnBlackOnly                },
  {"syncGlitter",       "Sync Segments",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getSynched,         NULL,           setSynched,             },
  {"s_basicHue",        "Hue Change",                   SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"hueTime",           "Hue interval (ms)",            NumberFieldType,    (uint16_t)0,                            (uint16_t)5000,                                   getHueTime,         NULL,           setHueTime                    },
  {"deltaHue",          "Hue Offset",                   NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getDeltaHue,        NULL,           setDeltaHue                   },    
  {"s_effectSettings",  "Effect Settings",              SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  {"cooling",           "Cooling",                      NumberFieldType,    (uint16_t)DEFAULT_COOLING_MIN,          (uint16_t)DEFAULT_COOLING_MAX,                    getCooling,         NULL,           setCooling},
  {"sparking",          "Sparking",                     NumberFieldType,    (uint16_t)DEFAULT_SPARKING_MIN,         (uint16_t)DEFAULT_SPARKING_MAX,                   getSparking,        NULL,           setSparking                   },
  {"twinkleSpeed",      "Twinkle speed",                NumberFieldType,    (uint16_t)DEFAULT_TWINKLE_S_MIN,        (uint16_t)DEFAULT_TWINKLE_S_MAX,                  getTwinkleSpeed,    NULL,           setTwinkleSpeed               },
  {"twinkleDensity",    "Twinkle density",              NumberFieldType,    (uint16_t)DEFAULT_TWINKLE_NUM_MIN,      (uint16_t)DEFAULT_TWINKLE_NUM_MAX,                getTwinkleDensity,  NULL,           setTwinkleDensity             },
  {"damping",           "damping for bounce",           NumberFieldType,    (uint16_t)DEFAULT_DAMPING_MIN,          (uint16_t)DEFAULT_DAMPING_MAX,                    getDamping,         NULL,           setDamping                    },
  // time provided in Minutes and capped at 60 minutes actually.
  {"sunriseset",        "sunrise, sunset time in min",  NumberFieldType,    (uint16_t)DEFAULT_SUNRISETIME_MIN,      (uint16_t)DEFAULT_SUNRISETIME_MAX,                getSunRiseTime,     NULL,           setSunRiseTime                }, 
  {"s_otherSettings",   "Other settings",               SectionFieldType,   0,                                   0,                                             NULL,               NULL,           NULL                          },
  #ifdef HAS_KNOB_CONTROL
  {"wifiDisabled",      "WiFi Disabled",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWiFiDisabled,    NULL,           setWiFiDisabled               },  
  #endif
  {"currentLimit",      "Current limit",                NumberFieldType,    (uint16_t)100,                          (uint16_t)DEFAULT_CURRENT_MAX,                    getMilliamps,       NULL,           setMilliamps                  },
  // 111 max equals the minimum update time required for 300 pixels
  // this is the minimal delay being used anyway, so no use in being faster
  {"fps",               "max FPS",                      NumberFieldType,    (uint16_t)STRIP_MIN_FPS,                (uint16_t)(STRIP_MAX_FPS),                        getFPSValue,        NULL,           setFPSValue                   },                                                                           
  {"dithering",         "Dithering",                    BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getDithering,       NULL,           setDithering                  },
  {"resetdefaults",     "Reset default",                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getResetDefaults,   NULL,           setResetDefaults              },

#ifdef DEBUG
  // With the DEBUG flag enabled we can provoke some resets (SOFT WDT, HW WDT, Exception...)
  {"S_Debug",           "DEBUG only - not for production",        SectionFieldType                                                                                                        },
  {"resets",            "Resets (DEV Debug)",                     SelectFieldType,    (uint16_t)0,                            (uint16_t)5,                                      getReset,         getResets   },
#endif
};


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

uint8_t fieldCount = ARRAY_SIZE(fields);

void stripe_setup(const uint8_t volt = (uint8_t)5,
                  const LEDColorCorrection colc = UncorrectedColor /*TypicalLEDStrip*/)
{
  strip = new WS2812FX(volt, colc);
  strip->init();
  strip->start();
  strip->show();
}