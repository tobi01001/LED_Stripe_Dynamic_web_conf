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

const char * field_strings[][2] PROGMEM =
{  
  {"title",             LED_NAME                        },
  {"powerSection",	    "Power"                         },
  {"power",	            "On/Off"                        },
  {"isRunning",	        "Pause"                         },
  {"basicControl",	    "Basic"                         },
  {"br",	              "Brightness"                    },
  {"mo",                "Effect"                        },
  {"pa",	              "Color palette"                 },
  {"sp",	              "Speed"                         },
  {"stripeStruture",	  "Structrue"                     },
  {"segments",	        "Segments"                      },
  {"numBars",	          "# LED bars"                    },
  {"reverse",	          "Reverse"                       },
  {"mirror",	          "Mirror"                        },
  {"autoplay",	        "Autoplay"                      },
  {"autoplay",	        "Auto mode change"              },
  {"autoplayDuration",  "Auto mode interval (s)"        },
  {"autopal",	          "Auto palette change"           },
  {"autopalDuration",	  "Auto palette interval (s)"     },
  {"BackGroundColor",	  "Bcknd Color"                   },
  {"BckndHue",	        "Bcknd Hue"                     },
  {"BckndSat",	        "Bcknd Sat"                     },
  {"BckndBri",	        "Bcknd Bri"                     },
  {"advancedControl",	  "Advanced"                      },
  {"blendType",	        "Color blend"                   },
  {"ColorTemperature",	"Color temperature"             },
  {"LEDblur",	          "Effect blur / blending"        },
  {"solidColor",	      "Solid color"                   },
  {"solidColor",	      "Color"                         },
  {"glitter",	          "Glitter"                       },
  {"addGlitter",	      "Add Glitter"                   },
  {"WhiteOnly",	        "White Glitter"                 },
  {"onBlackOnly",	      "On Black"                      },
  {"glitterChance",	    "Chance of Glitter"             },
  {"hue",	              "Hue Change"                    },
  {"huetime",	          "Hue interval (ms)"             },
  {"deltahue",	        "Hue Offset"                    },
  {"effectSettings",  	"Effect Settings"               },
  {"cooling",	          "Cooling"                       },
  {"sparking",	        "Sparking"                      },
  {"twinkleSpeed",	    "Twinkle speed"                 },
  {"twinkleDensity",	  "Twinkle density"               },
  {"damping",	          "damping for bounce"            },
  {"sunriseset",	      "sunrise, sunset time in min"   },
  {"otherSettings",	    "Other settings"                },
  #ifdef HAS_KNOB_CONTROL
  {"wifiEnabled",	      "WiFi On/Off"                   },
  #endif
  {"current",	          "Current limit"                 },
  {"fps",	              "max FPS"                       },
  {"dithering",	        "Dithering"                     },
  {"resetdefaults",	    "Reset default"                 }
#ifdef DEBUG
   ,
  {"Debug",	            "DEBUG only - not for production"}, 
  {"resets",	          "Resets (DEV Debug)"            }
#endif
};


const char *TitleFieldType = "Title";
const char *NumberFieldType = "Number";
const char *BooleanFieldType = "Boolean";
const char *SelectFieldType = "Select";
const char *ColorFieldType = "Color";
const char *SectionFieldType = "Section";

Field getField(const char * name, FieldList fields, uint8_t count)
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

String getFieldValue(const char * name, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.getValue)
  {
    return String(field.getValue());
  }
  return String();
}

void setFieldValue(const char name[], uint16_t value, FieldList fields, uint8_t count)
{
  Field field = getField(name, fields, count);
  if (field.setValue)
  {
    field.setValue(value);
  }
}

