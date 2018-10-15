/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org

  Initially done by Harm Aldick - heavily adopted for personal use by tobi01001

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/

#include "WS2812FX.h"

uint16_t (*customMode)(void) = NULL;

/*
 * ColorPalettes
 */

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM = {  
  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
  CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
  CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green 
};

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM = {  
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
  Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM = {  
  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
  CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray 
};
// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM = {  
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
  CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray 
};

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM = {  
  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
  HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
  QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight 
};

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM = {  
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0xE0F0FF 
};

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM = {  
  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
  C9_Orange, C9_Red,    C9_Orange, C9_Red,
  C9_Green,  C9_Green,  C9_Green,  C9_Green,
  C9_Blue,   C9_Blue,   C9_Blue,
  C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM = {
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};

// Iced Colors
const TProgmemRGBPalette16 Ice_Colors_p FL_PROGMEM =  {
  CRGB::Black, CRGB::Black, CRGB::Blue,  CRGB::Blue,
  CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Aqua,
  CRGB::Aqua,  CRGB::Aqua,  CRGB::Aqua,  CRGB::Aqua,
  CRGB::Aqua,  CRGB::White, CRGB::White, CRGB::White
};

// Totally Black palette (for fade through black transitions)
const TProgmemRGBPalette16 Total_Black_p FL_PROGMEM = {
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
  CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
};

// Shades
#define SHADE01 0xF0
#define SHADE02 0x80
#define SHADE03 0x40
#define SHADE04 0x20
#define SHADE05 0x10
// Values
#define REDVAL(A)   ((A << 16)& 0xff0000)
#define GREENVAL(A) ((A <<  8)& 0x00ff00)
#define BLUEVAL(A)  ((A <<  0)& 0x0000ff)

// Shades of Red
const TProgmemRGBPalette16 Shades_Of_Red_p FL_PROGMEM = {
  REDVAL(SHADE01), REDVAL(SHADE02), REDVAL(SHADE03), REDVAL(SHADE04),
  REDVAL(SHADE05), CRGB::Black,     CRGB::Black,     REDVAL(SHADE04),
  REDVAL(SHADE03), REDVAL(SHADE02), REDVAL(SHADE01), CRGB::Black,
  CRGB::Black,     REDVAL(SHADE02), REDVAL(SHADE03), CRGB::Black
};

// Shades of Green
const TProgmemRGBPalette16 Shades_Of_Green_p FL_PROGMEM = {
  GREENVAL(SHADE01), GREENVAL(SHADE02), GREENVAL(SHADE03), GREENVAL(SHADE04),
  GREENVAL(SHADE05), CRGB::Black,       CRGB::Black,       GREENVAL(SHADE04),
  GREENVAL(SHADE03), GREENVAL(SHADE02), GREENVAL(SHADE01), CRGB::Black,
  CRGB::Black,       GREENVAL(SHADE02), GREENVAL(SHADE03), CRGB::Black
};

// Shades of Blue
const TProgmemRGBPalette16 Shades_Of_Blue_p FL_PROGMEM = {
  BLUEVAL(SHADE01), BLUEVAL(SHADE02), BLUEVAL(SHADE03), BLUEVAL(SHADE04),
  BLUEVAL(SHADE05), CRGB::Black,      CRGB::Black,      BLUEVAL(SHADE04),
  BLUEVAL(SHADE03), BLUEVAL(SHADE02), BLUEVAL(SHADE01), CRGB::Black,
  CRGB::Black,      BLUEVAL(SHADE02), BLUEVAL(SHADE03), CRGB::Black
};



/*
 * <Begin> Service routines
 */

// Not much to be initialized...
void WS2812FX::init() {
  RESET_RUNTIME;            // this should be the only occurrence of RESET_RUNTIME now... 
  fill_solid(_bleds, LED_COUNT, CRGB::Black);
  fill_solid(  leds, LED_COUNT, CRGB::Black);
  FastLED.clear(true);      // During init, all pixels should be black.
  FastLED.show();           // We show once to write the Led data.
}

/*
 * the overall service task. To be called as often as possible / useful
 * (at least at the desired frame rate)
 * --> see STRIP_MAX_FPS
 */
void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    
    if(now > SEGMENT_RUNTIME.next_time || _triggered) {
      uint16_t delay = (this->*_mode[SEGMENT.mode])();
      SEGMENT_RUNTIME.next_time = now + (int)delay; //STRIP_MIN_DELAY;
    }
    // check if we fade to a new FX mode.
    #define MAXINVERSE 32
    if(_transition)
    {
      EVERY_N_MILLISECONDS(8)
      {
        CRGB tmp = CRGB::Black;
        if(!SEGMENT.reverse) {
          for(uint16_t i=0; i < LED_COUNT; i++) {
            if(!SEGMENT.inverse) {
              nblend(_bleds[i],  leds[i], _blend);
            } else {
              tmp.r = qsub8(MAXINVERSE, leds[i].r);
              tmp.g = qsub8(MAXINVERSE, leds[i].g);
              tmp.b = qsub8(MAXINVERSE, leds[i].b);
              nblend(_bleds[i], tmp, _blend);
            }
          }
        } else {
          for(uint16_t i=0; i < LED_COUNT; i++) {
            if(!SEGMENT.inverse) {
              nblend(_bleds[i],  leds[LED_COUNT-1-i], _blend);
            } else {
              tmp.r = qsub8(MAXINVERSE, leds[i].r);
              tmp.g = qsub8(MAXINVERSE, leds[i].g);
              tmp.b = qsub8(MAXINVERSE, leds[i].b);
              nblend(_bleds[i], tmp, _blend);
              //nblend(_bleds[i], -leds[LED_COUNT-1-i], _blend);
            }
          }
        }
        _blend = qadd8(_blend, 1);
      }
      if(_blend == 255)
      {
        _transition = false;
        _blend = 0;
      }
    }
    else
    {
      EVERY_N_MILLISECONDS(10)
      {
        fadeToBlackBy(_bleds, LED_COUNT, 1);
      }
      CRGB tmp = CRGB::Black;
      if(!SEGMENT.reverse) {
        
        for(uint16_t i=0; i < LED_COUNT; i++) {
          if(!SEGMENT.inverse) {
            nblend(_bleds[i],  leds[i], _segment.blur);
          } else {
            tmp.r = qsub8(MAXINVERSE, leds[i].r);
            tmp.g = qsub8(MAXINVERSE, leds[i].g);
            tmp.b = qsub8(MAXINVERSE, leds[i].b);
            nblend(_bleds[i], tmp, _segment.blur);
            //nblend(_bleds[i], -leds[i], _segment.blur);
          }
        }
      } else {
        for(uint16_t i=0; i < LED_COUNT; i++) {
          if(!SEGMENT.inverse) {
            nblend(_bleds[i],  leds[LED_COUNT-1-i], _segment.blur);
          } else {
            tmp.r = qsub8(MAXINVERSE, leds[i].r);
            tmp.g = qsub8(MAXINVERSE, leds[i].g);
            tmp.b = qsub8(MAXINVERSE, leds[i].b);
            nblend(_bleds[i], tmp, _segment.blur);
            //nblend(_bleds[i], -leds[LED_COUNT-1-i], _segment.blur);
          }
        }
      }
    }
    // Write the data
    FastLED.show();


    // Every huetime we increase the baseHue by the respective deltaHue.
    // set deltahue to 0, to turn this off.
    //EVERY_N_MILLISECONDS(SEGMENT.hueTime)
    if(now > SEGMENT_RUNTIME.nextHue)
    {
      if(!SEGMENT.hueTime)
      {
        SEGMENT_RUNTIME.baseHue = SEGMENT.deltaHue;
      }
      else
      {
        SEGMENT_RUNTIME.baseHue++; // += SEGMENT.deltaHue;
      }
      SEGMENT_RUNTIME.nextHue = now + SEGMENT.hueTime;
    }

    // Palette fading / blending
    EVERY_N_MILLISECONDS(16) { // Blend towards the target palette
      nblendPaletteTowardPalette(_currentPalette, _targetPalette, 16);
      if(_currentPalette == _targetPalette)
      {
        _currentPaletteName = _targetPaletteName;
      }
    }

    // Autoplay
    //EVERY_N_SECONDS(SEGMENT.autoplayDuration)
    if(now > SEGMENT_RUNTIME.nextAuto)
    {
      if(SEGMENT.autoplay && !_transition)
      {
        if(SEGMENT.mode == (getModeCount()-1))
        {
          setMode(0);
        }
        else
        {
          setMode(SEGMENT.mode+1);
        }
        SEGMENT_RUNTIME.nextAuto = now + SEGMENT.autoplayDuration*1000;
      }
    }

    if(now > SEGMENT_RUNTIME.nextPalette)
    {
      if(SEGMENT.autoPal && !_transition)
      {
        if(getTargetPaletteNumber() >= getPalCount()-1)
        {
          setTargetPalette(0);
        }
        else
        {
          setTargetPalette(getTargetPaletteNumber()+1);
        }
        SEGMENT_RUNTIME.nextPalette = now + SEGMENT.autoPalDuration*1000;
      }       
    }

    // reset trigger...
    _triggered = false;
  }
}

