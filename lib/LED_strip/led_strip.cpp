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

const String TitleFieldType = "Title";
const String NumberFieldType = "Number";
const String BooleanFieldType = "Boolean";
const String SelectFieldType = "Select";
const String ColorFieldType = "Color";
const String SectionFieldType = "Section";

Field getField(String name, FieldList fields, uint8_t count)
{
  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (field.name == name)
    {
      return field;
    }
  }
  return Field();
}

String getFieldValue(String name, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.getValue)
  {
    return field.getValue();
  }
  return String();
}

void setFieldValue(String name, uint16_t value, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.setValue)
  {
    field.setValue(value);
  }
}

String getFieldsJson(FieldList fields, uint8_t count)
{
  String json = "[";

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];

    json += "{\"name\":\"" + field.name + "\",\"label\":\"" + field.label + "\",\"type\":\"" + field.type + "\"";

    if (field.getValue)
    {
      if (field.type == ColorFieldType || field.type == "String")
      {
        json += ",\"value\":\"" + field.getValue() + "\"";
      }
      else
      {
        json += ",\"value\":" + field.getValue();
      }
    }

    if (field.type == NumberFieldType)
    {
      json += ",\"min\":" + String(field.min);
      json += ",\"max\":" + String(field.max);
    }

    if (field.getOptions)
    {
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

/*
 *
 * get Values
 * 
 */
String getPower() {
  return String(strip->getPower());
}
String getIsRunning(void) {
  return String(strip->isRunning());
}
String getBrightness() {
  return String(strip->getBrightness());
}
String getPattern() {
  return String(strip->getMode());
}
String getPalette() {
  return String(strip->getTargetPaletteNumber());
}
String getSpeed() {
  return String(strip->getBeat88());
}
String getBlendType() {
  return String(strip->getBlendType());
}
String getColorTemp() {
  return String(strip->getColorTemp());
}
String getBlurValue(void) {
  return String(strip->getBlurValue());
}
String getReverse() {
  return String(strip->getReverse());
}
String getSegments(void) {
  return String(strip->getSegments());
}
String getMirror() {
  return String(strip->getMirror());
}
String getInverse() {
  return String(strip->getInverse());
}
String getAddGlitter(void) {
  return String(strip->getAddGlitter());
}
String getWhiteOnly(void) {
  return String(strip->getWhiteGlitter());
}
String getOnBlackOnly(void) {
  return String(strip->getOnBlackOnly());
}
String getChanceOfGlitter(void) {
  return String(strip->getChanceOfGlitter());
}
String getHueTime() {
  return String(strip->getHueTime());
}
String getDeltaHue()
{
  return String(strip->getDeltaHue());
}
String getAutoplay() {
  return String(strip->getAutoplay());
}
String getAutoplayDuration() {
  return String(strip->getAutoplayDuration());
}
String getAutopal() {
  return String(strip->getAutopal());
}
String getAutopalDuration(){
  return String(strip->getAutopalDuration());
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
String getTwinkleSpeed() {
  return String(strip->getTwinkleSpeed());
}
String getTwinkleDensity() {
  return String(strip->getTwinkleDensity());
}
String getNumBars() {
  return String(strip->getNumBars());
}
String getDamping() {
  return String(strip->getDamping());
}
String getSunRiseTime(void) {
  return String(strip->getSunriseTime());
}
String getMilliamps(void) {
  return String(strip->getMilliamps());
}
String getFPSValue(void) {
  return String(strip->getMaxFPS());
}
String getDithering(void) {
  return String(strip->getDithering());
}
String getResetDefaults(void) {
  return String(0);
}

/*
 * Options
 * 
 */
String getPatterns() {
  String json = "";

  for (uint8_t i = 0; i < strip->getModeCount(); i++)
  {
    json += "\"" + String(strip->getModeName(i)) + "\"";
    if (i < strip->getModeCount() - 1)
      json += ",";
  }

  return json;
}
String getPalettes() {
  String json = "";

  for (uint8_t i = 0; i < strip->getPalCount(); i++)
  {
    json += "\"" + String(strip->getPalName(i)) + "\"";
    //if (i < strip->getPalCount() - 1)
    json += ",";
  }
  json += "\"Custom\"";

  return json;
}

String getAutoplayModes() {
  String json = "";
  json += "\"Off\",";
  json += "\"Up\",";
  json += "\"Down\",";
  json += "\"Random\"";
  return json;
}
String getBlendTypes()
{
  return "\"NoBlend\",\"LinearBlend\"";
}
String getColorTemps()
{
  String json = "";

  for (uint8_t i = 0; i < 19; i++)
  {
    json += "\"" + String(strip->getColorTempName(i)) + "\"";
    json += ",";
  }
  json += "\"" + String(strip->getColorTempName(19)) + "\"";

  return json;
  //return "\"Candle\",\"Kelvin\",\"Tungsten40W\",\"Tungsten100W\",\"Halogen\",\"CarbonArc\",\"HighNoonSun\",\"DirectSunlight\",\"OvercastSky\",\"ClearBlueSky\",\"WarmFluorescent\",\"StandardFluorescent\",\"CoolWhiteFluorescent\",\"FullSpectrumFluorescent\",\"GrowLightFluorescent\",\"BlackLightFluorescent\",\"MercuryVapor\",\"SodiumVapor\",\"MetalHalide\",\"HighPressureSodium\",\"UncorrectedTemperature\"";
}

/*
 * Setters
 * 
 */

void setPower(uint16_t val) {
  strip->setPower(val);
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
void setChanceOfGlitter(uint16_t val) {
  strip->setChanceOfGlitter(val);
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
  strip->resetDefaults();
}


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
    {"title",             LED_NAME,                                 TitleFieldType,     NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"powerSection",      LED_NAME + String(" power"),                                 SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"power",             "LEDs On/Off",                              BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getPower,           NULL,           setPower      },
    {"isRunning",         "Running / Pause",                        BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getIsRunning,       NULL,           setIsRunning  },
    {"basicControl",      "Basic control",                          SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"br",                "Brightness",                             NumberFieldType,    (uint16_t)BRIGHTNESS_MIN,               (uint16_t)BRIGHTNESS_MAX,                         getBrightness,      NULL,           setBrightness },
    {"mo",                "Effect",                                 SelectFieldType,    (uint16_t)0,                            (uint16_t)strip->getModeCount(),                  getPattern,         getPatterns,    setPattern    },
    {"pa",                "Color palette",                          SelectFieldType,    (uint16_t)0,                            (uint16_t)(strip->getPalCount() + 1),             getPalette,         getPalettes,    setPalette    },
    {"sp",                "Speed (beat88)",                         NumberFieldType,    (uint16_t)BEAT88_MIN,                   (uint16_t)BEAT88_MAX,                             getSpeed,           NULL,           setSpeed      },
    {"blendType",         "Color blend type",                       SelectFieldType,    (uint16_t)NOBLEND,                      (uint16_t)LINEARBLEND,                            getBlendType,       getBlendTypes,  setBlendType  },
    {"ColorTemperature",  "Color temperature",                      SelectFieldType,    (uint16_t)0,                            (uint16_t)20,                                     getColorTemp,       getColorTemps,  setColorTemp  },
    {"LEDblur",           "LED effect blur / blending",             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBlurValue,       NULL,           setBlurValue  },
    {"reverse",           "Reverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getReverse,         NULL,           setReverse    },
    {"segments",          "Segments",                               NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_SEGMENTS, 1),               getSegments,        NULL,           setSegments   },
    {"mirror",            "Mirror",                                 BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getMirror,          NULL,           setMirror     },
    {"inverse",           "Inverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getInverse,         NULL,           setInverse    },
    {"glitter",           "Glitter / sparks",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"addGlitter",        "Add Glitter",                            BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getAddGlitter,      NULL,           setAddGlitter                 },
    {"WhiteOnly",         "White Glitter",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWhiteOnly,       NULL,           setWhiteOnly                  },
    {"onBlackOnly",       "On Black Only",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getOnBlackOnly,     NULL,           setOnBlackOnly                },
    {"glitterChance",     "Chance of Glitter",                      NumberFieldType,    (uint16_t)DEFAULT_GLITTER_CHANCE_MIN,   (uint16_t)DEFAULT_GLITTER_CHANCE_MAX,             getChanceOfGlitter, NULL,           setChanceOfGlitter            },     
    {"hue",               "Color Change / Hue Change",              SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"huetime",           "Hue change interval (ms)",               NumberFieldType,    (uint16_t)0,                            (uint16_t)5000,                                   getHueTime,     NULL, setHueTime                    },
    {"deltahue",          "Hue Offset",                             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getDeltaHue,    NULL, setDeltaHue                   },
    {"autoplay",          "Mode Autoplay",                          SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"autoplay",          "Automatic mode change",                  SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutoplay,    getAutoplayModes, setAutoplayMode },
    {"autoplayDuration",  "Automatic mode change interval (s)",     NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutoplayDuration, NULL, setAutoplayDuration           },
    {"autopal",           "Color Palette Autoplay",                 SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"autopal",           "Automatic color palette change",         SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutopal,     getAutoplayModes, setAutopal },
    {"autopalDuration",   "Automatic palette change interval (s)",  NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutopalDuration, NULL, setAutopalDuration            },
    {"solidColor",        "Solid color",                            SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"solidColor",        "Color",                                  ColorFieldType,     (uint16_t)0,                            (uint16_t)55,                                     getSolidColor,  NULL, NULL                 },
    {"fire",              "Fire and water settings",                SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"cooling",           "Cooling",                                NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getCooling,     NULL, setCooling},
    {"sparking",          "Sparking",                               NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getSparking,    NULL, setSparking                   },
    {"twinkles",          "Twinkle settings",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"twinkleSpeed",      "Twinkle speed",                          NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleSpeed, NULL, setTwinkleSpeed               },
    {"twinkleDensity",    "Twinkle density",                        NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleDensity, NULL, setTwinkleDensity             },
    {"ledBars",           "Other settings",                         SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"numBars",           "Number of LED bars for effects",         NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_BARS, 1),                   getNumBars,     NULL, setNumBars                    },
    {"damping",           "damping for bounce",                     NumberFieldType,    (uint16_t)0,                            (uint16_t)100,                                    getDamping,   NULL, setDamping                    },
    // time provided in Minutes and capped at 60 minutes actually.
    {"sunriseset",        "sunrise and sunset time in minutes",     NumberFieldType,    (uint16_t)1,                            (uint16_t)60,                                     getSunRiseTime, NULL, setSunRiseTime                }, 
    {"current",           "Current limit",                          NumberFieldType,    (uint16_t)100,                          (uint16_t)DEFAULT_CURRENT_MAX,                    getMilliamps, NULL, setMilliamps                  },
    // 111 max equals the minimum update time required for 300 pixels
    // this is the minimal delay being used anyway, so no use in being faster
    {"fps",               "Frames per second (FPS)",                NumberFieldType,    (uint16_t)STRIP_MIN_FPS,                (uint16_t)(STRIP_MAX_FPS),                        getFPSValue, NULL, setFPSValue                   },                                                                           
    {"dithering",         "Dithering",                              BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getDithering, NULL, setDithering                  },
    {"resetdefaults",     "Reset default values",                   BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getResetDefaults, NULL, setResetDefaults              },

#ifdef DEBUG
    // With the DEBUG flag enabled we can provoke some resets (SOFT WDT, HW WDT, Exception...)
    {"Debug",             "DEBUG only - not for production",        SectionFieldType                                                                                                        },
    {"resets",            "Resets (DEV Debug)",                     SelectFieldType,    (uint16_t)0,                            (uint16_t)5,                                      getReset,         getResets   },
#endif
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

uint8_t fieldCount = ARRAY_SIZE(fields);

// set all pixels to 'off'
void stripe_setup(const uint16_t LEDCount,
                  const uint8_t FPS = (uint8_t)60,
                  const uint8_t volt = (uint8_t)5,
                  const uint16_t milliamps = (uint16_t)500,
                  const CRGBPalette16 pal = Rainbow_gp,
                  const String Name = "Rainbow Colors",
                  const LEDColorCorrection colc = UncorrectedColor /*TypicalLEDStrip*/)
{
  strip = new WS2812FX(FPS, volt, milliamps, pal, Name, colc);
  strip->init();
  strip->start();
  strip->show();
}

uint32_t strip_color32(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
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
  uint32_t dimColor = strip_color32(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}

// Dim the Pixel to DimColor
void strip_dimPixel(uint16_t pixel, bool dim_default, uint8_t byValue)
{
  if (dim_default)
    strip->leds[pixel].fadeToBlackBy(2);
  else
  {
    strip->leds[pixel].subtractFromRGB(byValue);
  }
}