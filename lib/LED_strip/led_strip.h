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
#include "WS2812FX_FastLED.h"

// These define modes besides the fx library
#define FX_NO_FX        0
#define FX_SUNRISE      1
#define FX_SUNSET       2
#define FX_WS2812       3

extern WS2812FX *strip; 

// initialize the strip during boot (or at changes)
// strip can be any neopixel arrangement
// but is currently limited to NEO_GRB
void stripe_setup(  const uint16_t LEDCount, 
                    const uint8_t FPS, 
                    const uint8_t volt , 
                    const uint16_t milliamps , 
                    const CRGBPalette16 pal , 
                    const String Name ,
                    const LEDColorCorrection colc);

// resets the current effects and stops the strip
void reset(void);

// just calls the right effec routine according to the current Effect
void effectHandler(void);

// Sets a new Effect to be called
void setEffect(uint8_t Effect);

// returns the current Effect
uint8_t getEffect(void);

// return the previous effect
uint8_t getPreviousEffect(void);

// 32 Bit Color out of 3 Color values....
uint32_t strip_color32(uint8_t r, uint8_t g, uint8_t b);

// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color);

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color);

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color);

// helper for long delays (prevents watchdog reboot)
void delaymicro(unsigned int mics);

// set color for a single pixel (deactivates effects)
// but also sets the color for the effect library
void strip_setpixelcolor(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b);
void strip_setpixelcolor(uint16_t pixel, uint32_t color);

void set_Range(uint16_t start, uint16_t stop, uint8_t r, uint8_t g, uint8_t b);
void set_Range(uint16_t start, uint16_t stop, uint32_t color);

void stripe_setDelayInterval(uint16_t delay);

uint16_t stripe_getDelayInterval();

void strip_off(void);

void strip_On_Off(bool onOff);

void stripe_setBrightness(uint8_t brightness);

void mySunriseStart(uint32_t  mytime, uint16_t steps, bool up);

void mySunriseTrigger(void);

// ToDo: To be reqorked for the FastLED approach
typedef struct sunriseParam {
  bool isRunning;
  bool isSunrise;
  uint16_t steps;
  uint16_t step;
  uint32_t deltaTime;
  uint32_t lastChange;
} mysunriseParam;


extern bool sunrise_running;
extern bool stripWasOff;
extern bool stripIsOn;


//extern unsigned long last_delay_trigger;

extern uint8_t currentEffect;
extern uint8_t previousEffect;

extern mysunriseParam sunriseParam;

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


typedef String (*FieldSetter)(String);
typedef String (*FieldGetter)();



struct Field {
  String name;
  String label;
  String type;
  uint16_t min;
  uint16_t max;
  FieldGetter getValue;
  FieldGetter getOptions;
  FieldSetter setValue;
};

typedef Field FieldList[];


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

Field getField(String name, FieldList fields, uint8_t count);
String getFieldValue(String name, FieldList fields, uint8_t count);
String setFieldValue(String name, String value, FieldList fields, uint8_t count);
String getFieldsJson(FieldList fields, uint8_t count); 
String getPower(void);
String getMilliamps(void);
String getBrightness(void);
String getPattern(void);
String getPatterns(void);
String getPalette(void);
String getPalettes(void);
String getAutoplay(void); 
String getAutoplayDuration(void);
String getAutopal(void);
String getAutopalDuration(void);
String getSolidColor(void);
String getCooling(void);
String getSparking(void);
String getSpeed(void);
String getTwinkleSpeed(void);
String getTwinkleDensity(void);
String getBlendType(void);
String getBlendTypes(void);
String getReverse(void);
String getHueTime(void);
String getDeltaHue(void);

extern FieldList fields;
extern uint8_t fieldCount;

// /End Fields.h


#endif