void WS2812FX::start() {
  _running = true;
}

void WS2812FX::stop() {
  _running = false;
  strip_off();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::show() {
  nblend(_bleds, leds, LED_COUNT, SEGMENT.blur);
  FastLED.show();
}

/*
 * <End> Service routines
 */


/* 
 * <Begin> Helper Functions
 */


/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit triwave
 */
inline uint16_t WS2812FX::triwave16(uint16_t in) {
  if(in & 0x8000)
  {
    in = 65535 - in;
  }
  return in << 1;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit quadwave
 */
inline uint16_t WS2812FX::quadwave16(uint16_t in) {
   return ease16InOutQuad( triwave16( in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutQuad
 */
inline uint16_t WS2812FX::ease16InOutQuad( uint16_t i) {
    uint16_t j = i;
    if( j & 0x8000 ) {
        j = 65535 - j;
    }
    uint16_t jj  = scale16(  j, j);
    uint16_t jj2 = jj << 1;
    if( i & 0x8000 ) {
        jj2 = 65535 - jj2;
    }
    return jj2;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit cubicWave
 */
inline uint16_t WS2812FX::cubicwave16(uint16_t in) {
    return ease16InOutCubic( triwave16( in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutCubic
 */
inline uint16_t WS2812FX::ease16InOutCubic( uint16_t i) {
    
    uint16_t ii  = scale16(  i, i);
    uint16_t iii = scale16( ii, i);

    uint32_t r1 = (3 * (uint16_t)(ii)) - ( 2 * (uint16_t)(iii));

    uint16_t result = r1;

    // if we got "65536", return 65535:
    if( r1 & 0x10000) {
        result = 65535;
    }
    return result;
}


void WS2812FX::setColorTemperature(uint8_t index) {
  switch (index)
  {
    case 0:  SEGMENT.colorTemp = Candle;
    break;  
    case 1:  SEGMENT.colorTemp = Tungsten40W;
    break;
    case 2:  SEGMENT.colorTemp = Tungsten100W;
    break;
    case 3:  SEGMENT.colorTemp = Halogen;
    break;
    case 4:  SEGMENT.colorTemp = CarbonArc;
    break;
    case 5:  SEGMENT.colorTemp = HighNoonSun;
    break;
    case 6:  SEGMENT.colorTemp = DirectSunlight;
    break;
    case 7:  SEGMENT.colorTemp = OvercastSky;
    break;
    case 8:  SEGMENT.colorTemp = ClearBlueSky;
    break;
    case 9:  SEGMENT.colorTemp = WarmFluorescent;
    break;
    case 10: SEGMENT.colorTemp = StandardFluorescent;
    break;
    case 11: SEGMENT.colorTemp = CoolWhiteFluorescent;
    break;
    case 12: SEGMENT.colorTemp = FullSpectrumFluorescent;
    break;
    case 13: SEGMENT.colorTemp = GrowLightFluorescent;
    break;
    case 14: SEGMENT.colorTemp = BlackLightFluorescent;
    break;
    case 15: SEGMENT.colorTemp = MercuryVapor;
    break;
    case 16: SEGMENT.colorTemp = SodiumVapor;
    break;
    case 17: SEGMENT.colorTemp = MetalHalide;
    break;
    case 18: SEGMENT.colorTemp = HighPressureSodium;
    break;
    default: SEGMENT.colorTemp = UncorrectedTemperature;
    break;
  }
  FastLED.setTemperature(SEGMENT.colorTemp);
}

uint8_t WS2812FX::getColorTemp(void) {
  switch (SEGMENT.colorTemp)
  {
    case Candle:		              return 0;
    case Tungsten40W:		          return 1;
    case Tungsten100W:            return 2;
    case Halogen:                 return 3;
    case CarbonArc:               return 4;
    case HighNoonSun:             return 5;
    case DirectSunlight:          return 6;
    case OvercastSky:             return 7;
    case ClearBlueSky:            return 8;
    case WarmFluorescent:         return 9;
    case StandardFluorescent:     return 10;
    case CoolWhiteFluorescent:    return 11;
    case FullSpectrumFluorescent: return 12;
    case GrowLightFluorescent:    return 13;
    case BlackLightFluorescent:   return 14;
    case MercuryVapor:            return 15;
    case SodiumVapor:             return 16;
    case MetalHalide:             return 17;
    case HighPressureSodium:      return 18;
    
    default:                      return 19;
    break;
  }  
}

String WS2812FX::getColorTempName(uint8_t index) {
  String names[] = {  
                    "Candle",
                    "Tungsten40W",
                    "Tungsten100W",
                    "Halogen",
                    "CarbonArc",
                    "HighNoonSun",
                    "DirectSunlight",
                    "OvercastSky",
                    "ClearBlueSky",
                    "WarmFluorescent",
                    "StandardFluorescent",
                    "CoolWhiteFluorescent",
                    "FullSpectrumFluorescent",
                    "GrowLightFluorescent",
                    "BlackLightFluorescent",
                    "MercuryVapor",
                    "SodiumVapor",
                    "MetalHalide",
                    "HighPressureSodium",
                    "UncorrectedTemperature" };
  return names[index];
}


  /* Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
   * sixteenths of a pixel from the start of the strip.  Fractional positions are
   * rendered using 'anti-aliasing' of pixel brightness.
   * The bar width is specified in whole pixels.
   * Arguably, this is the interesting code. 
   */
void WS2812FX::drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, uint8_t cindex, uint8_t max_bright = 255, bool mixColors=true) {

  int i = pos16 / 16; // convert from pos to raw pixel number
  
  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
 
  // brightness of the first pixel in the bar is 1.0 - (fractional part of position)
  // e.g., if the light bar starts drawing at pixel "57.9", then
  // pixel #57 should only be lit at 10% brightness, because only 1/10th of it
  // is "in" the light bar:
  //
  //                       57.9 . . . . . . . . . . . . . . . . . 61.9
  //                        v                                      v
  //  ---+---56----+---57----+---58----+---59----+---60----+---61----+---62---->
  //     |         |        X|XXXXXXXXX|XXXXXXXXX|XXXXXXXXX|XXXXXXXX |  
  //  ---+---------+---------+---------+---------+---------+---------+--------->
  //                   10%       100%      100%      100%      90%        
  //
  // the fraction we get is in 64ths. We subtract from 255 because we want a high
  // fraction (e.g. 0.9) to turn into a low brightness (e.g. 0.1)
  uint8_t firstpixelbrightness = 255 - (frac*16);//map8(15 - (frac), 0, max_bright);
 
  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness  = 255 - firstpixelbrightness; //map8(15 - firstpixelbrightness, 0, max_bright);
 
  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
  uint8_t bright;
  bool mix = true;
  for( int n = 0; n <= width; n++) {
    if(n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    } else if( n == width ) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    } else {
      // middle pixels
      bright = max_bright;
      mix = false;
    }
 
    CRGB newColor;
    if(i<=SEGMENT.stop && i >= SEGMENT.start)
    {
      if(mixColors || mix)
      {
        newColor = leds[i] | ColorFromPalette(pal, cindex, bright, SEGMENT.blendType); 
         // we blend based on the "baseBeat"
        nblend(leds[i], newColor, qadd8(SEGMENT.beat88>>8, 24));
      }
      else
      {
        leds[i] = ColorFromPalette(pal, cindex, max_bright, SEGMENT.blendType); 
      }
    }
    i++;
  }
}

/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos, uint8_t dist = 42) {
  
  return (pos + dist + random(255-dist));
}

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off() {
  _running = false;
  FastLED.clear();
}

/*
 * Add sparks
 */
void WS2812FX::addSparks(uint8_t probability = 10, bool onBlackOnly = true, bool white = false) {
  if(random8(probability) != 0) return;

  uint16_t pos = random16(SEGMENT.start, SEGMENT.stop); // Pick an LED at random.
  
  if(leds[pos] && onBlackOnly) return;

  if(white)
  {
    leds[pos] += CRGB(0xffffff);
  }
  else
  {
    leds[pos] += ColorFromPalette(_currentPalette, random8(SEGMENT_RUNTIME.baseHue, (uint8_t)(SEGMENT_RUNTIME.baseHue+128)), random8(92,255), SEGMENT.blendType);
  }
  return;
}

void WS2812FX::map_pixels_palette(uint8_t *hues, uint8_t bright = 255, TBlendType blend = LINEARBLEND) { 
  for(uint16_t i = 0; i<LED_COUNT; i++)
  {
    leds[i + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[i], bright, blend);
  }
  return;
}


CRGB WS2812FX::computeOneTwinkle( uint32_t ms, uint8_t salt) {
  //  This function takes a time in pseudo-milliseconds,
  //  figures out brightness = f( time ), and also hue = f( time )
  //  The 'low digits' of the millisecond time are used as 
  //  input to the brightness wave function.  
  //  The 'high digits' are used to select a color, so that the color
  //  does not change over the course of the fade-in, fade-out
  //  of one cycle of the brightness wave function.
  //  The 'high digits' are also used to determine whether this pixel
  //  should light at all during this cycle, based on the TWINKLE_DENSITY.
  //  uint8_t TWINKLE_SPEED = _twinkleSpeed; //map8(SEGMENT.beat88>>8, 2, 8);
  //  Overall twinkle density.
  //  0 (NONE lit) to 8 (ALL lit at once).  
  //  Default is 5.
  //  #define TWINKLE_DENSITY _twinkleDensity //6

  uint16_t ticks = ms >> (8-SEGMENT.twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < SEGMENT.twinkleDensity) {
    bright = attackDecayWave8( fastcycle8);
  }

  #define COOL_LIKE_INCANDESCENT 0

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( _currentPalette, hue, bright, SEGMENT.blendType);
    if( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


uint8_t WS2812FX::attackDecayWave8( uint8_t i) {
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}


void WS2812FX::coolLikeIncandescent( CRGB& c, uint8_t phase) {
  /* 
  This function is like 'triwave8', which produces a 
  symmetrical up-and-down triangle sawtooth waveform, except that this
  function produces a triangle wave with a faster attack and a slower decay:
  
      / \ 
     /     \ 
    /         \ 
   /             \ 
  
  This function takes a pixel, and if its in the 'fading down'
  part of the cycle, it adjusts the color a little bit like the 
  way that incandescent bulbs fade toward 'red' as they dim. */
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}
/*
 * End Twinklle Fox
 */

uint16_t WS2812FX::pride(bool glitter = false) {
  if(SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.b16.p.sPseudotime = 0;
    SEGMENT_RUNTIME.b16.p.sLastMillis = 0;
    SEGMENT_RUNTIME.b16.p.sHue16 = 0;
  }
 

  uint8_t brightdepth = beatsin88( SEGMENT.beat88/3 + 1, 96, 224); 
  uint16_t brightnessthetainc16 = beatsin88( SEGMENT.beat88/5+1, (25 * 256), (40 * 256)); 
  uint8_t msmultiplier = beatsin88(SEGMENT.beat88/7+1, 23, 60);

  uint16_t hue16 = SEGMENT_RUNTIME.b16.p.sHue16;
  uint16_t hueinc16 = beatsin88(SEGMENT.beat88/9+1, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - SEGMENT_RUNTIME.b16.p.sLastMillis ;
  SEGMENT_RUNTIME.b16.p.sLastMillis  = ms;
  SEGMENT_RUNTIME.b16.p.sPseudotime += deltams * msmultiplier;
  SEGMENT_RUNTIME.b16.p.sHue16 += deltams * beatsin88( (SEGMENT.beat88/5)*2+1, 5, 9);
  uint16_t brightnesstheta16 = SEGMENT_RUNTIME.b16.p.sPseudotime;

  for ( uint16_t i = 0 ; i < LED_COUNT; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = ColorFromPalette(_currentPalette, hue8, bri8, SEGMENT.blendType); //CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (SEGMENT.stop) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }

  if(!glitter) return STRIP_MIN_DELAY;

  addSparks(10, false, true);

  return STRIP_MIN_DELAY;

}

/*
 * fade out function
 * fades out the current segment by dividing each pixel's intensity by 2
 */
void WS2812FX::fade_out(uint8_t fadeB = 32) {
  fadeToBlackBy(&leds[SEGMENT.start], LED_COUNT, fadeB);
}

/* 
 * <End> Helper Functions
 */


/* 
 * <Begin> User Interface Functions (setables and getables)
 */

/*
 * Lets us set the Blend type (No blend or Linear blend).
 * This affects most effects.
 */
void WS2812FX::setBlendType(TBlendType t = LINEARBLEND) {
  SEGMENT.blendType = t;
}

/*
 * Lets us toggle the Blend type
 */
void WS2812FX::toggleBlendType(void){
  SEGMENT.blendType == NOBLEND ? SEGMENT.blendType = LINEARBLEND : SEGMENT.blendType = NOBLEND;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 */
void WS2812FX::setCurrentPalette(CRGBPalette16 p, String Name = "Custom") { 
  _currentPalette = p;
  _currentPaletteName = Name;
  _currentPaletteNum = NUM_PALETTES;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setCurrentPalette(uint8_t n=0) { 
  _currentPalette = *(_palettes[n % NUM_PALETTES]);
  _currentPaletteName = _pal_name[n % NUM_PALETTES];
  _currentPaletteNum = n % NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * p: the Palette
 * Name: The name
 */
void WS2812FX::setTargetPalette(CRGBPalette16 p, String Name = "Custom") { 
  for(uint8_t i = 0; i< NUM_PALETTES; i++)
  {
    String tName = getPalName(i);
    if(tName == Name)
    {
      setTargetPalette(i);
      return;
    }
  }
  _targetPalette = p;
  _targetPaletteName = Name;
  _targetPaletteNum = NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setTargetPalette(uint8_t n=0) {
  _targetPalette = *(_palettes[n % NUM_PALETTES]);
  _targetPaletteName = _pal_name[n % NUM_PALETTES];
  _targetPaletteNum = n % NUM_PALETTES;
}

/*
 * Change to the mode being provided
 * m: mode number
 */ 
void WS2812FX::setMode(uint8_t m) {
  
  if(m == SEGMENT.mode) return;  // not really a new mode...
  
  // make sure its a valid mode
  SEGMENT.mode = constrain(m, 0, MODE_COUNT - 1);
  if(!_transition)
  {
    // if we are not currently in a transition phase
    // we clear the led array (the one holding the effect
    // the real LEDs are drawn from _bleds and blended to the leds)
    fill_solid(leds, LED_COUNT, CRGB::Black);
  }
  // start the transition phase
  _transition = true;
  _blend = 0;
  SEGMENT_RUNTIME.modeinit = true;
  setBlurValue(_pblur);

  //setBrightness(_brightness);
}

void WS2812FX::setSpeed(uint16_t s) {
  //This - now actually sets a "beat"
  SEGMENT.beat88 = constrain(s, BEAT88_MIN, BEAT88_MAX);
  SEGMENT_RUNTIME.tb.timebase = millis();
}

void WS2812FX::increaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.beat88 + s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.beat88 - s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(CRGBPalette16(((uint32_t)r << 16) | ((uint32_t)g << 8) | b));
}

void WS2812FX::setColor(CRGBPalette16 c) {
  //SEGMENT.cPalette = c;
  setTargetPalette(c);  
}

void WS2812FX::setColor(uint32_t c) {
 
  setColor(CRGBPalette16(c));
  setBrightness(_brightness);
}

void WS2812FX::setBrightness(uint8_t b) {
  _brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
 
  FastLED.setBrightness(_brightness);
  FastLED.show();
}

void WS2812FX::increaseBrightness(uint8_t s) {
  s = constrain(_brightness + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(uint8_t s) {
  s = constrain(_brightness - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::setLength(uint16_t b) {}

void WS2812FX::increaseLength(uint16_t s) {}

void WS2812FX::decreaseLength(uint16_t s) {}

boolean WS2812FX::isRunning() {
  return _running;
}

uint8_t WS2812FX::getMode(void) {
  if(_new_mode != 255) {
    return _new_mode;
  }
  else
  {
    return SEGMENT.mode;
  }
}

uint16_t WS2812FX::getBeat88(void) {
  return SEGMENT.beat88;
}

uint8_t WS2812FX::getBrightness(void) {
  return _brightness;
}

uint16_t WS2812FX::getLength(void) {
  return SEGMENT.stop - SEGMENT.start + 1;
}

uint8_t WS2812FX::getModeCount(void) {
  return MODE_COUNT;
}

uint8_t WS2812FX::getPalCount(void) {
  return NUM_PALETTES;
}

uint32_t WS2812FX::getColor(uint8_t p_index = 0) {
  return ColorFromPalette(_currentPalette, p_index);
}

const __FlashStringHelper* WS2812FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _name[m];
  } else {
    return F("");
  }
}

const __FlashStringHelper* WS2812FX::getPalName(uint8_t p) {
  if(p < NUM_PALETTES) {
    return _pal_name[p];
  } else {
    return F("");
  }
}

/*
 * Custom mode helper
 */
void WS2812FX::setCustomMode(uint16_t (*p)()) {
  setMode(FX_MODE_CUSTOM);
  customMode = p;
}

/* 
 * <End> User Interface Functions (setables and getables)
 */


/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */


/*
 * The "Off" mode clears the Leds.
 *
uint16_t WS2812FX::mode_off(void) {
  FastLED.clear(true);
  return 1000;
}
*/

/*
 * No blinking. Just plain old static light - but mapped on a color palette.
 * Palette ca be "moved" by SEGMENT.baseHue
 * will distribute the palette over the display length
 */
uint16_t WS2812FX::mode_static(void) {
  
  fill_palette(&leds[SEGMENT.start], LED_COUNT, SEGMENT_RUNTIME.baseHue, (LED_COUNT > 255 ? 1 : (255 / LED_COUNT) + 1), _currentPalette, _brightness, SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 */
uint16_t WS2812FX::mode_ease(void) {
  return this->mode_ease_func(false);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles will be additionally applied.
 */
uint16_t WS2812FX::mode_twinkle_ease(void) {
  return this->mode_ease_func(true);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles can additionally applied.
 */
uint16_t WS2812FX::mode_ease_func(bool sparks = true) {
  // number of pixels for "antialised" (fractional) bar
  const uint8_t width = 1;
  // pixel position on the strip we make two out of it...
  uint16_t lerpVal    = 0;

  if(SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    // need to know if we are in the middle (to smoothly update random beat)
    SEGMENT_RUNTIME.nb.trigger = false;
    // beat being modified during runtime
    SEGMENT_RUNTIME.b16.e.beat = SEGMENT.beat88;
    // to check if beat88 recently changed
    // ToDo (idea) maybe a global runtime flag could help 
    // which is recent by the active effect making use of the "beat"
    SEGMENT_RUNTIME.b16.e.oldbeat = SEGMENT.beat88;
    // to check if we have movement.
    // maybe easier but works good for now.
    SEGMENT_RUNTIME.b16.e.p_lerp = lerpVal;
  }
  // instead of moving the color around (palette wise)
  // we set it to the baseHue. So it can still be changed 
  // and also change over time
  uint8_t colorMove = SEGMENT_RUNTIME.baseHue;  //= quadwave8(map(beat88(max(SEGMENT.beat88/2,1),SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  // this is the fading tail....
  // we adjust it a bit on the speed (beat)
  fade_out(SEGMENT.beat88 >> 5);

  // now e calculate a sine curve for the led position
  // factor 16 is used for the fractional bar
  lerpVal = beatsin88(SEGMENT_RUNTIME.b16.e.beat, SEGMENT.start*16, SEGMENT.stop*16-(width*16), SEGMENT_RUNTIME.tb.timebase);
  
  // once we are in the middle
  // we can modify the speed a bit
  if(lerpVal == ((LED_COUNT*16)/2))
  {
    // the trigger is used because we are more frames in the middle 
    // but only one should trigger
    if(SEGMENT_RUNTIME.nb.trigger)
    {
      // if the changed the base speed (external source)
      // we refesh the values
      if(SEGMENT_RUNTIME.b16.e.oldbeat != SEGMENT.beat88)
      {
        SEGMENT_RUNTIME.b16.e.beat = SEGMENT.beat88;
        SEGMENT_RUNTIME.b16.e.oldbeat = SEGMENT.beat88;
        //SEGMENT_RUNTIME.tb.timebase = millis();
      }
      // reset the trigger
      SEGMENT_RUNTIME.nb.trigger = false;
      // tiimebase starts fresh in the middle (avoid jumping pixels)
      SEGMENT_RUNTIME.tb.timebase = millis();
      // we randomly increase or decrease
      // as we work with unsigned values we do this with an offset...
      // smallest value should be 255
      if(SEGMENT_RUNTIME.b16.e.beat < 255)
      {
        // avoid roll over to 65535
        SEGMENT_RUNTIME.b16.e.beat += 2 * random8();
      }
      else
      {
        // randomly increase or decrease beat
        SEGMENT_RUNTIME.b16.e.beat += 2 * (128 - random8());
      }
      
    }
  }
  else
  {
    // activate trigger if we are moving
    if(lerpVal != SEGMENT_RUNTIME.b16.e.p_lerp) SEGMENT_RUNTIME.nb.trigger = true;
  }

  SEGMENT_RUNTIME.b16.e.p_lerp = lerpVal;
  // we draw two fractional bars here. for the color mapping we need the overflow and therefore cast to uint8_t
  drawFractionalBar(lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal/16-SEGMENT.start) + colorMove), _brightness);
  drawFractionalBar((SEGMENT.stop*16)-lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal/16-SEGMENT.start) + colorMove), _brightness);

  if(sparks) addSparks(10, true, false);

  return STRIP_MIN_DELAY;
}

// moves a fractional bar along the stip based on noise
uint16_t WS2812FX::mode_inoise8_mover(void) {
  return this->mode_inoise8_mover_func(false);
}

// moves a fractional bar along the stip based on noise
// random twinkles are added
uint16_t WS2812FX::mode_inoise8_mover_twinkle(void) {
  return this->mode_inoise8_mover_func(true);
}

uint16_t WS2812FX::mode_inoise8_mover_func(bool sparks) {
  uint16_t xscale = LED_COUNT; //30;                                         
  uint16_t yscale = 30;
  const uint16_t width = 6; //max(SEGMENT.beat88/256,1);
  if(SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.b16.dist = 1234;
  }

  
  uint8_t locn = inoise8(xscale, SEGMENT_RUNTIME.b16.dist + yscale);       
  uint16_t pixlen = map(locn,0,255,SEGMENT.start*16, SEGMENT.stop*16-width*16);
  
  uint8_t colormove = SEGMENT_RUNTIME.baseHue; // quadwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  fade_out(48);
  
  drawFractionalBar(pixlen, width, _currentPalette, (uint8_t)((uint8_t)(pixlen / 64) + colormove)); //, beatsin88(max(SEGMENT.beat88/2,1),200 % _brightness, _brightness, SEGMENT_RUNTIME.tb.timebase));
  
  SEGMENT_RUNTIME.b16.dist += beatsin88(SEGMENT.beat88,1,6, SEGMENT_RUNTIME.tb.timebase);  

  if(sparks) addSparks(10, true, false);

  return STRIP_MIN_DELAY;
}

/*
 * Plasma like Effect over the complete strip.
 */
uint16_t WS2812FX::mode_plasma(void) {
  uint8_t thisPhase = beatsin88(SEGMENT.beat88, 0, 255, SEGMENT_RUNTIME.tb.timebase);                           // Setting phase change for a couple of waves.
  uint8_t thatPhase = beatsin88((SEGMENT.beat88*11)/10, 0, 255, SEGMENT_RUNTIME.tb.timebase); // was int thatPhase = 64 - beatsin88((SEGMENT.beat88*11)/10, 0, 128, SEGMENT_RUNTIME.tb.timebase);

  for (int k=SEGMENT.start; k<SEGMENT.stop; k++) {                              // For each of the LED's in the strand, set a brightness based on a wave as follows:

    uint8_t colorIndex = cubicwave8((k*15)+thisPhase)/2 + cos8((k*8)+thatPhase)/2 + SEGMENT_RUNTIME.baseHue;           // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsuba(colorIndex, beatsin88((SEGMENT.beat88*12)/10,0,128));             // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
    CRGB newColor = ColorFromPalette(_currentPalette, colorIndex, thisBright, SEGMENT.blendType);  // Let's now add the foreground colour.
    leds[k] = nblend(leds[k], newColor, 64);
  }
  return STRIP_MIN_DELAY;
}

/*
 * Move 3 dots / small bars (antialised) at different speeds
 */
uint16_t WS2812FX::mode_juggle_pal(void) {
  //const uint8_t numdots = 3;
  
  const uint8_t width = max(LED_COUNT/15,2);
  uint8_t curhue = 0;
  if(SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.co.thishue = 0;
  }
  curhue = SEGMENT_RUNTIME.co.thishue; // Reset the hue values.
  EVERY_N_MILLISECONDS(100)
  {
    SEGMENT_RUNTIME.co.thishue = random8(curhue, qadd8(curhue,8));
  }

  fade_out(96);
  
  for( int i = 0; i < SEGMENT.numBars; i++) {
    uint16_t pos = beatsin88(max(SEGMENT.beat88/2,1)+i*256+762,SEGMENT.start*16, SEGMENT.stop*16-width*16, SEGMENT_RUNTIME.tb.timebase);
    drawFractionalBar(pos, width, _currentPalette, curhue+(255/SEGMENT.numBars)*i, _brightness);
    uint8_t delta = random8(9);
    if(delta < 5)
    {
      curhue = curhue - (uint8_t)(delta)  + SEGMENT_RUNTIME.baseHue;
    }
    else
    {
      curhue = curhue + (uint8_t)(delta/2) + SEGMENT_RUNTIME.baseHue;
    }
    
  }
  return STRIP_MIN_DELAY;
}

/*
 * Confetti, yeah
 */
/*
uint16_t WS2812FX::mode_confetti(void) {

  EVERY_N_MILLIS(50)
  {
    fade_out(8);
  }
  
  
  if(random8(3) != 0) return STRIP_MIN_DELAY;

  uint16_t pos;
  uint8_t index = (uint8_t)beatsin88(SEGMENT.beat88, 0, 255, SEGMENT_RUNTIME.tb.timebase) + SEGMENT_RUNTIME.baseHue;
  uint8_t bright = random8(192 % _brightness, _brightness);
  const uint8_t space = 1;
  bool newSpark = true;
  
  pos = random16((SEGMENT.start + 1)*16, (SEGMENT.stop - 2)*16-32);   
  for(int_fast8_t i = 0 - space; i<=space; i++)
  {
    if((pos/16+i) >=SEGMENT.start && (pos/16+i) < SEGMENT.stop)
    {
      if(leds[(pos/16+i)]) newSpark = false;
    }
  }

  if(!newSpark) return STRIP_MIN_DELAY;

  drawFractionalBar(pos, 1, _currentPalette, index, bright);
  
  return STRIP_MIN_DELAY;
}
*/

/*
 * Fills the strip with waving color and brightness
 */
uint16_t WS2812FX::mode_fill_beat(void) {
  CRGB newColor = CRGB::Black;
  uint8_t br, index;
  for(uint8_t k=SEGMENT.start; k<SEGMENT.stop; k++)
  {
    
    br = beatsin88(SEGMENT.beat88, _brightness/10, _brightness, SEGMENT_RUNTIME.tb.timebase, k*2);//= quadwave8(v1);
    index = (uint8_t)((uint8_t)triwave8(beat8(SEGMENT.beat88>>8) + 
                      (uint8_t)beatsin8(SEGMENT.beat88>>8,0,20) + 
                      (uint8_t)map(k, SEGMENT.start, SEGMENT.stop, 0, 255)));
    newColor = ColorFromPalette(_currentPalette, index, br, SEGMENT.blendType); 

    leds[k] = nblend(leds[k], newColor, qadd8(SEGMENT.beat88>>8, 24));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Wave Effect over the complete strip.
 */
uint16_t WS2812FX::mode_fill_wave(void) {
  fill_palette( &leds[SEGMENT.start], 
                (LED_COUNT), 
                SEGMENT_RUNTIME.baseHue + (uint8_t)beatsin88(SEGMENT.beat88*2, 0, 255, SEGMENT_RUNTIME.tb.timebase),
                // SEGMENT_RUNTIME.baseHue + triwave8( (uint8_t)map( beat88( max(  SEGMENT.beat88/4, 2), SEGMENT_RUNTIME.tb.timebase), 0,  65535,  0,  255)),
                          max(  255/LED_COUNT+1, 1), 
                _currentPalette, 
                (uint8_t)beatsin88(  max(SEGMENT.beat88*1 , 1), 
                            _brightness/10, 255, 
                            SEGMENT_RUNTIME.tb.timebase),
                SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


/*
 * 3 "dots / small bars" moving with different 
 * wave functions and different speed.
 * fading can be specified separate to create several effects...
 */
uint16_t WS2812FX::mode_dot_beat_base(uint8_t fade) {
  if(SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.b16.beats[0] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88);
    SEGMENT_RUNTIME.b16.beats[1] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88);
    SEGMENT_RUNTIME.b16.beats[2] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88);
    SEGMENT_RUNTIME.oldVal = SEGMENT.beat88;

    SEGMENT_RUNTIME.tb.timebases[0] =
                    SEGMENT_RUNTIME.tb.timebases[1] = 
                    SEGMENT_RUNTIME.tb.timebases[2] = millis();
    SEGMENT_RUNTIME.nb.newBase[0] = 
                    SEGMENT_RUNTIME.nb.newBase[1] = 
                    SEGMENT_RUNTIME.nb.newBase[2] = false;
    SEGMENT_RUNTIME.co.coff[0] = random8(0,   85 );
    SEGMENT_RUNTIME.co.coff[1] = random8(85,  170);
    SEGMENT_RUNTIME.co.coff[2] = random8(170, 255);
  }
    

  if(SEGMENT_RUNTIME.oldVal != SEGMENT.beat88)
  {
    SEGMENT_RUNTIME.oldVal = SEGMENT.beat88;
    SEGMENT_RUNTIME.b16.beats[0] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88); 
    SEGMENT_RUNTIME.b16.beats[1] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88); 
    SEGMENT_RUNTIME.b16.beats[2] = max( (uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3,6)) , SEGMENT.beat88);
  }

  uint16_t cled = 0;
  const uint8_t width = 2;//max(LED_COUNT/15, 2);

  fade_out(fade);


  for(uint8_t i=0; i< 3; i++)
  {
    uint8_t cind = 0;
    switch (i)
    {
      case 0:
        cled = map(triwave16  (beat88(SEGMENT_RUNTIME.b16.beats[i], SEGMENT_RUNTIME.tb.timebases[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      case 1:
        cled = map(quadwave16 (beat88(SEGMENT_RUNTIME.b16.beats[i], SEGMENT_RUNTIME.tb.timebases[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      case 2:
        cled = map(cubicwave16(beat88(SEGMENT_RUNTIME.b16.beats[i], SEGMENT_RUNTIME.tb.timebases[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
      default:
        cled = map(quadwave16 (beat88(SEGMENT_RUNTIME.b16.beats[i], SEGMENT_RUNTIME.tb.timebases[i])), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
        
      break;
    }

    if(cled == SEGMENT.start*16)
    {
      if(SEGMENT_RUNTIME.nb.newBase[i]) 
      {
        SEGMENT_RUNTIME.tb.timebases[i] = millis();
        SEGMENT_RUNTIME.nb.newBase[i] = false;
      }
      SEGMENT_RUNTIME.b16.beats[i] = max((uint16_t)(SEGMENT_RUNTIME.b16.beats[i] + (int16_t)((int16_t)256 - (int16_t)random16(0 , 512))), SEGMENT.beat88);
      


      if(SEGMENT_RUNTIME.b16.beats[i] <= 256) SEGMENT_RUNTIME.b16.beats[i] = 256;
      if(SEGMENT_RUNTIME.b16.beats[i] >= 65535-512) SEGMENT_RUNTIME.b16.beats[i] = 65535-512;
      
      SEGMENT_RUNTIME.co.coff[i] = get_random_wheel_index(SEGMENT_RUNTIME.co.coff[i]); //random8(coff[i], 255) + rnd_hue;
    }
    else
    {
      SEGMENT_RUNTIME.nb.newBase[i] = true;
    }

    cind = SEGMENT_RUNTIME.co.coff[i]; // + map(cled/16, SEGMENT.start, SEGMENT.stop , 0, 255);

    drawFractionalBar(cled, width, _currentPalette, cind, _brightness, false);
   
  }
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_dot_beat(void) {
  return mode_dot_beat_base(64);
}

uint16_t WS2812FX::mode_dot_col_move(void) {
  return mode_dot_beat_base(0);
}

/* 
 *  color wipes
 */
uint16_t WS2812FX::mode_col_wipe_sawtooth(void) {
  return mode_col_wipe_func(3);
}

uint16_t WS2812FX::mode_col_wipe_sine(void) {
  return mode_col_wipe_func(0);
}

uint16_t WS2812FX::mode_col_wipe_quad(void) {
  return mode_col_wipe_func(2);
}

uint16_t WS2812FX::mode_col_wipe_triwave(void) {
  return mode_col_wipe_func(2);
}

uint16_t WS2812FX::mode_col_wipe_func(uint8_t mode) {
  #pragma message "Not perfect, but pretty close..."
  
  static uint8_t npos = 0;
  static uint16_t prev = SEGMENT.start;
  uint16_t i = 0;

  switch (mode)
  {
    case 0:
      i = beatsin16((SEGMENT.beat88*2)%65535, SEGMENT.start, SEGMENT.stop, SEGMENT_RUNTIME.tb.timebase);  
    break;
    case 1:
      i = triwave16(beat88((SEGMENT.beat88*2)%65535, SEGMENT_RUNTIME.tb.timebase));
      i = map(i, 0, 65534, SEGMENT.start, SEGMENT.stop+2);
    break;
    case 2:
      i = quadwave16(beat88((SEGMENT.beat88*2)%65535, SEGMENT_RUNTIME.tb.timebase));
      i = map(i, 0, 65535, SEGMENT.start, SEGMENT.stop+2);
    break;
    case 3:
      i = beat88((SEGMENT.beat88*4)%65535, SEGMENT_RUNTIME.tb.timebase);
      i = map(i, 0, 65535, SEGMENT.start, SEGMENT.stop+2);
    break;
    default:
      i = SEGMENT.start;
      fill_solid(leds, LED_COUNT, CRGB::Black);
  }

  if(i >= SEGMENT.stop) i = SEGMENT.stop;

  if(i == SEGMENT.start || i == SEGMENT.stop)
  {
    SEGMENT_RUNTIME.nb.newcolor = true;
  }
  else
  {
    if(SEGMENT_RUNTIME.nb.newcolor)
    {
      npos = get_random_wheel_index(npos, 16);
      SEGMENT_RUNTIME.nb.newcolor = false;
    }
  }
  if(prev > i)
  {
    fill_solid(&leds[i], prev-i+1, ColorFromPalette(_currentPalette, npos, _brightness, SEGMENT.blendType));
  }
  else if(prev < i)
  {
    fill_solid(&leds[prev], i-prev+1, ColorFromPalette(_currentPalette, npos, _brightness, SEGMENT.blendType));
  }
  else
  {
    leds[i] = ColorFromPalette(_currentPalette, npos, _brightness, SEGMENT.blendType);
  }

  prev = i;


  return STRIP_MIN_DELAY;
}

/*
 * Pulsing to the inner middle from both ends..
 */
uint16_t WS2812FX::mode_to_inner(void) {
  uint16_t led_up_to = (((LED_COUNT)/2+1)+SEGMENT.start);
  uint8_t fade = SEGMENT.beat88 * 5 <= 16320 ? (SEGMENT.beat88 * 5)>>6 : 255;
  SEGMENT.blur = max(fade, (uint8_t)16);
  fade_out(max(fade, (uint8_t)16)); //(64);
  
  fill_palette(&leds[SEGMENT.start], 
               beatsin88(
                         SEGMENT.beat88 < 13107 ? SEGMENT.beat88 * 5 : 65535, 
                         0, led_up_to, SEGMENT_RUNTIME.tb.timebase), 
               SEGMENT_RUNTIME.baseHue, 5, _currentPalette, 255, SEGMENT.blendType);
  for(uint8_t i = (LED_COUNT)-1; i>=((LED_COUNT) - led_up_to); i--)
  {
    if(((LED_COUNT)-i) >= 0 && ((LED_COUNT)-i) < (LED_COUNT))
    {
      leds[i] = leds[(LED_COUNT)-i];
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  
  fill_palette(&leds[SEGMENT.start], LED_COUNT, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, beatsin88(SEGMENT.beat88 * 2, 15, 255, SEGMENT_RUNTIME.tb.timebase), SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void) {
  if(SEGMENT_RUNTIME.modeinit) {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.tb.last = 0;
    
  }
  if(millis() > SEGMENT_RUNTIME.tb.last)
  {
    
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {

      SEGMENT_RUNTIME.co.last_index = get_random_wheel_index(SEGMENT_RUNTIME.co.last_index, 32);
      leds[i] = ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.co.last_index, _brightness, SEGMENT.blendType);
        
    }
    SEGMENT_RUNTIME.tb.last = millis() + ((BEAT88_MAX - SEGMENT.beat88)>>6);
  }

  return STRIP_MIN_DELAY; 
}

/*
 * Waving brightness over the complete strip.
 */
uint16_t WS2812FX::mode_fill_bright(void) {
  fill_palette(&leds[SEGMENT.start], (LED_COUNT), beat88(max((SEGMENT.beat88/128),2), SEGMENT_RUNTIME.tb.timebase), 
      max(255/LED_COUNT+1,1), _currentPalette, beatsin88(max(SEGMENT.beat88/32,1), 10, 255, SEGMENT_RUNTIME.tb.timebase),SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}


uint16_t WS2812FX::mode_firework(void){
  const uint8_t dist = max(LED_COUNT/20,2);

  static uint8_t colors[LED_COUNT];
  static uint8_t keeps[LED_COUNT];
  
  blur1d(&leds[SEGMENT.start], LED_COUNT, 172); //qadd8(255-(SEGMENT.beat88 >> 8), 32)%172); //was 2 instead of 16 before!
  
  for(uint16_t i=0; i<LED_COUNT; i++)
  {
    if(keeps[i])
    {
      keeps[i]--;
      nblend(leds[i], ColorFromPalette(_currentPalette, colors[i]  , 255, SEGMENT.blendType), 196);
      //leds[i] = ColorFromPalette(_currentPalette, colors[i]  , 255, SEGMENT.blendType);
    }
  }

  if(random8(max(6, LED_COUNT/7)) <= max(3, LED_COUNT/14)) 
  {
    uint8_t lind = random16(dist+SEGMENT.start, SEGMENT.stop-dist);
    uint8_t cind = random8() + SEGMENT_RUNTIME.baseHue;
    for(int8_t i = 0-dist; i<=dist; i++)
    {
      if(lind+i >= SEGMENT.start && lind + i < SEGMENT.stop)
      {
        if(!(leds[lind+i] == CRGB(0x0))) return STRIP_MIN_DELAY;
      }
    }
    colors[lind] = cind;
    leds[lind] = ColorFromPalette(_currentPalette, cind  , 255, SEGMENT.blendType);
    keeps[lind] = random8(2, 30);
  }

  addSparks(100, true, true);
  
  return STRIP_MIN_DELAY; // (BEAT88_MAX - SEGMENT.beat88) / 256; // STRIP_MIN_DELAY;
}

/*
 * Fades the LEDs on and (almost) off again.
 */
uint16_t WS2812FX::mode_fade(void) {
  //int lum = SEGMENT_RUNTIME.counter_mode_step - 31;
  //lum = 63 - (abs(lum) * 2);
  //lum = map(lum, 0, 64, 25, 255);

  fill_palette(&(leds[SEGMENT.start]), LED_COUNT, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, map8(triwave8(map(beat88(SEGMENT.beat88*10, SEGMENT_RUNTIME.tb.timebase),0,65535,0,255)),24,255), SEGMENT.blendType);
  
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 2) % 64;
  return STRIP_MIN_DELAY; //(SEGMENT.speed / 64);
}

/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255)), 0, 255, SEGMENT.start*16, SEGMENT.stop*16);
  const uint16_t width = 2; // max(2, LED_COUNT/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase)), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);
  
  // maybe we change this to fade?
  fill_solid(&(leds[SEGMENT.start]), LED_COUNT, CRGB(0, 0, 0));
  
  drawFractionalBar(SEGMENT.start * 16 + led_offset, width, _currentPalette, led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness);

  return STRIP_MIN_DELAY;
  
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255)), 0, 255, SEGMENT.start*16, SEGMENT.stop*16);
  const uint16_t width = 2; // max(2, LED_COUNT/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase)), 0, 65535, SEGMENT.start*16, SEGMENT.stop*16- width*16);
  

  fill_solid(&(leds[SEGMENT.start]), LED_COUNT, CRGB(0, 0, 0));

  drawFractionalBar(SEGMENT.stop  * 16 - led_offset, width, _currentPalette, 255-led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness); 
  drawFractionalBar(SEGMENT.start * 16 + led_offset, width, _currentPalette, led_offset/16 + SEGMENT_RUNTIME.baseHue, _brightness);

  return STRIP_MIN_DELAY;
}

/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  
  fill_solid(&leds[SEGMENT.start], LED_COUNT, ColorFromPalette(_currentPalette, map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255),_brightness, SEGMENT.blendType)); /*CHSV(beat8(max(SEGMENT.beat/2,1), SEGMENT_RUNTIME.tb.timebase)*/ //_brightness));
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 2) & 0xFF;
  return STRIP_MIN_DELAY; 
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
 
  fill_palette( &leds[SEGMENT.start], 
                LED_COUNT, 
                map(  beat88( SEGMENT.beat88, 
                              SEGMENT_RUNTIME.tb.timebase), 
                      0, 65535, 0, 255), 
                (LED_COUNT > 255 ? 1 : (255/LED_COUNT)+1), 
                _currentPalette, 
                255, SEGMENT.blendType);
  
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pride(void) {
  return pride(false);
}

uint16_t WS2812FX::mode_pride_glitter(void) {
  return pride(true);
}

/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(CRGBPalette16 color1, CRGBPalette16 color2) {
  uint16_t off = map(beat88(SEGMENT.beat88/2, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, 255) % 3;
  
  for(uint16_t i=0; i<LED_COUNT; i++)
  {
    uint8_t pal_index = map(i, 0, LED_COUNT-1, 0, 255) + SEGMENT_RUNTIME.baseHue;
    if((i % 3) == off) {  
      leds[SEGMENT.start + i] = ColorFromPalette(color1, pal_index, _brightness, SEGMENT.blendType);
    } else {
      leds[SEGMENT.start + i] = ColorFromPalette(color2, 128 + pal_index, _brightness/10, SEGMENT.blendType);
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return theater_chase(_currentPalette, CRGBPalette16(CRGB::Black));
}

uint16_t WS2812FX::mode_theater_chase_dual_pal(void) {
  return theater_chase(_currentPalette, _currentPalette);
}

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return theater_chase(CRGBPalette16(ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.counter_mode_step)), CRGBPalette16(CRGB::Black));
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
 
  for(uint16_t i=0; i < LED_COUNT; i++) {
    uint8_t lum = qsub8(sin8_C(map(i,0, LED_COUNT-1, 0, 255)), 2);
    uint16_t offset = map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, LED_COUNT-1);
    offset = (offset+i)%LED_COUNT;

    CRGB newColor = CRGB::Black;

    newColor = ColorFromPalette(_currentPalette, map(offset, 0, LED_COUNT-1, 0, 255) + SEGMENT_RUNTIME.baseHue, lum, SEGMENT.blendType); 
    nblend (leds[SEGMENT.start + offset], newColor, qadd8(SEGMENT.beat88>>8, 16));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  fade_out(qadd8(SEGMENT.beat88>>8, 12));
  addSparks(4, true, false);
  return STRIP_MIN_DELAY;
}

/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  
  const uint16_t width = max(1,LED_COUNT/15);
  fade_out(96);

  uint16_t pos = triwave16(beat88(SEGMENT.beat88*4, SEGMENT_RUNTIME.tb.timebase));
  
  pos = map(pos, 0, 65535, SEGMENT.start*16, SEGMENT.stop*16-width*16);

  
  drawFractionalBar(pos, 
                    width, 
                    _currentPalette, 
                    SEGMENT_RUNTIME.baseHue + map(pos, SEGMENT.start*16, SEGMENT.stop*16-width*16, 0, 255));

  
  return STRIP_MIN_DELAY;
}

/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  const uint16_t width = max(1,LED_COUNT/15);
  fade_out(96);

  uint16_t pos = map(beat88(SEGMENT.beat88*4, SEGMENT_RUNTIME.tb.timebase), 0, 65535, 0, LED_COUNT*16);

  drawFractionalBar((SEGMENT.start*16 + pos), 
                      width, 
                      _currentPalette, 
                      map(SEGMENT.start*16 + pos, SEGMENT.start*16, SEGMENT.stop*16, 0, 255) + SEGMENT_RUNTIME.baseHue);

  return STRIP_MIN_DELAY; 
}

/*
 * Fire flicker function
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity) {
 #pragma message "FixMe --> We shold really work with nblend or something like that"
  byte lum = 255 / rev_intensity;
  
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    int flicker = random8(0, lum);
    leds[i] = ColorFromPalette(_currentPalette, map(i, SEGMENT.start, SEGMENT.stop, 0, 255) + SEGMENT_RUNTIME.baseHue, _brightness, SEGMENT.blendType);   
    leds[i] -= CRGB(random8(flicker), random8(flicker), random8(flicker));
  }
  #pragma message "needs to be seen how this behaves - strip min delay would have been expected"
  return STRIP_MIN_DELAY; //(BEAT88_MAX - SEGMENT.beat88) / 256; // SEGMENT.beat * 100 / LED_COUNT; //return (SEGMENT.speed / LED_COUNT);
}

/*
 * Random flickering.
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  return fire_flicker(4);
}

/*
 * Random flickering, less intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_soft(void) {
  return fire_flicker(6);
}

/*
 * Random flickering, more intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_intense(void) {
  return fire_flicker(2);
}

uint16_t WS2812FX::mode_bubble_sort(void) {
  static uint8_t *hues;
  static bool init = true;
  static bool movedown = false;
  static uint16_t ci = 0;
  static uint16_t co = 0;
  static uint16_t cd = 0;
  if(init) {
    init = false;
    hues = new uint8_t[LED_COUNT];
    for(uint8_t i=0; i<LED_COUNT; i++)
    {
      if(i == 0)
      {
        hues[i] = random8();
      }
      else
      {
        hues[i] = get_random_wheel_index(hues[i-1], 48);
      }
    }
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    co = 0;
    ci = 0;
    return STRIP_MIN_DELAY;
  }
  if(!movedown) {
    if(co <= LED_COUNT) {
      if(ci <= LED_COUNT)
      {
        if(hues[ci] > hues[co])
        {
          uint8_t tmp = hues[ci];
          hues[ci] = hues[co];
          hues[co] = tmp;
          cd = ci;
          movedown = true;
        }
        ci++;
      }
      else
      {
        co++;
        ci = co;
      }
    }
    else
    {
      delete hues;
      init = true;
      return 5000;
    }
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    leds[ci + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); //CRGB::Green;
    leds[co + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); // CRGB::Red;
  }
  else
  {
    map_pixels_palette(hues, 32, SEGMENT.blendType);
    leds[co + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[ci], _brightness, SEGMENT.blendType); // CRGB::Red;
    leds[cd + SEGMENT.start] = ColorFromPalette(_currentPalette, hues[cd], _brightness, SEGMENT.blendType); // +=CRGB::Green;
    if(cd == co) {
      movedown = false;
    }
    cd --;
    return 0;  //STRIP_MIN_DELAY;
  }
  return 0;   //STRIP_MIN_DELAY;
}

/* 
 * Fire with Palette
 */
uint16_t WS2812FX::mode_fire2012WithPalette(void) {
  //uint8_t cooling = map(SEGMENT.beat88, BEAT88_MIN, BEAT88_MAX, 20, 100);
  //uint8_t sparking = map(SEGMENT.beat88, BEAT88_MIN, BEAT88_MAX, 50, 200);
  // Array of temperature readings at each simulation cell
  static byte *heat = new byte[LED_COUNT];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < LED_COUNT; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((SEGMENT.cooling * 10) / LED_COUNT) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= LED_COUNT - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SEGMENT.sparking ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < LED_COUNT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( _currentPalette, colorindex);

      leds[j + SEGMENT.start] = color;
    }
    return STRIP_MIN_DELAY;
}


/*
 * TwinleFox Implementation
 */
uint16_t WS2812FX::mode_twinkle_fox( void) {
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg = CRGB::Black;
  /*
  if((_currentPalette[0] == _currentPalette[1] ) && ) {
    bg = _currentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = CRGB::Black; //gBackgroundColor; // just use the explicitly defined background color
  }
  */

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( uint16_t i=0; i<LED_COUNT; i++) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      leds[i+SEGMENT.start] = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      leds[+SEGMENT.start] = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      leds[i+SEGMENT.start] = bg;
    }
  }
  return STRIP_MIN_DELAY;
}


/*
 * SoftTwinkles
 */
uint16_t WS2812FX::mode_softtwinkles(void) {
  CRGB lightcolor = CRGB (8,7,1);
  

  for( int i = 0; i < LED_COUNT; i++) {
    if( !leds[i+SEGMENT.start]) continue; // skip black pixels
    if( leds[i+SEGMENT.start].r & 1) { // is red odd?
      leds[i+SEGMENT.start] -= lightcolor; // darken if red is odd
    } else {
      leds[i+SEGMENT.start] += lightcolor; // brighten if red is even
    }
  }
  
  // Randomly choose a pixel, and if it's black, 'bump' it up a little.
  // Since it will now have an EVEN red component, it will start getting
  // brighter over time.
  if( random8() < 200 && !_transition) {
    int j = random16(SEGMENT.start, SEGMENT.stop);
    if( !leds[j] ) // && !leds[j+1] && !leds[j-1]) 
    {
      leds[j] = lightcolor;
    }
  }
  return STRIP_MIN_DELAY;
} 

/*
 * Shooting Star...
 * 
 */
uint16_t WS2812FX::mode_shooting_star() {
  
  uint16_t pos;

  static uint8_t numBars = 0;
  static uint16_t basebeat = 0;
  static uint16_t *delta_b;
  static uint8_t  *cind;
  static boolean  *new_cind;

  
  if(SEGMENT.numBars != numBars || basebeat != SEGMENT.beat88)
  {
    numBars = SEGMENT.numBars;
    basebeat = SEGMENT.beat88;

    delete [] delta_b;
    delete [] cind;
    delete [] new_cind;
    delta_b   = new uint16_t [numBars];
    cind      = new uint8_t  [numBars];
    new_cind  = new boolean  [numBars];
    for(uint8_t i = 0; i<numBars; i++)
    {
      delta_b[i]  = (65535 / numBars) * i;
      if(i>0)
          cind[i] = get_random_wheel_index(cind[i-1], 16);
        else
          cind[i] = get_random_wheel_index(cind[numBars-1], 16);

      new_cind[i] = false;
    }
  }

  fadeToBlackBy(leds, LED_COUNT>8?LED_COUNT-8:LED_COUNT, (SEGMENT.beat88>>8)|0x60);
  if(LED_COUNT > 8) blur1d(&leds[SEGMENT.stop-7], 8, 120);

  for(uint8_t i = 0; i<numBars; i++)
  {
    uint16_t beat = beat88(SEGMENT.beat88) + delta_b[i];
    
    double_t q_beat = (beat/100) * (beat/100);
    pos = map(static_cast<uint32_t>(q_beat + 0.5), 0, 429484, SEGMENT.start*16, SEGMENT.stop*16);
    
    //we use the fractional bar and 16 brghtness values per pixel 
    drawFractionalBar(pos, 1, _currentPalette, cind[i], _brightness); 

    
    if(pos/16 > (SEGMENT.stop - 6))
    {
      uint8_t tmp = 0;
      CRGB led = ColorFromPalette(_currentPalette, cind[i], _brightness, _segment.blendType); //leds[pos/16];
      if(led)
      {
        tmp = led.r | led.g | led.b;
        leds[pos/16].addToRGB(tmp%128);
      }


      new_cind[i] = true;
    }
    else
    {
      if(new_cind[i]) 
      {
        if(i>0)
          cind[i] = get_random_wheel_index(cind[i-1], 16);
        else
          cind[i] = get_random_wheel_index(cind[numBars-1], 16);

      }
      new_cind[i] = false;
    }
  }
 
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_beatsin_glow(void) {
  static uint8_t num_bars = 0;
  static uint16_t *beats;
  static uint16_t *theta;
  static int16_t  *prev;
  static uint32_t *times;
  static uint8_t *cinds;
  static bool    *newval;
  static uint16_t basebeat = SEGMENT.beat88;

  const uint16_t lim = (SEGMENT.beat88*10)/50;

  if(num_bars != SEGMENT.numBars || basebeat != SEGMENT.beat88)
  {
    _transition = true;
    _blend = 0;
    num_bars = SEGMENT.numBars;
    basebeat = SEGMENT.beat88;
    delete [] beats;
    delete [] theta;
    delete [] cinds;
    delete [] newval;
    delete [] prev;
    delete [] times;
    beats   = new uint16_t  [num_bars];
    theta   = new uint16_t  [num_bars];
    cinds   = new uint8_t   [num_bars];
    newval  = new bool      [num_bars];
    times   = new uint32_t  [num_bars];
    prev    = new int16_t   [num_bars];

    for(uint8_t i = 0; i<num_bars; i++)
    {
      beats[i] = SEGMENT.beat88 + lim/2 - random16(lim);
      theta[i] = (65535 / num_bars)*i + (65535 / (4*num_bars)) - random16(65535 / (2*num_bars));
      uint8_t temp = random8 (255   / (2*num_bars));
      if(temp & 0x01)
      {
        cinds[i] = (255   / num_bars)*i - temp;
      }
      else
      {
        cinds[i] = (255   / num_bars)*i + temp;
      }
      times[i] = millis() + random8();
      prev[i] = 0;
      newval[i] = false;
      #ifdef DEBUG
      Serial.printf("\n\tWe at init for bar number %d of %d\n", i+1, num_bars);
      Serial.println("\tWe randomly selected new values:");
      Serial.printf("\t\ti = \t%d \n\t\tbeat = \t%d \n\t\ttheta = \t%d\n\t\tcinds = \t%d\n\t\ttime = %d\n", i, beats[i], theta[i], cinds[i], times[i]);
      #endif
    }
  }

  fadeToBlackBy(leds, LED_COUNT, (SEGMENT.beat88 >> 8) | 32);

  uint16_t pos = 0;

  for(uint8_t i= 0; i < SEGMENT.numBars; i++)
  {
    uint16_t beatval = beat88(beats[i], times[i] + theta[i]);
    int16_t si = sin16(beatval);// + theta[i]);
    
    if(si > -2 && si < 2 && prev[i] < si) //si >= 32640 || si <= -32640)
    {
      #ifdef DEBUG
      int32_t deltaB, deltaT;
      int16_t deltaC;
      deltaB = beats[i];
      deltaT = theta[i];
      deltaC = cinds[i];
      #endif
      const uint8_t rand_delta = 32;
      beats[i] = beats[i] + (SEGMENT.beat88*10)/50 - random16((SEGMENT.beat88*10)/25);  //+= (random8(128)%2)?1:-1; // = beats[i] + (SEGMENT.beat88*10)/200 - random16((SEGMENT.beat88*10)/100); //
      if(beats[i] < (SEGMENT.beat88/2)) beats[i] = SEGMENT.beat88/2;
      if(beats[i] > (SEGMENT.beat88 + SEGMENT.beat88/2)) beats[i] = SEGMENT.beat88 + SEGMENT.beat88/2;
      theta[i] = theta[i] + (rand_delta/2)-random8(rand_delta);                                               //+= (random8(128)%2)?1:-1; // = theta[i] + 8-random8(16);  //
      cinds[i] = cinds[i] + (rand_delta/2)-random8(rand_delta); //+= (random8(128)%2)?1:-1;  
      times[i] = millis() - theta[i];
      #ifdef DEBUG
      deltaB-=beats[i];
      deltaT-=theta[i];
      deltaC-=cinds[i];
      Serial.printf("\n\tWe got stopped at i %d for angle si %d\n", i, si);
      Serial.println("\tWe came from the bottom and randomly selected new values:");
      Serial.printf("\t\ti = \t%d \n\t\tbeat = \t%d \n\t\ttheta = \t%d\n\t\tcinds = \t%d\n\t\ttime = %d\n", i, beats[i], theta[i], cinds[i], times[i]);
      Serial.printf("\tdelta Values: \n\t\tbeat = \t%d \n\t\ttheta = \t%d\n\t\tcinds = \t%d\n", deltaB, deltaT, deltaC);
      #endif
      
      newval[i] = false;
      
    }
    else
    {
      newval[i] = true;
    }

    prev[i] = si;

    pos = map((65535>>1) + si, 0, 65535, SEGMENT.start*16, SEGMENT.stop*16);
    drawFractionalBar(pos, 2, _currentPalette, cinds[i] + i * (255 / num_bars), _brightness, true);
  }
  return STRIP_MIN_DELAY;
}



/*
 * Custom mode
 */


uint16_t WS2812FX::mode_custom() {
  if(customMode == NULL) {
    return mode_static(); // if custom mode not set, we just do "static"
  } else {
    return customMode();
  }
}