const char * getFieldsJson(FieldList fields, uint8_t count)
{
  String json = "[";

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];

    json += "{\"name\":\"" + String(field.name) + "\",\"label\":\"" + String(field.label) + "\",\"type\":\"" + String(field.type) + "\"";

    if (field.getValue)
    {
      if (field.type == ColorFieldType || field.type == (const char*)"String")
      {
        json += ",\"value\":\"" + String(field.getValue()) + "\"";
      }
      else
      {
        json += ",\"value\":" + String(field.getValue());
      }
    }

    if (field.type == NumberFieldType)
    {
      json += ",\"min\":" + String(field.min);
      json += ",\"max\":" + String(field.max);
    }

    if (field.getOptions)
    {
      String opts ="";
      DynamicJsonBuffer buf;
      JsonArray & jArr = buf.createArray();
      field.getOptions(jArr);
      opts.reserve(jArr.measureLength());
      jArr.printTo(opts);
      json += ",\"options\":[";
      json += opts;
      json += "]";
    }

    json += "}";

    if (i < count - 1)
      json += ",";
  }

  json += "]";

  return json.c_str();
}

/*
 *
 * get Values
 * 
 */
uint16_t getPower() {
  return uint16_t(strip->getPower());
}
uint16_t getIsRunning(void) {
  return uint16_t(strip->isRunning());
}
uint16_t getBrightness() {
  return uint16_t(strip->getBrightness());
}
uint16_t getPattern() {
  return uint16_t(strip->getMode());
}
uint16_t getPalette() {
  return uint16_t(strip->getTargetPaletteNumber());
}
uint16_t getSpeed() {
  return uint16_t(strip->getBeat88());
}
uint16_t getBlendType() {
  return uint16_t(strip->getBlendType());
}
uint16_t getColorTemp() {
  return uint16_t(strip->getColorTemp());
}
uint16_t getBlurValue(void) {
  return uint16_t(strip->getBlurValue());
}
uint16_t getReverse() {
  return uint16_t(strip->getReverse());
}
uint16_t getSegments(void) {
  return uint16_t(strip->getSegments());
}
uint16_t getMirror() {
  return uint16_t(strip->getMirror());
}
uint16_t getInverse() {
  return uint16_t(strip->getInverse());
}
uint16_t getAddGlitter(void) {
  return uint16_t(strip->getAddGlitter());
}
uint16_t getWhiteOnly(void) {
  return uint16_t(strip->getWhiteGlitter());
}
uint16_t getOnBlackOnly(void) {
  return uint16_t(strip->getOnBlackOnly());
}
uint16_t getChanceOfGlitter(void) {
  return uint16_t(strip->getChanceOfGlitter());
}
uint16_t getHueTime() {
  return uint16_t(strip->getHueTime());
}
uint16_t getDeltaHue()
{
  return uint16_t(strip->getDeltaHue());
}
uint16_t getAutoplay() {
  return uint16_t(strip->getAutoplay());
}
uint16_t getAutoplayDuration() {
  return uint16_t(strip->getAutoplayDuration());
}
uint16_t getAutopal() {
  return uint16_t(strip->getAutopal());
}
uint16_t getAutopalDuration(){
  return uint16_t(strip->getAutopalDuration());
}
uint16_t getSolidColor() {
  CRGB solidColor = strip->getTargetPalette().entries[0];
  //return uint16_t(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
  return 250; // hmmm what to do with this solid color field? limit t one byte? Change all to uint32 to enable color?
}
uint16_t getCooling() {
  return uint16_t(strip->getCooling());
}
uint16_t getSparking() {
  return uint16_t(strip->getSparking());
}
uint16_t getTwinkleSpeed() {
  return uint16_t(strip->getTwinkleSpeed());
}
uint16_t getTwinkleDensity() {
  return uint16_t(strip->getTwinkleDensity());
}
uint16_t getNumBars() {
  return uint16_t(strip->getNumBars());
}
uint16_t getDamping() {
  return uint16_t(strip->getDamping());
}
uint16_t getSunRiseTime(void) {
  return uint16_t(strip->getSunriseTime());
}
uint16_t getMilliamps(void) {
  return uint16_t(strip->getMilliamps());
}
uint16_t getFPSValue(void) {
  return uint16_t(strip->getMaxFPS());
}
uint16_t getDithering(void) {
  return uint16_t(strip->getDithering());
}
uint16_t getResetDefaults(void) {
  return uint16_t(0);
}
uint16_t getBckndHue() {
  return uint16_t(strip->getBckndHue());
}
uint16_t getBckndSat() {
  return uint16_t(strip->getBckndSat());
}
uint16_t getBckndBri() {
  return uint16_t(strip->getBckndBri());
}



