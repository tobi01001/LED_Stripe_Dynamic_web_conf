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

String setFieldValue(String name, String value, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.setValue)
  {
    return field.setValue(value);
  }
  return String();
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

String getPower()
{
  return String(strip->getPower());
}

String getBrightness()
{
  return String(strip->getBrightness());
}

String getPattern()
{
  return String(strip->getMode());
}

String getPatterns()
{
  String json = "";

  for (uint8_t i = 0; i < strip->getModeCount(); i++)
  {
    json += "\"" + String(strip->getModeName(i)) + "\"";
    if (i < strip->getModeCount() - 1)
      json += ",";
  }

  return json;
}

String getPalette()
{
  return String(strip->getTargetPaletteNumber());
}

#ifdef DEBUG
String getReset()
{
  return String(0);
}
String getResets()
{
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

String getPalettes()
{
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

String getAutoplayModes()
{
  String json = "";
  json += "\"Off\",";
  json += "\"Up\",";
  json += "\"Down\",";
  json += "\"Random\"";
  return json;
}

String getInverse()
{
  return String(strip->getInverse());
}

String getMirror()
{
  return String(strip->getMirror());
}

String getAutoplay()
{
  return String(strip->getAutoplay());
}

String getAutoplayDuration()
{
  return String(strip->getAutoplayDuration());
}

String getAutopal()
{
  return String(strip->getAutopal());
}

String getAutopalDuration()
{
  return String(strip->getAutopalDuration());
}

String getSolidColor()
{
  CRGB solidColor = strip->getTargetPalette().entries[0];
  return String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
}

String getCooling()
{
  return String(strip->getCooling());
}

String getSparking()
{
  return String(strip->getSparking());
}

String getSpeed()
{
  return String(strip->getBeat88());
}

String getTwinkleSpeed()
{
  return String(strip->getTwinkleSpeed());
}

String getTwinkleDensity()
{
  return String(strip->getTwinkleDensity());
}

String getNumBars()
{
  return String(strip->getNumBars());
}

String getHueTime()
{
  return String(strip->getHueTime());
}

String getDeltaHue()
{
  return String(strip->getDeltaHue());
}

String getBlendType()
{
  return String(strip->getBlendType());
}

String getDamping()
{
  return String(strip->getDamping());
}

String getBlendTypes()
{
  return "\"NoBlend\",\"LinearBlend\"";
}

String getColorTemp()
{
  return String(strip->getColorTemp());
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

String getReverse()
{
  return String(strip->getReverse());
}

String getMilliamps(void)
{
  return String(strip->getMilliamps());
}

String getBlurValue(void)
{
  return String(strip->getBlurValue());
}

String getFPSValue(void)
{
  return String(strip->getMaxFPS());
}

String getSegments(void)
{
  return String(strip->getSegments());
}

String getResetDefaults(void)
{
  return String(0);
}

String getDithering(void)
{
  return String(strip->getDithering());
}

String getSunRiseTime(void)
{
  return String(strip->getSunriseTime());
}

String getIsRunning(void)
{
  return String(strip->isRunning());
}

String getAddGlitter(void)
{
  return String(strip->getAddGlitter());
}

String getWhiteOnly(void)
{
  return String(strip->getWhiteGlitter());
}

String getOnBlackOnly(void)
{
  return String(strip->getOnBlackOnly());
}

String getChanceOfGlitter(void)
{
  return String(strip->getChanceOfGlitter());
}

FieldList fields = {
    {"title",             LED_NAME,                                 TitleFieldType                                                                                                          },
    {"powerSection",      LED_NAME,                                 SectionFieldType                                                                                                        },
    {"power",             "LED Power",                              BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getPower                      },
    {"isRunning",         "Running / Pause",                        BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getIsRunning                  },
    {"basicControl",      "Basic control",                          SectionFieldType                                                                                                        },
    {"br",                "Brightness",                             NumberFieldType,    (uint16_t)BRIGHTNESS_MIN,               (uint16_t)BRIGHTNESS_MAX,                         getBrightness                 },
    {"mo",                "Effect",                                 SelectFieldType,    (uint16_t)0,                            (uint16_t)strip->getModeCount(),                  getPattern,     getPatterns   },
    {"pa",                "Color palette",                          SelectFieldType,    (uint16_t)0,                            (uint16_t)(strip->getPalCount() + 1),   getPalette,     getPalettes   },
    {"sp",                "Speed (beat88)",                         NumberFieldType,    (uint16_t)BEAT88_MIN,                   (uint16_t)BEAT88_MAX,                             getSpeed                      },
    {"blendType",         "Color blend type",                       SelectFieldType,    (uint16_t)NOBLEND,                      (uint16_t)LINEARBLEND,                            getBlendType,   getBlendTypes },
    {"ColorTemperature",  "Color temperature",                      SelectFieldType,    (uint16_t)0,                            (uint16_t)20,                                     getColorTemp,   getColorTemps },
    {"LEDblur",           "LED effect blur / blending",             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBlurValue                  },
    {"reverse",           "Reverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getReverse                    },
    {"segments",          "Segments",                               NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_SEGMENTS, 1),               getSegments                   },
    {"mirror",            "Mirror",                                 BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getMirror                     },
    {"inverse",           "Inverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getInverse                    },
    {"glitter",           "Glitter / sparks",                       SectionFieldType                                                                                                        },
    {"addGlitter",        "Add Glitter",                            BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getAddGlitter                 },
    {"WhiteOnly",         "White Glitter",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWhiteOnly                  },
    {"onBlackOnly",       "On Black Only",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getOnBlackOnly                },
    {"glitterChance",     "Chance of Glitter",                      NumberFieldType,    (uint16_t)DEFAULT_GLITTER_CHANCE_MIN,   (uint16_t)DEFAULT_GLITTER_CHANCE_MAX,             getChanceOfGlitter            },     
    {"hue",               "Color Change / Hue Change",              SectionFieldType                                                                                                        },
    {"huetime",           "Hue change interval (ms)",               NumberFieldType,    (uint16_t)0,                            (uint16_t)5000,                                   getHueTime                    },
    {"deltahue",          "Hue Offset",                             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getDeltaHue                   },
    {"autoplay",          "Mode Autoplay",                          SectionFieldType                                                                                                        },
    {"autoplay",          "Automatic mode change",                  SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutoplay, getAutoplayModes },
    {"autoplayDuration",  "Automatic mode change interval (s)",     NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutoplayDuration           },
    {"autopal",           "Color Palette Autoplay",                 SectionFieldType                                                                                                        },
    {"autopal",           "Automatic color palette change",         SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutopal,  getAutoplayModes },
    {"autopalDuration",   "Automatic palette change interval (s)",  NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutopalDuration            },
    {"solidColor",        "Solid color",                            SectionFieldType                                                                                                        },
    {"solidColor",        "Color",                                  ColorFieldType,     (uint16_t)0,                            (uint16_t)55,                                     getSolidColor                 },
    {"fire",              "Fire and water settings",                SectionFieldType                                                                                                        },
    {"cooling",           "Cooling",                                NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getCooling                    },
    {"sparking",          "Sparking",                               NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getSparking                   },
    {"twinkles",          "Twinkle settings",                       SectionFieldType                                                                                                        },
    {"twinkleSpeed",      "Twinkle speed",                          NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleSpeed               },
    {"twinkleDensity",    "Twinkle density",                        NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleDensity             },
    {"ledBars",           "Other settings",                         SectionFieldType                                                                                                        },
    {"numBars",           "Number of LED bars for effects",         NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_BARS, 1),                   getNumBars                    },
    {"damping",           "damping for bounce",                     NumberFieldType,    (uint16_t)0,                            (uint16_t)100,                                    getDamping                    },
    // time provided in Minutes and capped at 60 minutes actually.
    {"sunriseset",        "sunrise and sunset time in minutes",     NumberFieldType,    (uint16_t)1,                            (uint16_t)60,                                     getSunRiseTime                }, 
    {"current",           "Current limit",                          NumberFieldType,    (uint16_t)100,                          (uint16_t)DEFAULT_CURRENT_MAX,                    getMilliamps                  },
    // 111 max equals the minimum update time required for 300 pixels
    // this is the minimal delay being used anyway, so no use in being faster
    {"fps",               "Frames per second (FPS)",                NumberFieldType,    (uint16_t)STRIP_MIN_FPS,                (uint16_t)(STRIP_MAX_FPS),                        getFPSValue                   },                                                                           
    {"dithering",         "Dithering",                              BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getDithering                  },
    {"resetdefaults",     "Reset default values",                   BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getResetDefaults              },
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