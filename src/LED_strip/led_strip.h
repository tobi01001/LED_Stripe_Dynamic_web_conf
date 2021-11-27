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
#define led_strip_h

// we gonna need our own adoption from the library. 
#include "../WS2812FX/WS2812FX_FastLED.h"
#include <ArduinoJson.h>

// These define modes besides the fx library
#define FX_NO_FX        0
#define FX_SUNRISE      1
#define FX_SUNSET       2
#define FX_WS2812       3

extern WS2812FX *strip; 

// initialize the strip during boot (or at changes)
// strip can be any neopixel arrangement
// but is currently limited to NEO_GRB
void stripe_setup(  CRGB * pleds, CRGB* eleds, const uint8_t volt ,
                    const LEDColorCorrection colc);

// Field.h
/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2016 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


typedef void (*FieldSetter)(uint32_t);
typedef uint32_t (*FieldGetter)();
typedef void (*FieldGetterOpts)(JsonArray & arr);

enum fieldtypes {
    NumberFieldType,
    BooleanFieldType,
    SelectFieldType,
    ColorFieldType,
    TitleFieldType,
    SectionFieldType,
    InvalidFieldType
  };


struct Field {
  const char* name;
  const char* label;
  //const char* type;
  fieldtypes type;
  uint16_t min;
  uint16_t max;
  FieldGetter getValue;
  FieldGetterOpts getOptions;
  FieldSetter setValue;
};


// /End Field.h

// Fields.h
/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2016 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

Field getField(const char * name);
String getFieldValue(const char * name);
void setFieldValue(const char * name, uint32_t value);
bool isField(const char * name);
const Field * getFields(void);
uint8_t getFieldCount(void);
// writes all Name / value pairs to the provided JSON Array
// "values": [
//    {
//       "name": "power",
//       "value": 0
//    },
//    {
//       "name": "effect",
//       "value": 0
//    },
//    {
//       "name": "brightness",
//       "value": 200
//    },
// returns true if at least on object was written
bool getAllValuesJSONArray(JsonArray &arr);

// writes all fields to the JSON obj as array.
// {
// "name": "power",
// "label": "On/Off",
// "type": 1,
// "value": 0
// },
// {
// "name": "effect",
// "label": "Effect",
// "type": 2,
// "value": 0,
// "options": [
// "Static",
// "Ease",
// returns true if at least on object was written
void getAllJSON(JsonArray &arr);
// writes all Name / value pairs to the provided JSON
// "power": "off",
// "effect": "Static",
// "brightness": "200"...
bool getAllValuesJSON(JsonObject & obj);

// /End Fields.h


#endif