#ifdef HAS_KNOB_CONTROL
uint16_t getWiFiEnabled(void) {
  return uint16_t(strip->getWiFiEnabled());
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

  jArr.add("\"Custom\"");
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
void setWiFiEnabled(uint16_t val) {
  if(val)
    strip->setWiFiEnabled(true);
  else
    strip->setWiFiEnabled(false);
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
 { field_strings[ 0][0],	field_strings[ 0][1],	TitleFieldType,		NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[ 1][0],	field_strings[ 1][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[ 2][0],	field_strings[ 2][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getPower,	NULL,	setPower },
 { field_strings[ 3][0],	field_strings[ 3][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getIsRunning,	NULL,	setIsRunning },
 { field_strings[ 4][0],	field_strings[ 4][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[ 5][0],	field_strings[ 5][1],	NumberFieldType,	(uint16_t)BRIGHTNESS_MIN,	(uint16_t)BRIGHTNESS_MAX,	getBrightness,	NULL,	setBrightness },
 { field_strings[ 6][0],	field_strings[ 6][1],	SelectFieldType,	(uint16_t)0,	(uint16_t)strip->getModeCount(),	getPattern,	getPatterns,	setPattern },
 { field_strings[ 7][0],	field_strings[ 7][1],	SelectFieldType,	(uint16_t)0,	(uint16_t)(strip->getPalCount() + 1),	getPalette,	getPalettes,	setPalette },
 { field_strings[ 8][0],	field_strings[ 8][1],	NumberFieldType,	(uint16_t)BEAT88_MIN,	(uint16_t)BEAT88_MAX,	getSpeed,	NULL,	setSpeed },
 { field_strings[ 9][0],	field_strings[ 9][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[10][0],	field_strings[10][1],	NumberFieldType,	(uint16_t)1,	(uint16_t)max(MAX_NUM_SEGMENTS,	1),	getSegments,	NULL,	setSegments },
 { field_strings[11][0],	field_strings[11][1],	NumberFieldType,	(uint16_t)1,	(uint16_t)max(MAX_NUM_BARS,	1),	getNumBars,	NULL,	setNumBars },
 { field_strings[12][0],	field_strings[12][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getReverse,	NULL,	setReverse },
 { field_strings[13][0],	field_strings[13][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getMirror,	NULL,	setMirror },
 { field_strings[14][0],	field_strings[14][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[15][0],	field_strings[15][1],	SelectFieldType,	(uint16_t)AUTO_MODE_OFF,	(uint16_t)AUTO_MODE_RANDOM,	getAutoplay,	getAutoplayModes,	setAutoplayMode },
 { field_strings[16][0],	field_strings[16][1],	NumberFieldType,	(uint16_t)5,	(uint16_t)1000,	getAutoplayDuration,	NULL,	setAutoplayDuration },
 { field_strings[17][0],	field_strings[17][1],	SelectFieldType,	(uint16_t)AUTO_MODE_OFF,	(uint16_t)AUTO_MODE_RANDOM,	getAutopal,	getAutoplayModes,	setAutopal },
 { field_strings[18][0],	field_strings[18][1],	NumberFieldType,	(uint16_t)5,	(uint16_t)1000,	getAutopalDuration,	NULL,	setAutopalDuration },
 { field_strings[19][0],	field_strings[19][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[20][0],	field_strings[20][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getBckndHue,	NULL,	setBckndHue },
 { field_strings[21][0],	field_strings[21][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getBckndSat,	NULL,	setBckndSat },
 { field_strings[22][0],	field_strings[22][1],	NumberFieldType,	(uint16_t)BCKND_MIN_BRI,	(uint16_t)BCKND_MAX_BRI,	getBckndBri,	NULL,	setBckndBri },
 { field_strings[23][0],	field_strings[23][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[24][0],	field_strings[24][1],	SelectFieldType,	(uint16_t)NOBLEND,	(uint16_t)LINEARBLEND,	getBlendType,	getBlendTypes,	setBlendType },
 { field_strings[25][0],	field_strings[25][1],	SelectFieldType,	(uint16_t)0,	(uint16_t)20,	getColorTemp,	getColorTemps,	setColorTemp },
 { field_strings[26][0],	field_strings[26][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getBlurValue,	NULL,	setBlurValue },
 { field_strings[27][0],	field_strings[27][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[28][0],	field_strings[28][1],	ColorFieldType,		(uint16_t)0,	(uint16_t)55,	getSolidColor,	NULL,	NULL },
 { field_strings[29][0],	field_strings[29][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[30][0],	field_strings[30][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getAddGlitter,	NULL,	setAddGlitter },
 { field_strings[31][0],	field_strings[31][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getWhiteOnly,	NULL,	setWhiteOnly },
 { field_strings[32][0],	field_strings[32][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getOnBlackOnly,	NULL,	setOnBlackOnly },
 { field_strings[33][0],	field_strings[33][1],	NumberFieldType,	(uint16_t)DEFAULT_GLITTER_CHANCE_MIN,	(uint16_t)DEFAULT_GLITTER_CHANCE_MAX,	getChanceOfGlitter,	NULL,	setChanceOfGlitter },	
 { field_strings[34][0],	field_strings[34][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[35][0],	field_strings[35][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)5000,	getHueTime,	NULL,	setHueTime },
 { field_strings[36][0],	field_strings[36][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getDeltaHue,	NULL,	setDeltaHue },	
 { field_strings[37][0],	field_strings[37][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 { field_strings[38][0],	field_strings[38][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getCooling,	NULL,	setCooling},
 { field_strings[39][0],	field_strings[39][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)255,	getSparking,	NULL,	setSparking },
 { field_strings[40][0],	field_strings[40][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)8,	getTwinkleSpeed,	NULL,	setTwinkleSpeed },
 { field_strings[41][0],	field_strings[41][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)8,	getTwinkleDensity,	NULL,	setTwinkleDensity },
 { field_strings[42][0],	field_strings[42][1],	NumberFieldType,	(uint16_t)0,	(uint16_t)100,	getDamping,	NULL,	setDamping },
 { field_strings[43][0],	field_strings[43][1],	NumberFieldType,	(uint16_t)1,	(uint16_t)60,	getSunRiseTime,	NULL,	setSunRiseTime },	
 { field_strings[44][0],	field_strings[44][1],	SectionFieldType,	NULL,	NULL,	NULL,	NULL,	NULL },
 #ifdef HAS_KNOB_CONTROL
 { field_strings[45][0],	field_strings[45][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getWiFiEnabled,	NULL,	setWiFiEnabled },
 #endif
 { field_strings[46][0],	field_strings[46][1],	NumberFieldType,		(uint16_t)100,	(uint16_t)DEFAULT_CURRENT_MAX,	getMilliamps,	NULL,	setMilliamps },
 // 111 max equals the minimum update time required for 300 pixels
 // this is the minimal delay being used anyway,	so no use in being faster
 { field_strings[47][0],	field_strings[47][1],	NumberFieldType,		(uint16_t)STRIP_MIN_FPS,	(uint16_t)(STRIP_MAX_FPS),	getFPSValue,	NULL,	setFPSValue },	
 { field_strings[48][0],	field_strings[48][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getDithering,	NULL,	setDithering },
 { field_strings[49][0],	field_strings[49][1],	BooleanFieldType,	(uint16_t)0,	(uint16_t)1,	getResetDefaults,	NULL,	setResetDefaults },

#ifdef DEBUG
 // With the DEBUG flag enabled we can provoke some resets (SOFT WDT,	HW WDT,	Exception...)
 { field_strings[50][0],	field_strings[50][1],	SectionFieldType },
 { field_strings[51][0],	field_strings[51][1],	SelectFieldType,		(uint16_t)0,	(uint16_t)5,	getReset,	getResets },
#endif
};
/*

    {"title",             LED_NAME,                                 TitleFieldType,     NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"powerSection",      "Power",                                  SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"power",             "On/Off",                            BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getPower,           NULL,           setPower      },
    {"isRunning",         "Pause",                        BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getIsRunning,       NULL,           setIsRunning  },
    {"basicControl",      "Basic",                          SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"br",                "Brightness",                             NumberFieldType,    (uint16_t)BRIGHTNESS_MIN,               (uint16_t)BRIGHTNESS_MAX,                         getBrightness,      NULL,           setBrightness },
    {"mo",                "Effect",                                 SelectFieldType,    (uint16_t)0,                            (uint16_t)strip->getModeCount(),                  getPattern,         getPatterns,    setPattern    },
    {"pa",                "Color palette",                          SelectFieldType,    (uint16_t)0,                            (uint16_t)(strip->getPalCount() + 1),             getPalette,         getPalettes,    setPalette    },
    {"sp",                "Speed",                         NumberFieldType,    (uint16_t)BEAT88_MIN,                   (uint16_t)BEAT88_MAX,                             getSpeed,           NULL,           setSpeed      },
    {"stripeStruture",    "Structrue",                   SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"segments",          "Segments",                               NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_SEGMENTS, 1),               getSegments,        NULL,           setSegments   },
    {"numBars",           "# LED bars",         NumberFieldType,    (uint16_t)1,                            (uint16_t)max(MAX_NUM_BARS, 1),                   getNumBars,     NULL, setNumBars                    },
    {"reverse",           "Reverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getReverse,         NULL,           setReverse    },
    {"mirror",            "Mirror",                                 BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getMirror,          NULL,           setMirror     },
  //{"inverse",           "Inverse",                                BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getInverse,         NULL,           setInverse    },
    {"autoplay",          "Autoplay",                          SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"autoplay",          "Auto mode change",                  SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutoplay,    getAutoplayModes, setAutoplayMode },
    {"autoplayDuration",  "Auto mode interval (s)",     NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutoplayDuration, NULL, setAutoplayDuration           },
  //  {"autopal",           "Color Palette Autoplay",                 SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"autopal",           "Auto palette change",         SelectFieldType,    (uint16_t)AUTO_MODE_OFF,                (uint16_t)AUTO_MODE_RANDOM,                       getAutopal,     getAutoplayModes, setAutopal },
    {"autopalDuration",   "Auto palette interval (s)",  NumberFieldType,    (uint16_t)5,                            (uint16_t)1000,                                   getAutopalDuration, NULL, setAutopalDuration            },
    {"BackGroundColor",   "Bcknd Color",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"BckndHue",          "Bcknd Hue",                         NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndHue,       NULL,           setBckndHue  },
    {"BckndSat",          "Bcknd Sat",                         NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBckndSat,       NULL,           setBckndSat  },
    {"BckndBri",          "Bcknd Bri",                         NumberFieldType,    (uint16_t)BCKND_MIN_BRI,                (uint16_t)BCKND_MAX_BRI,                          getBckndBri,       NULL,           setBckndBri  },
    
    {"advancedControl",   "Advanced",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"blendType",         "Color blend",                       SelectFieldType,    (uint16_t)NOBLEND,                      (uint16_t)LINEARBLEND,                            getBlendType,       getBlendTypes,  setBlendType  },
    {"ColorTemperature",  "Color temperature",                      SelectFieldType,    (uint16_t)0,                            (uint16_t)20,                                     getColorTemp,       getColorTemps,  setColorTemp  },
    {"LEDblur",           "Effect blur / blending",             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getBlurValue,       NULL,           setBlurValue  },
    {"solidColor",        "Solid color",                            SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"solidColor",        "Color",                                  ColorFieldType,     (uint16_t)0,                            (uint16_t)55,                                     getSolidColor,  NULL, NULL                 },
    {"glitter",           "Glitter",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,               NULL,           NULL          },
    {"addGlitter",        "Add Glitter",                            BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getAddGlitter,      NULL,           setAddGlitter                 },
    {"WhiteOnly",         "White Glitter",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWhiteOnly,       NULL,           setWhiteOnly                  },
    {"onBlackOnly",       "On Black",                          BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getOnBlackOnly,     NULL,           setOnBlackOnly                },
    {"glitterChance",     "Chance of Glitter",                      NumberFieldType,    (uint16_t)DEFAULT_GLITTER_CHANCE_MIN,   (uint16_t)DEFAULT_GLITTER_CHANCE_MAX,             getChanceOfGlitter, NULL,           setChanceOfGlitter            },     
    {"hue",               "Hue Change",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"huetime",           "Hue interval (ms)",               NumberFieldType,    (uint16_t)0,                            (uint16_t)5000,                                   getHueTime,     NULL, setHueTime                    },
    {"deltahue",          "Hue Offset",                             NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getDeltaHue,    NULL, setDeltaHue                   },    
    {"effectSettings",    "Effect Settings",                        SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"cooling",           "Cooling",                                NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getCooling,     NULL, setCooling},
    {"sparking",          "Sparking",                               NumberFieldType,    (uint16_t)0,                            (uint16_t)255,                                    getSparking,    NULL, setSparking                   },
    //{"twinkles",          "Twinkle settings",                       SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    {"twinkleSpeed",      "Twinkle speed",                          NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleSpeed, NULL, setTwinkleSpeed               },
    {"twinkleDensity",    "Twinkle density",                        NumberFieldType,    (uint16_t)0,                            (uint16_t)8,                                      getTwinkleDensity, NULL, setTwinkleDensity             },
    {"damping",           "damping for bounce",                     NumberFieldType,    (uint16_t)0,                            (uint16_t)100,                                    getDamping,   NULL, setDamping                    },
    // time provided in Minutes and capped at 60 minutes actually.
    {"sunriseset",        "sunrise, sunset time in min",     NumberFieldType,    (uint16_t)1,                            (uint16_t)60,                                     getSunRiseTime, NULL, setSunRiseTime                }, 
    {"otherSettings",      "Other settings",                        SectionFieldType,   NULL,                                   NULL,                                             NULL,           NULL,          NULL          },
    #ifdef HAS_KNOB_CONTROL
    {"wifiEnabled",        "WiFi On/Off",                           BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getWiFiEnabled, NULL, setWiFiEnabled         },
    #endif
    {"current",           "Current limit",                          NumberFieldType,    (uint16_t)100,                          (uint16_t)DEFAULT_CURRENT_MAX,                    getMilliamps, NULL, setMilliamps                  },
    // 111 max equals the minimum update time required for 300 pixels
    // this is the minimal delay being used anyway, so no use in being faster
    {"fps",               "max FPS",                NumberFieldType,    (uint16_t)STRIP_MIN_FPS,                (uint16_t)(STRIP_MAX_FPS),                        getFPSValue, NULL, setFPSValue                   },                                                                           
    {"dithering",         "Dithering",                              BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getDithering, NULL, setDithering                  },
    {"resetdefaults",     "Reset default",                   BooleanFieldType,   (uint16_t)0,                            (uint16_t)1,                                      getResetDefaults, NULL, setResetDefaults              },

#ifdef DEBUG
    // With the DEBUG flag enabled we can provoke some resets (SOFT WDT, HW WDT, Exception...)
    {"Debug",             "DEBUG only - not for production",        SectionFieldType                                                                                                        },
    {"resets",            "Resets (DEV Debug)",                     SelectFieldType,    (uint16_t)0,                            (uint16_t)5,                                      getReset,         getResets   },
#endif
};
*/

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