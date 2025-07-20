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

// TODO: Probably make _segments more private?
// TODO: May something like https://gist.github.com/kriegsman/626dca2f9d2189bd82ca ??

#include "WS2812FX_FastLed.h"
#include "Effect.h"
#include "effects/StaticEffect.h"
#include "effects/EaseEffect.h"
#include "effects/PrideEffect.h"
#include "effects/ScanEffect.h"
#include "effects/DualScanEffect.h"
#include "effects/MultiDynamicEffect.h"
#include "effects/RainbowEffect.h"
#include "effects/RainbowCycleEffect.h"

/*
 * ColorPalettes
 */

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM = {
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray,
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green};

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red 0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM = {
    Holly_Green, Holly_Green, Holly_Green, Holly_Green,
    Holly_Green, Holly_Green, Holly_Green, Holly_Green,
    Holly_Green, Holly_Green, Holly_Green, Holly_Green,
    Holly_Green, Holly_Green, Holly_Green, Holly_Red};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM = {
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray};
// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM = {
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray};

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM = {
    CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight,
    HALFFAIRY, HALFFAIRY, CRGB::FairyLight, CRGB::FairyLight,
    QUARTERFAIRY, QUARTERFAIRY, CRGB::FairyLight, CRGB::FairyLight,
    CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight};

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM = {
    0x304048, 0x304048, 0x304048, 0x304048,
    0x304048, 0x304048, 0x304048, 0x304048,
    0x304048, 0x304048, 0x304048, 0x304048,
    0x304048, 0x304048, 0x304048, 0xE0F0FF};

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red 0xB80400
#define C9_Orange 0x902C02
#define C9_Green 0x046002
#define C9_Blue 0x070758
#define C9_White 0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM = {
    C9_Red, C9_Orange, C9_Red, C9_Orange,
    C9_Orange, C9_Red, C9_Orange, C9_Red,
    C9_Green, C9_Green, C9_Green, C9_Green,
    C9_Blue, C9_Blue, C9_Blue,
    C9_White};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM = {
    Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
    Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
    Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
    Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3};

// Iced Colors
const TProgmemRGBPalette16 Ice_Colors_p FL_PROGMEM = {
    CRGB::Black, CRGB::Black, CRGB::Blue, CRGB::Blue,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Aqua,
    CRGB::Aqua, CRGB::Aqua, CRGB::Aqua, CRGB::Aqua,
    CRGB::Aqua, CRGB::White, CRGB::White, CRGB::White};

// Sanity (human error prevention)
const TProgmemRGBPalette16 Random_p FL_PROGMEM = {
    CRGB::Black, CRGB::Black, CRGB::Blue, CRGB::Blue,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Aqua,
    CRGB::Aqua, CRGB::Aqua, CRGB::Aqua, CRGB::Aqua,
    CRGB::Aqua, CRGB::White, CRGB::White, CRGB::White};

// Totally Black palette (for fade through black transitions)
const TProgmemRGBPalette16 Total_Black_p FL_PROGMEM = {
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black};

// Shades
#define SHADE01 0xFE
#define SHADE02 0x90
#define SHADE03 0x60
#define SHADE04 0x40
#define SHADE05 0x18
// Values
#define REDVAL(A) ((A << 16) & 0xff0707)
#define GREENVAL(A) ((A << 8) & 0x07ff07)
#define BLUEVAL(A) ((A << 0) & 0x0707ff)

// Shades of Red
const TProgmemRGBPalette16 Shades_Of_Red_p FL_PROGMEM = {
    REDVAL(SHADE01), REDVAL(SHADE02), REDVAL(SHADE03), REDVAL(SHADE04),
    REDVAL(SHADE05), REDVAL(SHADE02), REDVAL(SHADE04), REDVAL(SHADE01),
    REDVAL(SHADE03), REDVAL(SHADE05), REDVAL(SHADE04), REDVAL(SHADE03),
    REDVAL(SHADE02), REDVAL(SHADE01), REDVAL(SHADE03), REDVAL(SHADE05)};

// Shades of Green
const TProgmemRGBPalette16 Shades_Of_Green_p FL_PROGMEM = {
    GREENVAL(SHADE01), GREENVAL(SHADE02), GREENVAL(SHADE03), GREENVAL(SHADE04),
    GREENVAL(SHADE05), GREENVAL(SHADE02), GREENVAL(SHADE04), GREENVAL(SHADE01),
    GREENVAL(SHADE03), GREENVAL(SHADE05), GREENVAL(SHADE04), GREENVAL(SHADE03),
    GREENVAL(SHADE02), GREENVAL(SHADE01), GREENVAL(SHADE03), GREENVAL(SHADE05)};

// Shades of Blue
const TProgmemRGBPalette16 Shades_Of_Blue_p FL_PROGMEM = {
    BLUEVAL(SHADE01), BLUEVAL(SHADE02), BLUEVAL(SHADE03), BLUEVAL(SHADE04),
    BLUEVAL(SHADE05), BLUEVAL(SHADE02), BLUEVAL(SHADE04), BLUEVAL(SHADE01),
    BLUEVAL(SHADE03), BLUEVAL(SHADE05), BLUEVAL(SHADE04), BLUEVAL(SHADE03),
    BLUEVAL(SHADE02), BLUEVAL(SHADE01), BLUEVAL(SHADE03), BLUEVAL(SHADE05)};

// #32 Color Palettes for the Effect
const TProgmemRGBPalette16 pacifica_palette_p1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 
      0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 
      0x00003B, 0x000046, 0x14554B, 0x28AA50 };
const TProgmemRGBPalette16 pacifica_palette_p2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 
      0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 
      0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
const TProgmemRGBPalette16 pacifica_palette_p3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 
      0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 
      0x001C70, 0x002080, 0x1040BF, 0x2060FF };
/*
 * <Begin> Service routines
 */

// Not much to be initialized...
void WS2812FX::init()
{
  //RESET_RUNTIME;            // this should be the only occurrence of RESET_RUNTIME now...
  fill_solid(_bleds, LED_COUNT, CRGB::Black);
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.clear(true); // During init, all pixels should be black.
  FastLED.setMaxRefreshRate(0, false); // Make sure tu use our own fps calculation
  FastLED.show();      // We show once to write the Led data.

  RESET_RUNTIME;

  SEG_RT.start = 0;
  SEG_RT.stop = LED_COUNT - 1;
  SEG_RT.length = LED_COUNT;

  _brightness = 255;

  bool isRunning = SEG.isRunning;
  bool power = SEG.power;

  // initialising segment
  setReverse              (SEG.reverse);
  setMirror               (SEG.mirror);
  setAutoplay             (SEG.autoplay);
  setAutopal              (SEG.autoPal);
  setBeat88               (SEG.beat88);
  setHuetime              (SEG.hueTime);
  setMilliamps            (SEG.milliamps);
  setAutoplayDuration     (SEG.autoplayDuration);
  setAutopalDuration      (SEG.autoPalDuration);
  setSegments             (SEG.segments);
  setCooling              (SEG.cooling);
  setSparking             (SEG.sparking);
  setTwinkleSpeed         (SEG.twinkleSpeed);
  setTwinkleDensity       (SEG.twinkleDensity);
  setNumBars              (SEG.numBars);
  setMode                 (SEG.mode);
  setMaxFPS               (SEG.fps);
  setDeltaHue             (SEG.deltaHue);
  setBlur                 (SEG.blur);
  setDamping              (SEG.damping);
  setDithering            (SEG.dithering);
  setSunriseTime          (SEG.sunrisetime);
  setTargetBrightness     (SEG.targetBrightness);
  setBlendType            (SEG.blendType);
  setColorTemp            (SEG.colorTemp);
  setTargetPaletteNumber  (SEG.targetPaletteNum);
  setCurrentPaletteNumber (SEG.currentPaletteNum);
  setAddGlitter           (SEG.addGlitter);
  setWhiteGlitter         (SEG.whiteGlitter);
  setOnBlackOnly          (SEG.onBlackOnly);
  setSynchronous          (SEG.synchronous);
  setBckndHue             (SEG.backgroundHue);
  setBckndSat             (SEG.backgroundSat);
  setBckndBri             (SEG.backgroundBri);
  setColCor               (SEG.colCor);

  #ifdef HAS_KNOB_CONTROL
  setWiFiDisabled         (SEG.wifiDisabled);
  #endif

  old_segs = 0;

  // should start with tranistion after init
  setTransition();
  setIsRunning(isRunning);
  setPower(power);

}

void WS2812FX::resetDefaults(void)
{
  SEG_RT.start = 0;
  SEG_RT.stop = LED_COUNT - 1;
  SEG_RT.length = LED_COUNT;

  fill_solid(_bleds, LED_COUNT, CRGB::Black);
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.clear(true); // During init, all pixels should be black.
  FastLED.show();      // We show once to write the Led data.

  _brightness = 255;
  SEG.solidColor = 0xC0C000;

  setIsRunning              (DEFAULT_RUNNING );
  setPower                  (DEFAULT_POWER);
  setReverse                (DEFAULT_REVERSE );
  setMirror                 (DEFAULT_MIRRORED );
  setAutoplay               (DEFAULT_AUTOMODE );
  setAutopal                (DEFAULT_AUTOCOLOR );
  setBeat88                 (DEFAULT_SPEED );
  setHuetime                (DEFAULT_HUE_INT );
  setMilliamps              (DEFAULT_CURRENT );
  setAutoplayDuration       (DEFAULT_T_AUTOMODE);
  setAutopalDuration        (DEFAULT_T_AUTOCOLOR);
  setSegments               (DEFAULT_NUM_SEGS);
  setCooling                (DEFAULT_COOLING );
  setSparking               (DEFAULT_SPARKING );
  setTwinkleSpeed           (DEFAULT_TWINKLE_S);
  setTwinkleDensity         (DEFAULT_TWINKLE_NUM);
  setNumBars                (DEFAULT_LED_BARS);
  setMode                   (DEFAULT_MODE);
  setMaxFPS                 (STRIP_MAX_FPS);
  setDeltaHue               (DEFAULT_HUE_OFFSET );
  setBlur                   (DEFAULT_BLENDING );
  setDamping                (DEFAULT_DAMPING);
  setDithering              (DEFAULT_DITHER);
  setTargetPaletteNumber    (DEFAULT_PALETTE);
  setCurrentPaletteNumber   (DEFAULT_PALETTE);
  setPaletteDistribution    (100);  // Default: 100% = normal distribution
  setSunriseTime            (DEFAULT_SUNRISETIME);
  setTargetBrightness       (DEFAULT_BRIGHTNESS);
  setBlendType              (DEFAULT_BLEND);
  setColorTemp              (DEFAULT_COLOR_TEMP);
  setBckndBri               (DEFAULT_BCKND_BRI);
  setBckndHue               (DEFAULT_BCKND_HUE);
  setBckndSat               (DEFAULT_BCKND_SAT);
  setAddGlitter             (DEFAULT_GLITTER_ADD);
  setWhiteGlitter           (DEFAULT_GLITTER_WHITE);
  setOnBlackOnly            (DEFAULT_GLITTER_ONBLACK);
  setSynchronous            (DEFAULT_GLITTER_SYNC);
  setColCor                 (COR_UncorrectedColor);
  #ifdef HAS_KNOB_CONTROL
  setWiFiDisabled           (DEFAULT_WIFI_DISABLED);
  #endif
  
  // Initialize per-effect speeds with default speed
  for (uint8_t i = 0; i < MODE_COUNT; i++) {
    SEG.effectSpeeds[i] = DEFAULT_SPEED;
  }
  
  FastLED.setBrightness(255); //(DEFAULT_BRIGHTNESS);
  RESET_RUNTIME;
  setTransition();
}


unsigned int WS2812FX::calc_CRC16(unsigned int crc, unsigned char *buf, int len)
{
  for (int pos = 0; pos < len; pos++)
  {
    crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--)
    { // Loop over each bit
      if ((crc & 0x0001) != 0)
      {            // If the LSB is set
        crc >>= 1; // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else         // Else LSB is not set
        crc >>= 1; // Just shift right
    }
  }
  return crc;
}


// template map function
template <typename T> T WS2812FX::map(T x, T x1, T x2, T y1, T y2)
{
  // avoid DIV by zero!
  if(((x2 - x1) + y1) == 0) return 0;

  // return min or max if input out of bounds
  if(x2>x1) {
    
    if(x<x1) return y1;
    if(x>x2) return y2;
  } else {
    if(x>x1) return y1;
    if(x<x2) return y2;
  }
  // return the mapped value
  return (x - x1) * (y2 - y1) / (x2 - x1) + y1;
}

/*
 * the overall service task. To be called as often as possible / useful
 * (at least at the desired frame rate)
 * --> see STRIP_MAX_FPS
 */
void WS2812FX::service()
{
  unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
  static uint32_t last_show = 0;

  if ((SEG.segments != old_segs))
  {

    SEG_RT.start = 0;
    SEG_RT.length = (LED_COUNT / SEG.segments);
    SEG_RT.stop = SEG_RT.start + SEG_RT.length - 1;
    if(SEG.numBars > ((LED_COUNT / SEG.segments) / MAX_NUM_BARS_FACTOR))
    {
      SEG.numBars = max(((LED_COUNT / SEG.segments) / MAX_NUM_BARS_FACTOR),1);
    }
    // 12.04.2019
    // There are artefacts remaining if the distribution is not equal.
    // as we blend towards the new effect, we will remove the artefacts by clearing the leds array...
    fill_solid(leds, LED_COUNT, CRGB::Black);
    //fill_solid(_bleds, LED_COUNT, CRGB::Black);
    
    setTransition();
    
    old_segs = SEG.segments;

    //_c_bck_b = 0;
    //_c_bck_h = 0;
    //_c_bck_s = 0;
  }

  if (SEG_RT.modeinit)
  {
    fill_solid(leds, LED_COUNT, CRGB::Black);
    setTransition();
    // reset the modeinit flag
    SEG_RT.modeinit = false;
  }
  
  if (SEG.power)
  {
    if (SEG.isRunning || _triggered)
    {

      if (now > SEG_RT.next_time || _triggered)
      {
        uint16_t delay;
        
        // Check if a class-based effect is available for this mode
        Effect* classEffect = EffectFactory::createEffect(SEG.mode);
        bool useClassBasedForThisMode = (classEffect != nullptr) || _useClassBasedEffects;
        
        if (classEffect) {
          delete classEffect; // We just needed to check if it exists
        }
        
        if (useClassBasedForThisMode) {
          // Use new effect system
          if (!_currentEffect || _currentEffect->getModeId() != SEG.mode) {
            // Switch to new effect
            if (_currentEffect) {
              _currentEffect->cleanup();
              delete _currentEffect;
            }
            
            _currentEffect = EffectFactory::createEffect(SEG.mode);
            if (_currentEffect) {
              _currentEffect->init(this);
            }
          }
          
          if (_currentEffect) {
            delay = _currentEffect->update(this);
          } else {
            // Fallback to function-based system if effect not found
            delay = (this->*_mode[SEG.mode])();
          }
        } else {
          // Use original function-based system
          delay = (this->*_mode[SEG.mode])();
        }
        
        SEG_RT.next_time = now + delay; 
      }
      // reset trigger...
      _triggered = false;
    }
    else
    {
      return;
    }
    
  }
  else
  {
    last_show = 0;
    //if (now > SEG_RT.next_time || _triggered)
    EVERY_N_MILLISECONDS(STRIP_DELAY_MICROSEC/1000)
    {
      SEG_RT.next_time = now + (uint32_t)(STRIP_DELAY_MICROSEC/1000);
      // no need to write data if nothing is shown (but we safeguard)
      
      // next approach for #35
      CRGB resCol = CRGB::Black;
      for(uint16_t i = 0; i < LED_COUNT; i++)
      {
        resCol|=leds[i];
        resCol|=_bleds[i];
      }
      if(resCol != CRGB(CRGB::Black)) {
          fadeToBlackBy(_bleds, LED_COUNT, 16);
          fadeToBlackBy(  leds, LED_COUNT, 16);
          
      }
      FastLED.show();
      /* could be activated to finally fix #35 (if I need to reopen the ticket)
      else // to be sure for #35 - don't know if this will fix as the root cause was not 
      {
        EVERY_N_SECONDS(5)
        {
          fill_solid(_bleds, LED_COUNT, CRGB::Black);
          FastLED.show();
          return;
        }
      }
      */
    }
    return;
  }
  
  
// check if we fade to a new FX mode.
uint8_t l_blend = SEG.blur; // to not overshoot during transitions we fade at max to "SEG.blur" parameter.
if (_transition)
{
  l_blend = _blend < SEG.blur ? _blend : SEG.blur;
  EVERY_N_MILLISECONDS(20)
  {
    // quickly blend from "old" to "new"
    _blend = qadd8(_blend, 1);
  }

  // reset once at max...
  // we could reset at SEG.blur as well
  // but 255 will always work and transition will be constant
  if (_blend == 255)
  {
    _transition = false;
    //_blend = 0;
  }
}

// Smooth brightness change?
EVERY_N_MILLISECONDS(5)
{
  uint8_t b = SEG.brightness; //FastLED.getBrightness();
  if (SEG.targetBrightness > b)
  {
    SEG.brightness = b + 1; //FastLED.setBrightness(b + 1);
  }
  else if (SEG.targetBrightness < b)
  {
    SEG.brightness = b - 1; //FastLED.setBrightness(b - 1);
  }
}

bool LEDupdate = false;
// if there is time left for another service call, we do not write the led data yet...
// but if there is less than 300 microseconds left, we do write..
if(micros() < (last_show + STRIP_DELAY_MICROSEC - FRAME_CALC_WAIT_MICROINTERVAL))
{
  // we have the time for another calc cycle and do nothing.
  LEDupdate = false;
}
else
{

  while(micros() < (last_show + STRIP_DELAY_MICROSEC)) {
    yield();
  } // just wait until time is "synced"
  
  // for FPS calculation
  _service_Interval_microseconds = (micros() - last_show);
  last_show = micros();
  LEDupdate = true;
}

// When VOID is active, we do nothing. 
// All data in _bleds just gets written to the LEDS
if(LEDupdate && SEG.mode == FX_MODE_VOID)
{
  FastLED.show();
  return;
}

// Thanks to some discussions on Github, I do still not use any memmove 
// but I realised that I need to nblend from the calculated frames to the led data.
// this could be simplified within the following nested loop which does now all at once and saves 2 loops + 
// one nblend over the complete strip data....
// as the combination of "mirror" and "reverse" is a bit redundant, this could maybe be simplified as well (later)
if(LEDupdate)
{
  // try to generally fade a bit to slowly remove any artefacts remaining
  // this should not affect the effect running as long the the l_blend value is 255
  fadeToBlackBy(_bleds, LED_COUNT, 1);
  for (uint16_t j = 0; j < SEG.segments; j++)
  {
    for (uint16_t i = 0; i < SEG_RT.length; i++)
    {
      if (SEG.mirror && (j & 0x01))
      {
        if(SEG.reverse)
        {
          nblend(_bleds[j * SEG_RT.length + i], leds[i], l_blend);
        }
        else
        {
          nblend(_bleds[j * SEG_RT.length + i], leds[SEG_RT.stop - i], l_blend);
        }
      }
      else
      {
        if(SEG.reverse)
        {
          nblend(_bleds[j * SEG_RT.length + i], leds[SEG_RT.stop - i], l_blend);
        }
        else
        {
          nblend(_bleds[j * SEG_RT.length + i], leds[i], l_blend);
        }
        
      }
    }
  }
}
// Background Color: Good idea, but needs some improvement.
// TODO: How to mix colors of different RGB values to remove the "glitch" when suddenly background switches to foreground.
// --> Needs to be done without having to much backgroud at all.

EVERY_N_MILLISECONDS(20)
{
  if(_c_bck_b < SEG.backgroundBri)
    _c_bck_b++;
  else if (_c_bck_b > SEG.backgroundBri)
    _c_bck_b--;

  if(_c_bck_s < SEG.backgroundSat)
    _c_bck_s++;
  else if (_c_bck_s > SEG.backgroundSat)
    _c_bck_s--;
  
  if(_c_bck_h < SEG.backgroundHue)
    _c_bck_h++;
  else if (_c_bck_h > SEG.backgroundHue)
    _c_bck_h--;
}

  CRGB BackGroundColor = CHSV(_c_bck_h, _c_bck_s, _c_bck_b);

  if(SEG.backgroundBri)
  {
    while(!BackGroundColor.getLuma())  BackGroundColor = CHSV(_c_bck_h, _c_bck_s, ++_c_bck_b); // 255);  // 0; //0x100000;
  }
  else
  {
    BackGroundColor = CRGB::Black;
  }
  

  if(SEG.backgroundBri)
  {
    for(uint16_t i=0; i < LED_COUNT; i++)
    {
      uint8_t bk_blend = 0;
      #define LOCAL_SCALE_FACT 100
      #define LOCAL_LUM_FACT 3

      if((uint16_t)(BackGroundColor.getLuma() * LOCAL_SCALE_FACT) > (uint16_t)(_bleds[i].getLuma() * LOCAL_SCALE_FACT * LOCAL_LUM_FACT))
      {
        bk_blend = (uint8_t)map(_bleds[i].getLuma() * LOCAL_SCALE_FACT * LOCAL_LUM_FACT, 0, BackGroundColor.getLuma() * LOCAL_SCALE_FACT, 255, 0);
      }
      if(_transition)
      {
        _bleds[i] |= BackGroundColor;
      }
      else
      {
        nblend(_bleds[i], BackGroundColor, bk_blend);
      }
      //_bleds[i] |= BackGroundColor;
      #undef LOCAL_SCALE_FACT
      #undef LOCAL_LUM_FACT
    }
  }

  // Glitter
  if(SEG.addGlitter)
  {
    addSparks(SEG.twinkleDensity, SEG.onBlackOnly, SEG.whiteGlitter, SEG.synchronous);
  }

  // Write the data
  if(LEDupdate) 
  {
    nscale8(_bleds, LED_COUNT,SEG.brightness);
    FastLED.show();
  }

  // every "hueTime" we set either the deltaHue (fixed offset)
  // or we increase the offset...
  if (now > SEG_RT.nextHue)
  {
    if (!SEG.hueTime)
    {
      SEG_RT.baseHue = SEG.deltaHue;
    }
    else
    {
      SEG_RT.baseHue++; // += SEG.deltaHue;
    }
    SEG_RT.nextHue = now + SEG.hueTime;
  }

  // Palette fading / blending
  if (getTargetPaletteNumber() == RANDOM_PAL)
  {
    EVERY_N_MILLISECONDS(RND_PAL_CHANGE_INT)
    { // Blend towards the target palette
      
      _currentPalette = getRandomPalette();
      /*
      static uint8_t current_distance = 0;
      if(current_distance >= 32)
      {
        setTargetPalette(RANDOM_PAL);
        current_distance = 0;
      }
      current_distance++;
      nblendPaletteTowardPalette(_currentPalette, _targetPalette, 255);
    
      if (_currentPalette == _targetPalette)
      {
        //_currentPaletteName = _targetPaletteName;
      }
      */
    }
  }
  else
  {
    EVERY_N_MILLISECONDS(12)
    { // Blend towards the target palette

      nblendPaletteTowardPalette(_currentPalette, _targetPalette, 8);
    
      if (_currentPalette == _targetPalette)
      {
        //_currentPaletteName = _targetPaletteName;
        if (getTargetPaletteNumber() == RANDOM_PAL)
        {
          setTargetPalette(RANDOM_PAL);
        }
      }
    }
  }

  // Autoplay
  if (now > SEG_RT.nextAuto)
  {
    if (!_transition)
    {
      nextMode(SEG.autoplay);
      SEG_RT.nextAuto = now + SEG.autoplayDuration * 1000;
    }
  }

  if (now > SEG_RT.nextPalette)
  {
    if (!_transition)
    {
      nextPalette(SEG.autoPal);
      SEG_RT.nextPalette = now + SEG.autoPalDuration * 1000;
    }
  }
}

void WS2812FX::start()
{
  setIsRunning(true);
  setPower(true);
}

void WS2812FX::stop()
{
  SEG.isRunning = false;
  strip_off();
}

void WS2812FX::trigger()
{
  _triggered = true;
}

void WS2812FX::show()
{
  nblend(_bleds, leds, LED_COUNT, SEG.blur);
  nscale8(_bleds, LED_COUNT,SEG.targetBrightness);
  FastLED.show();
}

/*
 * <End> Service routines
 */

/* 
 * <Begin> Helper Functions
 */

/*
 *
 * random Palette
 * 
 */
CRGBPalette16 WS2812FX::getRandomPalette(void)
{
  const uint8_t min_distance = 32;
  static uint8_t thue[16] = { 255, 0, 16, 32, 64, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };
  static uint8_t chue[16] = { 0, 16, 32, 64, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255 };
  static bool countUp[16];
  for(uint8_t i=0; i < 16; i++)
  {
    if(thue[i] != chue[i])
    {
      if(countUp[i])
      {
        chue[i]++;
      }
      else
      {
        chue[i]--;
      }
    }
    else
    {
      thue[i] = get_random_wheel_index(thue[(i+1)%16], min_distance);
      uint8_t delta = 0;
      if(thue[i] > chue[i])
      {
        delta = thue[i] - chue[i];
        if(delta > 128) countUp[i] = false;
      }
      else
      {
        delta = chue[i] - thue[i];
        if(delta > 128) countUp[i] = true;
      }
    }
    /*  
    if(thue[i] > chue[i])
    {
      chue[i]++;
    }
    else if (thue[i] < chue[i])
    {
      chue[i]--;
    }
    else
    {
      thue[i] = get_random_wheel_index(thue[i], min_distance);
    }
    */
  }

  return CRGBPalette16(
      CHSV(chue[0],  255, 255), CHSV(chue[1],  255, 255),
      CHSV(chue[2],  255, 255), CHSV(chue[3],  255, 255),
      CHSV(chue[4],  255, 255), CHSV(chue[5],  255, 255),
      CHSV(chue[6],  255, 255), CHSV(chue[7],  255, 255),
      CHSV(chue[8],  255, 255), CHSV(chue[9],  255, 255),
      CHSV(chue[10], 255, 255), CHSV(chue[11], 255, 255),
      CHSV(chue[12], 255, 255), CHSV(chue[13], 255, 255),
      CHSV(chue[14], 255, 255), CHSV(chue[15], 255, 255));
}

/*
 * saturating add variant with limit at lim
 */
uint8_t WS2812FX::qadd8_lim(uint8_t i, uint8_t j, uint8_t lim = 255)
{
  unsigned int t = i + j;
  if (t > lim)
    t = lim;
  return t;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit triwave
 */
inline uint16_t WS2812FX::triwave16(uint16_t in)
{
  if (in & 0x8000)
  {
    in = 65535 - in;
  }
  return in << 1;
}


/*
 *
 * 
 *
 */
inline uint16_t WS2812FX::ease16OutQuad(uint16_t i) 
{
  
  double val = (double)i / 65536.0;
  val = -(val * (val-2));
  return (uint16_t)(i*val);
}

/*
 *
 * 
 *
 */
inline uint16_t WS2812FX::ease16InQuad(uint16_t i) 
{
  return (i>>8)*(i>>8);
  double val = (double)i / 65536.0;
  val = (val * val);
  return (uint16_t)(i*val);
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit quadwave
 */
inline uint16_t WS2812FX::quadwave16(uint16_t in)
{
  return ease16InOutQuad(triwave16(in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutQuad
 */
inline uint16_t WS2812FX::ease16InOutQuad(uint16_t i)
{
  uint16_t j = i;
  if (j & 0x8000)
  {
    j = 65535 - j;
  }
  uint16_t jj = scale16(j, j);
  uint16_t jj2 = jj << 1;
  if (i & 0x8000)
  {
    jj2 = 65535 - jj2;
  }
  return jj2;
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit cubicWave
 */
inline uint16_t WS2812FX::cubicwave16(uint16_t in)
{
  return ease16InOutCubic(triwave16(in));
}

/*
 * Due to Fractional leds / stripes 
 * I preferred a 16 bit easeInOutCubic
 */
inline uint16_t WS2812FX::ease16InOutCubic(uint16_t i)
{

  uint16_t ii = scale16(i, i);
  uint16_t iii = scale16(ii, i);

  uint32_t r1 = (3 * (uint16_t)(ii)) - (2 * (uint16_t)(iii));

  uint16_t result = r1;

  // if we got "65536", return 65535:
  if (r1 & 0x10000)
  {
    result = 65535;
  }
  return result;
}

void WS2812FX::setColorTemperature(uint8_t index)
{
  switch (index)
  {
  case 0:
    SEG.colorTemp = Candle;
    break;
  case 1:
    SEG.colorTemp = Tungsten40W;
    break;
  case 2:
    SEG.colorTemp = Tungsten100W;
    break;
  case 3:
    SEG.colorTemp = Halogen;
    break;
  case 4:
    SEG.colorTemp = CarbonArc;
    break;
  case 5:
    SEG.colorTemp = HighNoonSun;
    break;
  case 6:
    SEG.colorTemp = DirectSunlight;
    break;
  case 7:
    SEG.colorTemp = OvercastSky;
    break;
  case 8:
    SEG.colorTemp = ClearBlueSky;
    break;
  default:
    SEG.colorTemp = UncorrectedTemperature;
    break;
  }
  FastLED.setTemperature(SEG.colorTemp);
}

uint8_t WS2812FX::getColorTemp(void)
{
  switch (SEG.colorTemp)
  {
  case Candle:
    return 0;
  case Tungsten40W:
    return 1;
  case Tungsten100W:
    return 2;
  case Halogen:
    return 3;
  case CarbonArc:
    return 4;
  case HighNoonSun:
    return 5;
  case DirectSunlight:
    return 6;
  case OvercastSky:
    return 7;
  case ClearBlueSky:
    return 8;
  default:
    return 9;
    break;
  }
}


const __FlashStringHelper * WS2812FX::getColorTempName(uint8_t index)
{
  const __FlashStringHelper * names[] = {
      F("Candle"),
      F("Tungsten40W"),
      F("Tungsten100W"),
      F("Halogen"),
      F("CarbonArc"),
      F("HighNoonSun"),
      F("DirectSunlight"),
      F("OvercastSky"),
      F("ClearBlueSky"),
      F("UncorrectedTemperature")};
  return names[index];
}

// #32 helpers
void WS2812FX::pacifica_layer(const CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < SEG_RT.length; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}
// Add extra 'white' to areas where the four layers of light have lined up brightly
void WS2812FX::pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i <  SEG_RT.length; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void WS2812FX::pacifica_deepen_colors()
{
  for( uint16_t i = 0; i <  SEG_RT.length; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

/* Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
   *
   * sixteenths of a pixel from the start of the strip.  Fractional positions are
   * rendered using 'anti-aliasing' of pixel brightness.
   * The bar width is specified in whole pixels.
   * Arguably, this is the interesting code. 
   */
void WS2812FX::drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, const uint8_t cindex, const uint8_t max_bright = 255, const bool mixColors = true, const uint8_t incIndex = 0)
{

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
  uint8_t firstpixelbrightness = scale8((255 - (frac * 16)), max_bright); //map8(15 - (frac), 0, max_bright);

  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness = scale8((255 - firstpixelbrightness), max_bright); //map8(15 - firstpixelbrightness, 0, max_bright);

  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
  uint8_t bright;
  bool mix = true;
  CRGB newColor = CRGB::Black;

  for (int n = 0; n <= width; n++)
  {
    if (n == 0)
    {
      // first pixel in the bar
      bright = firstpixelbrightness;
      newColor = WS2812FX::ColorFromPaletteWithDistribution(pal, cindex + (uint8_t)(n * incIndex), bright, SEG.blendType);
    }
    else if (n == width)
    {
      // last pixel in the bar
      bright = lastpixelbrightness;
      newColor = WS2812FX::ColorFromPaletteWithDistribution(pal, cindex + (uint8_t)(n * incIndex), bright, SEG.blendType);
    }
    else
    {
      // middle pixels
      bright = max_bright;
      mix = false;
      newColor = WS2812FX::ColorFromPaletteWithDistribution(pal, cindex + (uint8_t)(n * incIndex), bright, SEG.blendType);
      if(incIndex)
      { 
        if(SEG.blendType == LINEARBLEND)
        {
          CRGB prev_col = WS2812FX::ColorFromPaletteWithDistribution(pal, cindex + (uint8_t)((n-1) * incIndex), bright, SEG.blendType);
          CRGB next_col = WS2812FX::ColorFromPaletteWithDistribution(pal, cindex + (uint8_t)((n+1) * incIndex), bright, SEG.blendType);
          newColor = nblend(newColor, nblend(prev_col, next_col, firstpixelbrightness), 128);
        }
      }
    }

    if (i <= SEG_RT.stop && i >= SEG_RT.start)
    {
      if (mixColors || mix)
      {
        leds[i] |= newColor;
        //newColor = leds[i] | newColor;//ColorFromPalette(pal, cindex, bright, SEG.blendType);
        // we blend based on the "baseBeat"
        //nblend(leds[i], newColor, qadd8(SEG.beat88 >> 8, 24));
      }
      else
      {
        leds[i] = newColor;
      }
    }
    i++;
  }
}

/*
 * Returns a new, random wheel index with a minimum distance of dist (default = 42) from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos, uint8_t dist = 42)
{
  dist = dist < 85 ? dist : 85; // dist shouldn't be too high (not higher than 85 actually)
  return (pos + random8(dist, 255 - (dist))); 
}

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off()
{
  SEG.isRunning = false;
  FastLED.clear();
}

/*
 * Add sparks
 */
void WS2812FX::addSparks(const uint8_t prob = 5, const bool onBlackOnly = true, const bool white = false, const bool synchronous = true)
{
  
  const uint8_t probability = constrain(prob, DEFAULT_TWINKLE_NUM_MIN, DEFAULT_TWINKLE_NUM_MAX);
  const uint16_t maxSparks = ((LED_COUNT*DEFAULT_TWINKLE_NUM_MAX)/100) + 5;
  static uint16_t pos[maxSparks] = {0};
  static CRGB  sparks[maxSparks] = {0};
  const uint16_t activeMax = ((SEG_RT.length * prob)/100)    + 9;
  uint16_t active = 0;
  EVERY_N_MILLIS(10)
  {
    for(uint16_t i = 0; i<maxSparks; i++)
    {  
      sparks[i].r = qsub8(sparks[i].r, (1 + 10*SEG.twinkleSpeed)); 
      sparks[i].g = qsub8(sparks[i].g, (1 + 10*SEG.twinkleSpeed)); 
      sparks[i].b = qsub8(sparks[i].b, (1 + 10*SEG.twinkleSpeed)); 
      if(_bleds[pos[i]])
      {
        if(onBlackOnly) 
        {
          sparks[i].r = sparks[i].r/4;
          sparks[i].g = sparks[i].g/4;
          sparks[i].b = sparks[i].b/4;
        }
      }
    }
  }
  for(uint16_t i = 0; i<maxSparks; i++)
  {
    if(sparks[i])
    {
      active++;
      if(_bleds[pos[i]])
      {
        _bleds[pos[i]] += sparks[i];
      }
      else
      {
        _bleds[pos[i]] = sparks[i];
      }
      if(synchronous)
      {
        for(uint8_t j=0; j<SEG.segments; j++)
        {
          if (SEG.mirror && (j & 0x01))
          {
            _bleds[j * SEG_RT.length + SEG_RT.stop - pos[i]] = _bleds[pos[i]];
          }
          else
          {
            _bleds[j * SEG_RT.length + pos[i]] =  _bleds[pos[i]];
          }
        }
      }
    }
  }
  if(active > activeMax || random8(DEFAULT_TWINKLE_NUM_MAX) > probability)
    return;

  EVERY_N_MILLIS_I(timerObj, 10)
  {
    timerObj.setPeriod(10 * (DEFAULT_TWINKLE_NUM_MAX - probability));
    for(uint16_t i = 0; i<maxSparks; i++)
    {
      if(!sparks[i])
      {
        if(synchronous) { 
          pos[i] = random16(SEG_RT.start, SEG_RT.stop);
        } else {
          pos[i] = random16(0, LED_COUNT);
        }
        if (onBlackOnly && _bleds[pos[i]])
          return;
        
        uint8_t br = random8(192, 255);
        if(white)
        {
          sparks[i] = CRGB(br,br,br);
        }
        else
        {
          sparks[i] = ColorFromPaletteWithDistribution(_currentPalette, random8(), br, SEG.blendType);
        }
        return;
      }
    }
  }
  timerObj.setPeriod(10 * (DEFAULT_TWINKLE_NUM_MAX - probability));
  return;
}

void WS2812FX::map_pixels_palette(uint8_t *hues, uint8_t bright = 255, TBlendType blend = LINEARBLEND)
{
  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    leds[i + SEG_RT.start] = ColorFromPaletteWithDistribution(_currentPalette, hues[i], bright, blend);
  }
  return;
}

// Helper function for palette distribution-aware color selection
CRGB WS2812FX::ColorFromPaletteWithDistribution(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness, TBlendType blendType)
{
  // Apply palette distribution transformation to the index
  // paletteDistribution: 100% = full palette, 200% = palette repeats twice, 50% = half palette
  uint8_t adjustedIndex = (uint16_t(index) * SEG.paletteDistribution) / 100;
  return ColorFromPalette(pal, adjustedIndex, brightness, blendType);
}

/*
 *based on https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
 */ 
CRGB WS2812FX::computeOneTwinkle(uint32_t *ms, uint8_t *salt)
{
  //  This function takes a time in pseudo-milliseconds,
  //  figures out brightness = f( time ), and also hue = f( time )
  //  The 'low digits' of the millisecond time are used as
  //  input to the brightness wave function.
  //  The 'high digits' are used to select a color, so that the color
  //  does not change over the course of the fade-in, fade-out
  //  of one cycle of the brightness wave function.
  //  The 'high digits' are also used to determine whether this pixel
  //  should light at all during this cycle, based on the TWINKLE_DENSITY.
  //  uint8_t TWINKLE_SPEED = _twinkleSpeed; //map8(SEG.beat88>>8, 2, 8);
  //  Overall twinkle density.
  //  0 (NONE lit) to 8 (ALL lit at once).
  //  Default is 5.
  //  #define TWINKLE_DENSITY _twinkleDensity //6

  uint16_t ticks = *ms >> (8 - SEG.twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + *salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E) / 2) < SEG.twinkleDensity)
  {
    bright = attackDecayWave8(fastcycle8);
  }

#define COOL_LIKE_INCANDESCENT 0

  uint8_t hue = slowcycle8 - *salt;
  CRGB c;
  if (bright > 0)
  {
    c = ColorFromPaletteWithDistribution(_currentPalette, hue, bright, SEG.blendType);
    if (COOL_LIKE_INCANDESCENT == 1)
    {
      coolLikeIncandescent(c, fastcycle8);
    }
  }
  else
  {
    c = CRGB::Black;
  }
  return c;
}

uint8_t WS2812FX::attackDecayWave8(uint8_t i)
{
  if (i < 86)
  {
    return i * 3;
  }
  else
  {
    i -= 86;
    return 255 - (i + (i / 2));
  }
}

void WS2812FX::coolLikeIncandescent(CRGB &c, uint8_t phase)
{
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
  if (phase < 128)
    return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8(c.g, cooling);
  c.b = qsub8(c.b, cooling * 2);
}
/*
 * End Twinkle Fox
 */


/*
 * pride based on https://gist.github.com/kriegsman/964de772d64c502760e5
 */
/*
 * fade out function
 * fades out the current segment by dividing each pixel's intensity by 2
 */
void WS2812FX::fade_out(uint8_t fadeB = 32)
{
  fadeToBlackBy(&leds[SEG_RT.start], SEG_RT.length, fadeB);
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
void WS2812FX::setBlendType(TBlendType t = LINEARBLEND)
{
  SEG.blendType = t;
}

/*
 * Lets us toggle the Blend type
 */
void WS2812FX::toggleBlendType(void)
{
  SEG.blendType == NOBLEND ? SEG.blendType = LINEARBLEND : SEG.blendType = NOBLEND;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 */
void WS2812FX::setCurrentPalette(CRGBPalette16 p, String Name = "Custom")
{
  _currentPalette = p;
  //_currentPaletteName = Name;
  SEG.currentPaletteNum = NUM_PALETTES;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setCurrentPalette(uint8_t n = 0)
{
  _currentPalette = *(_palettes[n % NUM_PALETTES]);
  //_currentPaletteName = _pal_name[n % NUM_PALETTES];
  SEG.currentPaletteNum = n % NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * p: the Palette
 * Name: The name
 */
void WS2812FX::setTargetPalette(CRGBPalette16 p, String Name = "Custom")
{
  for (uint8_t i = 0; i < NUM_PALETTES; i++)
  {
    String tName = getPalName(i);
    if (tName == Name)
    {
      setTargetPalette(i);
      return;
    }
  }
  _targetPalette = p;
  //_targetPaletteName = Name;
  SEG.targetPaletteNum = NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setTargetPalette(uint8_t n = 0)
{
  if (n > getPalCount())
  {
    n = 0;
  }
  if (n == NUM_PALETTES)
  {
    setTargetPalette(CRGBPalette16(SEG.solidColor));
    return;
  }
  if (n == RANDOM_PAL)
  {
    _targetPalette = getRandomPalette();
    //_targetPaletteName = _pal_name[n % NUM_PALETTES];
    SEG.targetPaletteNum = n % NUM_PALETTES;
    return;
  }
  _targetPalette = *(_palettes[n % NUM_PALETTES]);
  //_targetPaletteName = _pal_name[n % NUM_PALETTES];
  SEG.targetPaletteNum = n % NUM_PALETTES;
}

/*
 * Change to the mode being provided
 * m: mode number
 */
void WS2812FX::setMode(uint8_t m)
{
  static uint8_t segs = SEG.segments;
  SEG_RT.modeinit = true;

  if (m == SEG.mode)
    return; // not really a new mode...

  // make sure its a valid mode
  m = constrain(m, 0, MODE_COUNT - 1);

  if (SEG.mode == FX_MODE_VOID && m != FX_MODE_VOID)
  {
    SEG.segments = segs; // restore previous "segments";
    //fill_solid(physicalLeds, LED_OFFSET, CRGB::Black); // clear the not accessible leds
  }

  if (!_transition && SEG.mode != FX_MODE_VOID)
  {
    // if we are not currently in a transition phase
    // we clear the led array (the one holding the effect
    // the real LEDs are drawn from _bleds and blended to the leds)
    // we also clear only if we are outside the "void" mode where we do not touch the LED array.
    fill_solid(leds, SEG_RT.length, CRGB::Black);
  }
  
  // Store current speed for the current effect before switching
  if (SEG.mode < MODE_COUNT) {
    SEG.effectSpeeds[SEG.mode] = SEG.beat88;
  }
  
  SEG.mode = m;
  
  // Restore speed for the new effect
  if (m < MODE_COUNT && SEG.effectSpeeds[m] > 0) {
    SEG.beat88 = SEG.effectSpeeds[m];
  }
  
  // start the transition phase
  setTransition();
  setBlur(_pblur);
  
  if (m == FX_MODE_VOID)
  {
    segs = SEG.segments;
    SEG.segments = 1;
  }
}

uint8_t WS2812FX::nextPalette(AUTOPLAYMODES mode)
{
  const uint8_t current = getTargetPaletteNumber();
  uint8_t newModefx = current;
  switch (mode)
  {
  case AUTO_MODE_OFF:
    break;
  case AUTO_MODE_UP:
    if (current < getPalCount())
    {
      setTargetPalette(current + 1);
    }
    else
    {
      setTargetPalette(0);
    }
    break;
  case AUTO_MODE_DOWN:
    if (current > 0)
    {
      setTargetPalette(getTargetPaletteNumber() - 1);
    }
    else
    {
      setTargetPalette(getPalCount() - 1);
    }
    break;
  case AUTO_MODE_RANDOM:
    while (newModefx == current)
    {
      newModefx = random8(getPalCount() - 1);
    }
    setTargetPalette(newModefx);
    break;
  default:
    break;
  }
  return getTargetPaletteNumber();
}

uint8_t WS2812FX::nextMode(AUTOPLAYMODES mode)
{

  if(SEG.mode >= FX_MODE_VOID)
  {
    return SEG.mode;
  }
  uint8_t newModefx = SEG.mode;
  switch (mode)
  {
  case AUTO_MODE_OFF:
    break;
  case AUTO_MODE_UP:
    if ((SEG.mode + 1) == (FX_MODE_VOID))
    {
      setMode(FX_MODE_STATIC);
    }
    else
    {
      setMode(SEG.mode + 1);
    }
    break;
  case AUTO_MODE_DOWN:
    if (SEG.mode == FX_MODE_STATIC)
    {
      setMode(FX_MODE_VOID - 1);
    }
    else
    {
      setMode(SEG.mode - 1);
    }
    break;
  case AUTO_MODE_RANDOM:
    while (newModefx == SEG.mode)
    {
      newModefx = random8(FX_MODE_VOID);
    }
    setMode(newModefx);
    break;
  default:
    break;
  }
  return getMode();
}


void WS2812FX::increaseSpeed(uint8_t s)
{
  uint16_t newSpeed = constrain(SEG.beat88 + s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s)
{
  uint16_t newSpeed = constrain(SEG.beat88 - s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  setColor(CRGBPalette16(((uint32_t)r << 16) | ((uint32_t)g << 8) | b));
  setBrightness(_brightness);
}

void WS2812FX::setColor(CRGBPalette16 c)
{
  //SEG.cPalette = c;
  setTargetPalette(c);
  setBrightness(_brightness);
}

void WS2812FX::setColor(uint32_t c)
{
  setColor(CRGBPalette16(c));
  setBrightness(_brightness);
}

void WS2812FX::setBrightness(uint8_t b)
{
  //_brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  b = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  SEG.targetBrightness = b;
  //FastLED.setBrightness(b);
  //FastLED.show();
}

void WS2812FX::increaseBrightness(uint8_t s)
{
  s = constrain(getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(uint8_t s)
{
  s = constrain(getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

uint8_t WS2812FX::getMode(void)
{
  if (_new_mode != 255)
  {
    return _new_mode;
  }
  else
  {
    return SEG.mode;
  }
}

uint16_t WS2812FX::getLength(void)
{
  return SEG_RT.stop - SEG_RT.start + 1;
}

uint16_t WS2812FX::getStripLength(void)
{
  return LED_COUNT;
}

uint16_t WS2812FX::getLedsOn(void)               
{ 
  if(this->getBrightness())
  {
    uint16_t leds_on = 0; 
    for(uint16_t i=0; i<SEG_RT.length; i++)
    {
      if(/*leds[i] || */_bleds[i])
      {
        leds_on++;
      }
    }
    return (leds_on * SEG.segments);
  }
  return 0;
}

uint32_t WS2812FX::getCurrentPower(void)         
{ 
  double factor = (double)calculate_max_brightness_for_power_mW(FastLED.getBrightness(), SEG.milliamps*5) / 255.0;
  return (uint32_t)(factor * (calculate_unscaled_power_mW(_bleds, LED_COUNT) - (LED_COUNT * get_gDark_mW())) + (LED_COUNT * get_gDark_mW()) + get_gMCU_mW());  
}

uint8_t WS2812FX::getModeCount(void)
{
  return MODE_COUNT;
}

uint8_t WS2812FX::getPalCount(void)
{
  return NUM_PALETTES;
}

uint32_t WS2812FX::getColor(uint8_t p_index = 0)
{
  return ColorFromPaletteWithDistribution(_currentPalette, p_index, 255, SEG.blendType);
}

const __FlashStringHelper *WS2812FX::getModeName(uint8_t m)
{
  if (m < MODE_COUNT)
  {
    // For class-based effects, get the name from the effect class
    Effect* effect = EffectFactory::createEffect(m);
    if (effect) {
      const __FlashStringHelper* name = effect->getName();
      delete effect; // Clean up temporary instance
      return name;
    }
    
    // Fall back to hardcoded names for function-based effects
    return _name[m];
  }
  else
  {
    return F("");
  }
}

const __FlashStringHelper *WS2812FX::getPalName(uint8_t p)
{
  if (p < NUM_PALETTES)
  {
    return _pal_name[p];
  }
  else
  {
    return F("Custom");
  }
}

/* 
 * <End> User Interface Functions (setables and getables)
 */

/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */


// mode_static() has been removed - now implemented as StaticEffect class

// mode_ease() has been removed - now implemented as EaseEffect class


uint16_t WS2812FX::mode_inoise8_mover(void)
{
  uint16_t xscale = SEG_RT.length; //30;
  uint16_t yscale = 30;
  const uint16_t width = 6; //max(SEG.beat88/256,1);
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.inoise.dist = 1234;
  }

  uint8_t locn = inoise8(xscale, SEG_RT_MV.inoise.dist + yscale);
  uint16_t pixlen = map((uint16_t)locn, (uint16_t)0, (uint16_t)255, (uint16_t)(SEG_RT.start * 16), (uint16_t)(SEG_RT.stop * 16 - width * 16));

  uint8_t colormove = SEG_RT.baseHue; // quadwave8(map(beat88(SEG.beat88, SEG_RT.timebase), 0, 65535, 0, 255)) + SEG_RT.baseHue;

  fade_out(48);

  drawFractionalBar(pixlen, width, _currentPalette, (uint8_t)((uint8_t)(pixlen / 64) + colormove), 255, true, 1); //, beatsin88(max(SEG.beat88/2,1),200 % _brightness, _brightness, SEG_RT.timebase));

  SEG_RT_MV.inoise.dist += beatsin88(SEG.beat88, 1, 12, SEG_RT_MV.inoise.timebase);

  return STRIP_MIN_DELAY;
}

/*
 * Plasma like Effect over the complete strip.
 */
uint16_t WS2812FX::mode_plasma(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.plasma.timebase = millis();
  }

  uint8_t thisPhase = beatsin88(SEG.beat88, 0, 255, SEG_RT_MV.plasma.timebase);             // Setting phase change for a couple of waves.
  uint8_t thatPhase = beatsin88((SEG.beat88 * 11) / 10, 0, 255, SEG_RT_MV.plasma.timebase); // was int thatPhase = 64 - beatsin88((SEG.beat88*11)/10, 0, 128, SEG_RT.timebase);

  for (int k = SEG_RT.start; k < SEG_RT.stop; k++)
  { // For each of the LED's in the strand, set a brightness based on a wave as follows:

    uint8_t colorIndex = cubicwave8((k * 15) + thisPhase) / 2 + cos8((k * 8) + thatPhase) / 2 + SEG_RT.baseHue; // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsuba(colorIndex, beatsin88((SEG.beat88 * 12) / 10, 0, 128));                               // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
    CRGB newColor = ColorFromPaletteWithDistribution(_currentPalette, colorIndex, thisBright, SEG.blendType);                        // Let's now add the foreground colour.
    leds[k] = nblend(leds[k], newColor, 64);
  }
  return STRIP_MIN_DELAY;
}

/*
 * Move 3 dots / small bars (antialised) at different speeds
 */
uint16_t WS2812FX::mode_juggle_pal(void)
{
  //const uint8_t numdots = 3;

  const uint8_t width = max(SEG_RT.length / 15, 3);
  uint8_t curhue = 0;
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.juggle.thishue = 0;
    SEG_RT_MV.juggle.timebase = millis();
  }
  curhue = SEG_RT_MV.juggle.thishue; // Reset the hue values.
  EVERY_N_MILLISECONDS(100)
  {
    SEG_RT_MV.juggle.thishue = random8(curhue, qadd8(curhue, 8));
  }

  fade_out(96);

  for (int i = 0; i < SEG.numBars; i++)
  {
    uint16_t pos = beatsin88(max(SEG.beat88 / 2, 1) + i * (SEG.beat88 / SEG.numBars), SEG_RT.start * 16, SEG_RT.stop * 16 - width * 16, SEG_RT_MV.juggle.timebase);
    drawFractionalBar(pos, width, _currentPalette, curhue + (255 / SEG.numBars) * i, 255, true, 1);
    uint8_t delta = random8(9);
    if (delta < 5)
    {
      curhue = curhue - (uint8_t)(delta) + SEG_RT.baseHue;
    }
    else
    {
      curhue = curhue + (uint8_t)(delta / 2) + SEG_RT.baseHue;
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * Fills the strip with waving color and brightness
 */
uint16_t WS2812FX::mode_fill_beat(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.fill_beat.timebase = millis();
  }
  CRGB newColor = CRGB::Black;
  uint8_t br, index;
  for (uint8_t k = SEG_RT.start; k < SEG_RT.stop; k++)
  {

    br = beatsin88(SEG.beat88, 20, 255, SEG_RT_MV.fill_beat.timebase, k * 2); //= quadwave8(v1);
    index = (uint8_t)((uint8_t)triwave8(beat8(SEG.beat88 >> 8) +
                                        (uint8_t)beatsin8(SEG.beat88 >> 8, 0, 20) +
                                        (uint8_t)map((uint16_t)k, (uint16_t)SEG_RT.start, (uint16_t)SEG_RT.stop, (uint16_t)0, (uint16_t)255)));
    newColor = ColorFromPaletteWithDistribution(_currentPalette, index, br, SEG.blendType);

    leds[k] = nblend(leds[k], newColor, qadd8(SEG.beat88 >> 8, 24));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Wave Effect over the complete strip.
 */
uint16_t WS2812FX::mode_fill_wave(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.fill_wave.timebase = millis();
  }
  fill_palette(&leds[SEG_RT.start],
               (SEG_RT.length),
               SEG_RT.baseHue + (uint8_t)beatsin88(SEG.beat88 * 2, 0, 255, SEG_RT_MV.fill_wave.timebase),
               // SEG_RT.baseHue + triwave8( (uint8_t)map( beat88( max(  SEG.beat88/4, 2), SEG_RT.timebase), 0,  65535,  0,  255)),
               max(255 * 100 / (SEG_RT.length * SEG.paletteDistribution) + 1, 1),
               _currentPalette,
               (uint8_t)beatsin88(max(SEG.beat88 * 1, 1),
                                  _brightness / 10, 255,
                                  SEG_RT_MV.fill_wave.timebase),
               SEG.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * 3 "dots / small bars" moving with different 
 * wave functions and different speed.
 * fading can be specified separate to create several effects...
 * TODO: make number of dots "dynamic"
 */
uint16_t WS2812FX::mode_dot_beat_base(uint8_t fade)
{
  #define SRMVDB SEG_RT_MV.dot_beat
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;

    SRMVDB.oldVal = SEG.beat88;
    uint32_t tb = millis();
    for(uint8_t i=0; i< SEG.numBars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEG.beat88 / random8(1, 3)) * random8(3, 6)), SEG.beat88);
      SRMVDB.timebases[i] = tb;
      SRMVDB.newBase[i] = false;
      SRMVDB.coff[i] = random8(i*(255/SEG.numBars), (i+1)*SEG.numBars);
    }
  }

  if (SRMVDB.oldVal != SEG.beat88)
  {
    SRMVDB.oldVal = SEG.beat88;
    for(uint8_t i = 0; i<SEG.numBars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEG.beat88 / random8(1, 3)) * random8(3, 6)), SEG.beat88);
    }
  }

  uint16_t cled = 0;
  const uint8_t width = 3; //max(SEG_RT.length/15, 2);

  fade_out(fade);

  for (uint8_t i = 0; i < SEG.numBars; i++)
  {
    uint8_t cind = 0;
    
    uint8_t sw = i%3;
    switch (sw)
    {
    case 0:
      cled = triwave16(  beat88(SRMVDB.beats[i], SRMVDB.timebases[i]));

      break;
    case 1:
      cled = quadwave16( beat88(SRMVDB.beats[i], SRMVDB.timebases[i]));

      break;
    case 2:
      cled = cubicwave16(beat88(SRMVDB.beats[i], SRMVDB.timebases[i]));

      break;
    default:
      cled = quadwave16( beat88(SRMVDB.beats[i], SRMVDB.timebases[i]));

      break;
    }
    cled = map(cled, (uint16_t)0, (uint16_t)65535, (uint16_t)(SEG_RT.start * 16), (uint16_t)(SEG_RT.stop * 16 - width * 16));

    if (cled == SEG_RT.start * 16)
    {
      if (SRMVDB.newBase[i])
      {
        SRMVDB.timebases[i] = millis();
        SRMVDB.newBase[i] = false;
      }
      SRMVDB.beats[i] = max((uint16_t)(SRMVDB.beats[i] + (int16_t)((int16_t)256 - (int16_t)random16(0, 512))), SEG.beat88);

      if (SRMVDB.beats[i] <= 256)
        SRMVDB.beats[i] = 256;
      if (SRMVDB.beats[i] >= 65535 - 512)
        SRMVDB.beats[i] = 65535 - 512;

      SRMVDB.coff[i] = get_random_wheel_index(SRMVDB.coff[i], 64); //random8(coff[i], 255) + rnd_hue;
    }
    else
    {
      SRMVDB.newBase[i] = true;
    }

    cind = SRMVDB.coff[i]; // + map(cled/16, SEG_RT.start, SEG_RT.stop , 0, 255);

    drawFractionalBar(cled, width, _currentPalette, cind, 255, false, 1);
  }
  #undef SRMVDB
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_dot_beat(void)
{
  return mode_dot_beat_base(64);
}

uint16_t WS2812FX::mode_dot_col_move(void)
{
  return mode_dot_beat_base(0);
}

/* 
 *  color wipes
 */
uint16_t WS2812FX::mode_col_wipe_sawtooth(void)
{
  return mode_col_wipe_func(3);
}

uint16_t WS2812FX::mode_col_wipe_sine(void)
{
  return mode_col_wipe_func(0);
}

uint16_t WS2812FX::mode_col_wipe_quad(void)
{
  return mode_col_wipe_func(2);
}

uint16_t WS2812FX::mode_col_wipe_triwave(void)
{
  return mode_col_wipe_func(2);
}

uint16_t WS2812FX::mode_col_wipe_func(uint8_t mode)
{
  
  
  uint16_t i = 0;

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.col_wipe.npos = get_random_wheel_index(0, 32);
    SEG_RT_MV.col_wipe.pnpos = get_random_wheel_index(SEG_RT_MV.col_wipe.npos, 32);
    SEG_RT_MV.col_wipe.prev = SEG_RT.start;
    SEG_RT_MV.col_wipe.newcolor = true;
    SEG_RT_MV.col_wipe.up = true;
    SEG_RT_MV.col_wipe.timebase = millis();
  }

  switch (mode)
  {
  case 0:
    i = beatsin16((SEG.beat88 * 2) % 65535, 0, 65535,  SEG_RT_MV.col_wipe.timebase);
    break;
  case 1:
    i = triwave16(beat88((SEG.beat88 * 2) % 65535,  SEG_RT_MV.col_wipe.timebase));
    break;
  case 2:
    i = quadwave16(beat88((SEG.beat88 * 2) % 65535,  SEG_RT_MV.col_wipe.timebase));
    break;
  case 3:
    i = beat88((SEG.beat88 * 4) % 65535,  SEG_RT_MV.col_wipe.timebase);
    break;
  default:
    i = SEG_RT.start;
    fill_solid(leds, SEG_RT.length, CRGB::Black);
  }
  i = map((uint16_t)i, (uint16_t)0, (uint16_t)65535, (uint16_t)SEG_RT.start, (uint16_t)(SEG_RT.stop + 2));

  if (i >= SEG_RT.stop)
    i = SEG_RT.stop;

  if((( SEG_RT_MV.col_wipe.up && i < SEG_RT_MV.col_wipe.prev) || 
     (!SEG_RT_MV.col_wipe.up && i > SEG_RT_MV.col_wipe.prev))) // direction changed - we've been at the bottom
  {
    SEG_RT_MV.col_wipe.up = !SEG_RT_MV.col_wipe.up;
    SEG_RT_MV.col_wipe.newcolor = true;
  }
  if (SEG_RT_MV.col_wipe.newcolor)
  {
    SEG_RT_MV.col_wipe.pnpos = SEG_RT_MV.col_wipe.npos;
    SEG_RT_MV.col_wipe.npos = get_random_wheel_index(SEG_RT_MV.col_wipe.npos, 32);
    SEG_RT_MV.col_wipe.newcolor = false;
  }

  CRGB Col1 = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.col_wipe.npos  + SEG_RT.baseHue, _brightness, SEG.blendType);
  CRGB Col2 = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.col_wipe.pnpos + SEG_RT.baseHue, _brightness, SEG.blendType);


  if(SEG_RT_MV.col_wipe.up)
  {
    if(i > 0)
    { 
      fill_solid(leds, i, Col1);  
    }
    if(i < SEG_RT.stop)
    { 
      fill_solid(&leds[i], SEG_RT.stop-i, Col2);
    }
  }
  else
  {
    if(i<SEG_RT.stop)
    { 
      fill_solid(&leds[i], SEG_RT.stop-i, Col1);
    }
    if(i>0)
    { 
      fill_solid(leds, i, Col2);  
    }
  }
  SEG_RT_MV.col_wipe.prev = i;

  return STRIP_MIN_DELAY;
}

/*
 * Pulsing to the inner middle from both ends..
 */
uint16_t WS2812FX::mode_to_inner(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.to_inner.timebase = millis();
  }
  uint16_t led_up_to = (((SEG_RT.length) / 2 + 1) + SEG_RT.start);
  uint8_t fade = SEG.beat88 * 5 <= 16320 ? (SEG.beat88 * 5) >> 6 : 255;
 // SEG.blur = max(fade, (uint8_t)16);
  fade_out(max(fade, (uint8_t)16)); //(64);

  fill_palette(&leds[SEG_RT.start],
               beatsin88(
                   SEG.beat88 < 13107 ? SEG.beat88 * 5 : 65535,
                   0, led_up_to, SEG_RT_MV.to_inner.timebase),
               SEG_RT.baseHue, 5, _currentPalette, 255, SEG.blendType);
  for (uint16_t i = (SEG_RT.length) - 1; i >= ((SEG_RT.length) - led_up_to); i--)
  {
    if (((SEG_RT.length) - i) >= 0 && ((SEG_RT.length) - i) < (SEG_RT.length))
    {
      leds[i] = leds[(SEG_RT.length) - i];
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.breath.timebase = millis();
  }
  fill_palette(&leds[SEG_RT.start], SEG_RT.length, 0 + SEG_RT.baseHue, 5, _currentPalette, beatsin88(SEG.beat88 * 2, 10, 255, SEG_RT_MV.breath.timebase), SEG.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * Waving brightness over the complete strip.
 */
uint16_t WS2812FX::mode_fill_bright(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.fill_bright.timebase = millis();
  }
  fill_palette(&leds[SEG_RT.start], (SEG_RT.length), beat88(max((SEG.beat88 / 128), 2), SEG_RT_MV.fill_bright.timebase),
               max(255 * 100 / (SEG_RT.length * SEG.paletteDistribution) + 1, 1), _currentPalette, beatsin88(max(SEG.beat88 / 32, 1), 10, 255, SEG_RT_MV.fill_bright.timebase), SEG.blendType);
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_firework(void)
{
  #define FW1MV SEG_RT_MV.firework
  const uint8_t dist = max(SEG_RT.length / 20, 2);
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    fill_solid(leds, LED_COUNT, CRGB::Black);
    memset(FW1MV.fireworks, 0x0, MAX_NUM_BARS * sizeof(uint16_t));
    memset(FW1MV.colIndex,  0x0, MAX_NUM_BARS * sizeof( uint8_t));
    memset(FW1MV.isBurning, 0x0, MAX_NUM_BARS * sizeof( uint8_t));
    //memset(FW1MV.colors, 0x0, SEG_RT.length * sizeof(uint8_t));
    //memset(FW1MV.keeps, 0x0, SEG_RT.length * sizeof(uint8_t));
  }

  blur1d(&leds[SEG_RT.start], SEG_RT.length, qadd8(255-(SEG.beat88 >> 8), 32)%172); // 172); //qadd8(255-(SEG.beat88 >> 8), 32)%172); //was 2 instead of 16 before!

  for (uint16_t i = 0; i<MAX_NUM_BARS; i++)
  {
    if (FW1MV.isBurning[i])
    {
      FW1MV.isBurning[i]--;
      nblend(leds[FW1MV.fireworks[i]], ColorFromPaletteWithDistribution(_currentPalette, FW1MV.colIndex[i], 255, SEG.blendType), 196);
      //leds[i] = ColorFromPalette(_currentPalette, colors[i]  , 255, SEG.blendType);
    }
  }

  if (random8(max(6, SEG_RT.length / 7)) <= max(3, SEG_RT.length / 14))
  {
    uint16_t lind = random16(dist + SEG_RT.start, SEG_RT.stop - dist);
    for (int8_t i = 0 - dist; i <= dist; i++)
    {
      if (lind + i >= SEG_RT.start && lind + i < SEG_RT.stop)
      {
        if (!(leds[lind + i] == CRGB(0x0)))
          return STRIP_MIN_DELAY;
      }
    }
    uint8_t barPos = 0xff;
    for(uint8_t i=0; i<MAX_NUM_BARS; i++)
    {
      if(!leds[FW1MV.fireworks[i]])
      {
        FW1MV.fireworks[i] = lind;
        barPos = i;
        break;
      }
    }
    if(barPos != 0xff)
    {
      FW1MV.colIndex [barPos] = get_random_wheel_index(FW1MV.colIndex[barPos], 64);
      FW1MV.fireworks[barPos] = lind;
      FW1MV.isBurning[barPos] = random8(10, 30);
      leds[lind] = ColorFromPaletteWithDistribution(_currentPalette, FW1MV.colIndex [barPos], random8(192, 255), SEG.blendType);
    }
  }

  return STRIP_MIN_DELAY; // (BEAT88_MAX - SEG.beat88) / 256; // STRIP_MIN_DELAY;
}

/*
 * Fades the LEDs on and (almost) off again.
 */
/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(CRGB color)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.theater_chase.timebase = millis();
  }
  uint16_t off = map(beat88(SEG.beat88 / 2,  SEG_RT_MV.theater_chase.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255) % 3;

  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    
    if ((i % 3) == off)
    {
      leds[SEG_RT.start + i] = color;
    }
    else
    {
      leds[SEG_RT.start + i] = CRGB::Black;
    }
  }
  return STRIP_MIN_DELAY;
}

// theater_chase(bool dual) implementation removed - functionality now in TheaterChaseDualPaletteEffect class

// mode_theater_chase implementation removed - now implemented as TheaterChaseEffect class

// mode_theater_chase_dual_pal implementation removed - now implemented as TheaterChaseDualPaletteEffect class

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void)
{
  SEG_RT_MV.theater_chase.counter_mode_step = (SEG_RT_MV.theater_chase.counter_mode_step + 1) & 0xFF;
  return theater_chase(ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.theater_chase.counter_mode_step, 255, SEG.blendType));
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.running_lights.timebase = millis();
  }
  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    uint8_t lum = qsub8(sin8_C(map(i, (uint16_t)0, (uint16_t)(SEG_RT.length - 1), (uint16_t)0, (uint16_t)255)), 2);
    uint16_t offset = map(beat88(SEG.beat88, SEG_RT_MV.running_lights.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(SEG_RT.length * 10)); //map(beat88(SEG.beat88, SEG_RT.timebase), 0, 65535, 0, SEG_RT.length - 1);
    offset = (offset + i) % SEG_RT.length;

    CRGB newColor = CRGB::Black;

    newColor = ColorFromPaletteWithDistribution(_currentPalette, map(offset, (uint16_t)0, (uint16_t)(SEG_RT.length - 1), (uint16_t)0, (uint16_t)255) + SEG_RT.baseHue, lum, SEG.blendType);
    nblend(leds[SEG_RT.start + offset], newColor, qadd8(SEG.beat88 >> 8, 16));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
  }

  EVERY_N_MILLISECONDS(STRIP_MIN_DELAY)
  {
    fade_out(qadd8(SEG.beat88 >> 8, 12));
  }
  
  uint16_t numsparks = 0;
  for(uint16_t i=0; i<SEG_RT.length; i++)
  {
    if(leds[i])
    {
      numsparks++;
    }
  }
  uint16_t maxsparks = map((uint16_t)SEG.twinkleDensity, (uint16_t)0, (uint16_t)8, (uint16_t)0, (uint16_t)SEG_RT.length);
  
  if(numsparks < maxsparks)
  {
    uint16_t i = random16(SEG_RT.length);
    if(!leds[i]) 
    {
      leds[i] = ColorFromPaletteWithDistribution(_currentPalette, random8(), random8(128,255), SEG.blendType);
    }
    else
    {
      leds[i].fadeToBlackBy(16);
    }
  }
  else
  {
    
  }
  
  return 0;// STRIP_MIN_DELAY;
}

/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.larson_scanner.timebase = millis();
  }

  const uint16_t width = max(1, SEG_RT.length / 15);
  fade_out(96);

  uint16_t pos = triwave16(beat88(SEG.beat88 * 4, SEG_RT_MV.larson_scanner.timebase));

  pos = map(pos, (uint16_t)0, (uint16_t)65535, (uint16_t)(SEG_RT.start * 16), (uint16_t)(SEG_RT.stop * 16 - width * 16));

  drawFractionalBar(pos,
                    width,
                    _currentPalette,
                    SEG_RT.baseHue + map(pos, (uint16_t)(SEG_RT.start * 16), (uint16_t)(SEG_RT.stop * 16 - width * 16), (uint16_t)0, (uint16_t)255), 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.comet.timebase = millis();
  }
  const uint16_t width = max(1, SEG_RT.length / 15);
  fade_out(96);

  uint16_t pos = map(beat88(SEG.beat88 * 4, SEG_RT_MV.comet.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(SEG_RT.length * 16));

  drawFractionalBar((SEG_RT.start * 16 + pos),
                    width,
                    _currentPalette,
                    map((uint16_t)(SEG_RT.start * 16 + pos), (uint16_t)(SEG_RT.start * 16), (uint16_t)(SEG_RT.stop * 16), (uint16_t)0, (uint16_t)255) + SEG_RT.baseHue, 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Fire flicker function
 * 
 * TODO: Could add parameter for the intensity (instead of 3 different modes?)
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
  }
  byte lum = 255 / rev_intensity;

  for (uint16_t i = SEG_RT.start; i <= SEG_RT.stop; i++)
  {
    leds[i] = ColorFromPaletteWithDistribution(_currentPalette, map(i, SEG_RT.start, SEG_RT.stop, (uint16_t)0, (uint16_t)255) + SEG_RT.baseHue, _brightness, SEG.blendType);
    if (random8(3))
    {
      int flicker = random8(0, lum);
      CRGB temp = leds[i];
      temp -= CRGB(random8(flicker), random8(flicker), random8(flicker));
      nblend(leds[i], temp, 96);
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * Random flickering.
 */
uint16_t WS2812FX::mode_fire_flicker(void)
{
  return fire_flicker(2);
}

/*
 * Random flickering, less intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_soft(void)
{
  return fire_flicker(3);
}

/*
 * Random flickering, more intesity.
 */
uint16_t WS2812FX::mode_fire_flicker_intense(void)
{
  return fire_flicker(1);
}

uint16_t WS2812FX::mode_bubble_sort(void)
{
  const uint16_t framedelay = map(SEG.beat88, (uint16_t)10000, (uint16_t)0, (uint16_t)0, (uint16_t)50) + map(SEG_RT.length, (uint16_t)300, (uint16_t)0, (uint16_t)0, (uint16_t)25);

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.bubble_sort.movedown = false;
    SEG_RT_MV.bubble_sort.ci = SEG_RT_MV.bubble_sort.co = SEG_RT_MV.bubble_sort.cd = 0;
     SEG_RT_MV.bubble_sort.hues[0] = random8();
    for (uint16_t i = 1; i < SEG_RT.length; i++)
    {
      SEG_RT_MV.bubble_sort.hues[i] = get_random_wheel_index(SEG_RT_MV.bubble_sort.hues[i - 1], 48);
    }
    map_pixels_palette(SEG_RT_MV.bubble_sort.hues, 32, SEG.blendType);
    SEG_RT_MV.bubble_sort.co = 0;
    SEG_RT_MV.bubble_sort.ci = 0;
    return STRIP_MIN_DELAY;
  }
  if (!SEG_RT_MV.bubble_sort.movedown)
  {
    if (SEG_RT_MV.bubble_sort.co <= SEG_RT.length)
    {
      if (SEG_RT_MV.bubble_sort.ci <= SEG_RT.length)
      {
        if (SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci] > SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.co])
        {
          uint8_t tmp = SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci];
          SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci] = SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.co];
          SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.co] = tmp;
          SEG_RT_MV.bubble_sort.cd = SEG_RT_MV.bubble_sort.ci;
          SEG_RT_MV.bubble_sort.movedown = true;
        }
        SEG_RT_MV.bubble_sort.ci++;
      }
      else
      {
        SEG_RT_MV.bubble_sort.co++;
        SEG_RT_MV.bubble_sort.ci = SEG_RT_MV.bubble_sort.co;
      }
    }
    else
    {
      if (_transition)
        SEG_RT.modeinit = true;
      return STRIP_MIN_DELAY;
    }
    map_pixels_palette(SEG_RT_MV.bubble_sort.hues, 32, SEG.blendType);
    leds[SEG_RT_MV.bubble_sort.ci + SEG_RT.start] = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci], _brightness, SEG.blendType); //CRGB::Green;
    leds[SEG_RT_MV.bubble_sort.co + SEG_RT.start] = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci], _brightness, SEG.blendType); // CRGB::Red;
  }
  else
  {
    map_pixels_palette(SEG_RT_MV.bubble_sort.hues, 32, SEG.blendType);
    leds[SEG_RT_MV.bubble_sort.co + SEG_RT.start] = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.ci], _brightness, SEG.blendType); // CRGB::Red;
    leds[SEG_RT_MV.bubble_sort.cd + SEG_RT.start] = ColorFromPaletteWithDistribution(_currentPalette, SEG_RT_MV.bubble_sort.hues[SEG_RT_MV.bubble_sort.cd], _brightness, SEG.blendType); // +=CRGB::Green;
    if (SEG_RT_MV.bubble_sort.cd == SEG_RT_MV.bubble_sort.co)
    {
      SEG_RT_MV.bubble_sort.movedown = false;
    }
    SEG_RT_MV.bubble_sort.cd--;
    return framedelay; // 0; //STRIP_MIN_DELAY;
  }
  return framedelay; //0; //STRIP_MIN_DELAY;
}

/* 
 * Fire with Palette
 */
uint16_t WS2812FX::mode_fire2012WithPalette(void)
{
  // Array of temperature readings at each simulation cell
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    memset(SEG_RT_MV.fire2012.heat, 0x0, SEG_RT.length * sizeof(byte));
  }

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < SEG_RT.length; i++)
  {
    SEG_RT_MV.fire2012.heat[i] = qsub8(SEG_RT_MV.fire2012.heat[i], random8(0, ((SEG.cooling * 10) / SEG_RT.length) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = SEG_RT.length - 1; k >= 2; k--)
  {
    SEG_RT_MV.fire2012.heat[k] = 
      (SEG_RT_MV.fire2012.heat[k - 1] + 
       SEG_RT_MV.fire2012.heat[k - 2] + 
       SEG_RT_MV.fire2012.heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SEG.sparking)
  {
    int y = random8(7);
    SEG_RT_MV.fire2012.heat[y] = qadd8(SEG_RT_MV.fire2012.heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < SEG_RT.length; j++)
  {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8(SEG_RT_MV.fire2012.heat[j], 240);
    CRGB color = ColorFromPalette(*_palettes[HEAT_PAL], colorindex);

    leds[j + SEG_RT.start] = color;
  }
  return STRIP_MIN_DELAY;
}

/*
 * TwinleFox Implementation
 */
uint16_t WS2812FX::mode_twinkle_fox(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
  }
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

  uint8_t backgroundBrightness = bg.getAverageLight();

  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16;         // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 = ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle(&myclock30, &myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if (deltabright >= 32 || (!bg))
    {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      leds[i + SEG_RT.start] = c;
    }
    else if (deltabright > 0)
    {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      leds[+SEG_RT.start] = blend(bg, c, deltabright * 8);
    }
    else
    {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      leds[i + SEG_RT.start] = bg;
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * SoftTwinkles
 */

CRGB WS2812FX::makeBrighter(const CRGB &color, fract8 howMuchBrighter)
{
  CRGB incrementalColor = color;
  incrementalColor.nscale8_video(howMuchBrighter);
  return color + incrementalColor;
}

CRGB WS2812FX::makeDarker(const CRGB &color, fract8 howMuchDarker)
{
  CRGB newcolor = color;
  newcolor.nscale8(255 - howMuchDarker);
  return newcolor;
}

bool WS2812FX::getPixelDirection(uint16_t i, uint8 *directionFlags)
{
  uint16_t index = i / 8;
  uint8_t bitNum = i & 0x07;

  uint8_t andMask = 1 << bitNum;
  return (directionFlags[index] & andMask) != 0;
}

void WS2812FX::setPixelDirection(uint16_t i, bool dir, uint8 *directionFlags)
{
  uint16_t index = i / 8;
  uint8_t bitNum = i & 0x07;

  uint8_t orMask = 1 << bitNum;
  uint8_t andMask = 255 - orMask;
  uint8_t value = directionFlags[index] & andMask;
  if (dir)
  {
    value += orMask;
  }
  directionFlags[index] = value;
}

void WS2812FX::brightenOrDarkenEachPixel(fract8 fadeUpAmount, fract8 fadeDownAmount, uint8_t *directionFlags)
{
  enum
  {
    GETTING_DARKER = 0,
    GETTING_BRIGHTER = 1
  };
  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    if (getPixelDirection(i, directionFlags) == 0)
    {
      // This pixel is getting darker
      leds[i] = makeDarker(leds[i], fadeDownAmount);
    }
    else
    {
      // This pixel is getting brighter
      leds[i] = makeBrighter(leds[i], fadeUpAmount);
      // now check to see if we've maxxed out the brightness
      if (leds[i].r == 255 || leds[i].g == 255 || leds[i].b == 255)
      {
        // if so, turn around and start getting darker
        setPixelDirection(i, 0, directionFlags);
      }
    }
  }
}

uint16_t WS2812FX::mode_softtwinkles(void)
{
  // Compact implementation of
  // the directionFlags array, using just one BIT of RAM
  // per pixel.  This requires a bunch of bit wrangling,
  // but conserves precious RAM.  The cost is a few
  // cycles and about 100 bytes of flash program memory.
  const uint16_t num_flags = (SEG_RT.length + 7)/8;

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    memset(SEG_RT_MV.soft_twinkle.directionFlags, 0x0, num_flags);
  }
  if (SEG_RT_MV.soft_twinkle.directionFlags == NULL)
    return STRIP_MIN_DELAY;

  uint16_t speed = (10001 - SEG.beat88 % 10000) / 250 + 1;
  EVERY_N_MILLISECONDS_I(timerObj, speed)
  {
    timerObj.setPeriod(speed);
    // Make each pixel brighter or darker, depending on
    // its 'direction' flag.
    brightenOrDarkenEachPixel(SEG.twinkleSpeed * 8 + 2, SEG.twinkleSpeed * 5 + 1, SEG_RT_MV.soft_twinkle.directionFlags);

    // Now consider adding a new random twinkle
    if (random8() < SEG.twinkleDensity * 32)
    {
      int pos = random16(SEG_RT.length);
      if (!leds[pos])
      {
        leds[pos] = ColorFromPaletteWithDistribution(_currentPalette, random8(), SEG.sparking / 2, SEG.blendType);
        setPixelDirection(pos, 1, SEG_RT_MV.soft_twinkle.directionFlags);
      }
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * Shooting Star...
 * 
 */
uint16_t WS2812FX::mode_shooting_star()
{

  uint16_t pos;

  #define SRMVSS SEG_RT_MV.shooting
  
  if (SEG_RT.modeinit || SRMVSS.basebeat != SEG.beat88)
  {
    SEG_RT.modeinit = false;
    SRMVSS.basebeat = SEG.beat88;
    for (uint8_t i = 0; i < SEG.numBars; i++)
    {
      SRMVSS.delta_b[i] = (65535 / SEG.numBars) * i;
      if (i > 0)
      {
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[i - 1], 32);
      }
      else
      {
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SEG.numBars - 1], 32);
      }
      SRMVSS.new_cind[i] = false;
    }
  }

  fadeToBlackBy(leds, SEG_RT.length > 8 ? SEG_RT.length - 8 : SEG_RT.length, (SEG.beat88 >> 8) | 0x60);
  if (SEG_RT.length > 8)
    blur1d(&leds[SEG_RT.stop - 7], 8, 120);

  for (uint8_t i = 0; i < SEG.numBars; i++)
  {
    uint16_t beat = beat88(SEG.beat88) + SRMVSS.delta_b[i];

    double_t q_beat = (beat / 100) * (beat / 100);
    pos = map(static_cast<uint32_t>(q_beat + 0.5), (uint32_t)0, (uint32_t)429484, (uint32_t)(SEG_RT.start * 16), (uint32_t)(SEG_RT.stop * 16));

    //we use the fractional bar and 16 brghtness values per pixel
    drawFractionalBar(pos, 2, _currentPalette, SRMVSS.cind[i], 255, true, 1);

    if (pos / 16 > (SEG_RT.stop - 4)) // 6
    {
      uint8_t tmp = 0;
      CRGB led = ColorFromPaletteWithDistribution(_currentPalette, SRMVSS.cind[i], _brightness, SEG.blendType); //leds[pos/16];
      if (led)
      {
        tmp = led.r | led.g | led.b;
        leds[pos / 16].addToRGB(tmp % 128);
      }

      SRMVSS.new_cind[i] = true;
    }
    else
    {
      if (SRMVSS.new_cind[i])
      {
        if (i > 0)
          SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[i - 1], 32);
        else
          SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SEG.numBars - 1], 32);
      }
      SRMVSS.new_cind[i] = false;
    }
  }
  #undef SRMVSS
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_beatsin_glow(void)
{
  #define SRMVGB SEG_RT_MV.beatsin
  const uint16_t lim = (SEG.beat88 * 10) / 50;

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    //_transition = true;
    //_blend = 0;
    
    for (uint8_t i = 0; i < SEG.numBars; i++)
    {
      SRMVGB.beats[i] = SEG.beat88 + lim / 2 - random16(lim);
      SRMVGB.theta[i] = (65535 / SEG.numBars) * i + (65535 / (4 * SEG.numBars)) - random16(65535 / (2 * SEG.numBars));
      uint8_t temp = random8(255 / (2 * SEG.numBars));
      if (temp & 0x01)
      {
        SRMVGB.cinds[i] = (255 / SEG.numBars) * i - temp;
      }
      else
      {
        SRMVGB.cinds[i] = (255 / SEG.numBars) * i + temp;
      }
      SRMVGB.times[i] = millis() + random8();
      SRMVGB.prev[i] = 0;
      SRMVGB.newval[i] = false;
    }
  }

  fadeToBlackBy(leds, SEG_RT.length, (SEG.beat88 >> 8) | 32);

  uint16_t pos = 0;

  for (uint8_t i = 0; i < SEG.numBars; i++)
  {
    uint16_t beatval = beat88(SRMVGB.beats[i], SRMVGB.times[i] + SRMVGB.theta[i]);
    int16_t si = sin16(beatval); // + theta[i]);

    if (si > -2 && si < 2 && SRMVGB.prev[i] < si) //si >= 32640 || si <= -32640)
    {
      const uint8_t rand_delta = 64;
      SRMVGB.beats[i] = SRMVGB.beats[i] + (SEG.beat88 * 10) / 50 - random16((SEG.beat88 * 10) / 25); //+= (random8(128)%2)?1:-1; // = beats[i] + (SEG.beat88*10)/200 - random16((SEG.beat88*10)/100); //
      if (SRMVGB.beats[i] < (SEG.beat88 / 2))
        SRMVGB.beats[i] = SEG.beat88 / 2;
      if (SRMVGB.beats[i] > (SEG.beat88 + SEG.beat88 / 2))
        SRMVGB.beats[i] = SEG.beat88 + SEG.beat88 / 2;
      SRMVGB.theta[i] = SRMVGB.theta[i] + (rand_delta / 2) - random8(rand_delta); //+= (random8(128)%2)?1:-1; // = theta[i] + 8-random8(16);  //
      SRMVGB.cinds[i] = SRMVGB.cinds[i] + (rand_delta / 2) - random8(rand_delta); //+= (random8(128)%2)?1:-1;
      SRMVGB.times[i] = millis() - SRMVGB.theta[i];

      SRMVGB.newval[i] = false;
    }
    else
    {
      SRMVGB.newval[i] = true;
    }

    SRMVGB.prev[i] = si;
    pos = map((65535 >> 1) + si, 0, 65535, SEG_RT.start * 16, SEG_RT.stop * 16);
    drawFractionalBar(pos, 2, _currentPalette, SRMVGB.cinds[i] + i * (255 / SEG.numBars), _brightness, true, 1);
  }
  #undef SRMVGB
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pixel_stack(void)
{
  #define SRMVPS SEG_RT_MV.pixel_stack
  // the beat88 translated to a effect speed (in relation to the number of segments)
  const uint16_t sp = map((uint16_t)(SEG.beat88>(20000/SEG.segments)?(20000/SEG.segments):SEG.beat88), (uint16_t)0, (uint16_t)(20000/SEG.segments), (uint16_t)0, (uint16_t)65535);
  // The number of leds for the effect (half of the strip)
  const uint16_t nLeds = SEG_RT.length / 2;
  // the base hue (the hue can move during the effect)
  const uint8_t sI = SEG_RT.baseHue;

  // init the effect
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;

    SRMVPS.up = true;
    SRMVPS.leds_moved = 0;
    SRMVPS.ppos16 = 0;
    
  }
  // We fade everything to black (a speed dependent amount) - leds will be rewritten later
  fadeToBlackBy(leds, SEG_RT.length, max(2, sp>>8));
  // write each active led with the color from the palette (without the one currently moving)
  for(uint16_t i = 0; i < nLeds; i++)
  {
    if(i<nLeds-SRMVPS.leds_moved)
    {
      // lower half (half the strip minus the number already moved)
      leds[i] = ColorFromPaletteWithDistribution(_currentPalette, sI + map(i, (uint16_t)0, (uint16_t)(nLeds-1), (uint16_t)0, (uint16_t)255), _brightness, SEG.blendType);
    }
    if(i < SRMVPS.leds_moved)
    {
      // upper half - the number of leds moved from the end
      leds[SEG_RT.length-1-i] = ColorFromPaletteWithDistribution(_currentPalette, sI + map((uint16_t)(nLeds - i), (uint16_t)0, (uint16_t)(nLeds-1), (uint16_t)0, (uint16_t)255), _brightness, SEG.blendType);
    }
  }
  // a value within the 0..65535 range (sawtooth) depending on the speed sp
  uint16_t p16 = beat88(sp);
  if(SRMVPS.up)
  {
    // mapping the p16 value to a (16 bit position) between the active leds
    uint16_t pos16 = map(p16, (uint16_t)0, (uint16_t)65535, (uint16_t)(16*(nLeds - SRMVPS.leds_moved)), (uint16_t)(16*(SEG_RT.length - 1 - SRMVPS.leds_moved)-16));
    // if the p16 starts from the beginning (sawtooth bottom)
    if(SRMVPS.ppos16 > pos16) 
    {
      SRMVPS.ppos16 = 0;
      // we moved all the leds
      if(SRMVPS.leds_moved == nLeds)
      {
        SRMVPS.leds_moved--;
        SRMVPS.up = false;
        SRMVPS.ppos16 = 65535;
      }
      SRMVPS.leds_moved++;
    }
    else // we are inbetween, we write the way up as fractional bar.
    { 
      // we start within an active led to avoid flicker
      if(pos16 > 16) pos16-=16;
      
      drawFractionalBar(pos16, 2, _currentPalette, sI + map(nLeds - SRMVPS.leds_moved, 0, nLeds-1, 0, 255), 255, true, 1);
      SRMVPS.ppos16 = pos16;
    }
  }
  else
  {
    uint16_t pos16 = map(p16, (uint16_t)0, (uint16_t)65535, (uint16_t)(16*(SEG_RT.length - 1 - SRMVPS.leds_moved)-16), (uint16_t)(16*(nLeds - SRMVPS.leds_moved)));
    if(SRMVPS.ppos16  < pos16) 
    {
      SRMVPS.ppos16  = 65535;
      if(SRMVPS.leds_moved == 0)
      {
        SRMVPS.leds_moved++;
        SRMVPS.up = true;
        SRMVPS.ppos16  = 0;
      }
      SRMVPS.leds_moved--;
      
    }
    else
    {
      drawFractionalBar(pos16, 2, _currentPalette, sI + map(nLeds - SRMVPS.leds_moved, 0, nLeds-1, 0, 255), 255, true, 1);
      SRMVPS.ppos16  = pos16;
    }
  }
  return STRIP_MIN_DELAY;// framedelay;
  #undef SRMVPS
}

uint16_t WS2812FX::mode_move_bar_sin(void)
{
  return mode_move_bar(0);
}
uint16_t WS2812FX::mode_move_bar_quad(void)
{
  return mode_move_bar(1);
}
uint16_t WS2812FX::mode_move_bar_cubic(void)
{
  return mode_move_bar(2);
}
uint16_t WS2812FX::mode_move_bar_sawtooth(void)
{
  return mode_move_bar(3);
}

uint16_t WS2812FX::mode_move_bar(uint8_t mode)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
  }
  const uint16_t width = SEG_RT.length/2;
  const uint16_t sp = map(SEG.beat88>(20000/SEG.segments)?(20000/SEG.segments):SEG.beat88, 0, (20000/SEG.segments), 0, 65535);


  //fadeToBlackBy(leds, SEG_RT.length, map(sp, 0, 65535, 0, 255)); 
  fadeToBlackBy(leds, SEG_RT.length, map(sp, (uint16_t)0, (uint16_t)65535, (uint16_t)64, (uint16_t)255)); 


  uint16_t pos16 = 0;
  switch (mode)
  {
  case 0:
    pos16 = beatsin16(sp/2, 0, (width*16));//beatsin16(sp/2, 0, (width)); //
    break;
  case 1:
    pos16 = map(ease16InOutQuad(triwave16(beat88(sp/2))), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(width*16)); //0, width); //
    break;
  case 2:
    pos16 = map(ease16InOutCubic(triwave16(beat88(sp/2))), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(width*16)); //0, width); //
    break;
  default:
    pos16 = map(triwave16(beat88(sp/2)), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(width*16)); //0, width); //
    break;
  }
  uint8_t inc = max(255/width, 1);
  drawFractionalBar(pos16, width, _currentPalette, SEG_RT.baseHue, 255, false, inc);

  //fill_palette(&leds[pos16], width, SEG_RT.baseHue, constrain(255/width, 1, 255), _currentPalette, 255, SEG.blendType);
  //blur1d(leds, LED_COUNT, map(sp, 0, 65535, 172, 32));
  

  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_popcorn(void)
{
  #define POPCMV SEG_RT_MV.pops
  #define LEDS_PER_METER 60
  // Length in 16 LEDs (close to 60 Leds per Meter in millimeters)
  // e.g. 60 leds / 1 meter ~~ (60*16 = 960 mm)
  // const double segLength = ((double)SEG_RT.length * 16.0); -> directly used in v0_max calc

  // Gravity 9.81 mm/ms² to 9810 mm/ms² (eq. earth 9.81 m/s²)
  const double gravity = -(map((double)SEG.beat88, 0.0, 10000.0, 9.810, 9810.0)/(1000.0*1000.0)); // mm/ms²
  
  // speed required to reach the end of the strip (segment). Everything lower will be before
  const double v0_max  = sqrt(-2.0*gravity*((double)SEG_RT.length * 16.0));
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    for(uint16_t i=0; i<SEG.numBars; i++)
    {
      POPCMV.pops[i].v0 = v0_max / (double)(i+2.0); 
      POPCMV.pops[i].color_index = get_random_wheel_index((255/SEG.numBars)*i, 32);
      POPCMV.pops[i].timebase = millis(); 
      POPCMV.pops[i].damp = 1; 
      POPCMV.pops[i].prev_pos = 0;
    }
  }

  fill_solid(leds, SEG_RT.length, CRGB::Black);

  double pos = 0;
  for(uint16_t i=0; i<SEG.numBars; i++)
  {
    double dT = (double)(millis()-POPCMV.pops[i].timebase);
    pos = (gravity/2 * dT + POPCMV.pops[i].v0)*dT;
    if(pos < 0)
    {
      if(SEG.damping < 100)
      {
        POPCMV.pops[i].v0 = POPCMV.pops[i].v0*((double)POPCMV.pops[i].damp/100.0)-0.02;  
      }
      POPCMV.pops[i].timebase = millis();
      if(POPCMV.pops[i].v0 < 0.01 && random8()<1)
      {
        POPCMV.pops[i].v0 = ((double)random8(80,100)/100.0) * v0_max;
        POPCMV.pops[i].color_index = get_random_wheel_index(POPCMV.pops[i].color_index, 32);
        POPCMV.pops[i].damp = SEG.damping<100?(uint8_t)((random8(90,100) * SEG.damping)/100):100;
        POPCMV.pops[i].prev_pos = 0;
      }
      pos=0;
    }
    uint8_t width = 1;
    uint16_t posInt = (uint16_t)pos;
    if(posInt > POPCMV.pops[i].prev_pos)
    {
      width = max((posInt - POPCMV.pops[i].prev_pos)/16, 1);
    }
    else
    {
      width = max((POPCMV.pops[i].prev_pos - posInt)/16, 1);
    }
    drawFractionalBar(posInt, width, _currentPalette,  POPCMV.pops[i].color_index, 255, true, 1);
    POPCMV.pops[i].prev_pos = posInt;
  }

  return STRIP_MIN_DELAY;
  #undef POPCMV
}

uint16_t WS2812FX::mode_firework2(void)
{
  #define FW2MV SEG_RT_MV.pops
  
  const uint16_t maxBlendWidth = SEG_RT.length/2>40?40:SEG_RT.length/2;
  const double segmentLength = ((double)(SEG_RT.length-maxBlendWidth/2) / (double)60) * (double)1000.0; // physical length in mm
  const double gravity = (double)SEG.beat88 / (double)-1019367.99184506;                                          // -0.00981; // gravity in mm per ms²
  const double v0_max = sqrt(-2 * gravity * segmentLength);


  uint32_t now = millis();

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;

    for (uint16_t i = 0; i < SEG.numBars; i++)
    {
      FW2MV.pops[i].timebase = now;
      FW2MV.pops[i].color_index = 256 / SEG.numBars * i; //get_random_wheel_index(pops[i].color_index);
      FW2MV.pops[i].pos = 0;
      FW2MV.pops[i].prev_pos = 0;
      FW2MV.pops[i].v0 = 0;
      FW2MV.pops[i].v = 0;
      //FW2MV.pops[i].ignite = false;
      //FW2MV.pops[i].P_ignite = random(100, 500);
      FW2MV.pops[i].v_explode = FW2MV.pops[i].v0 * ((double)random8(0, 10) / 100.0);
    }
  }
  fade_out(map(SEG.beat88, (uint16_t)0, (uint16_t)6000, (uint16_t)24, (uint16_t)255));
  for (uint16_t i = 0; i < SEG.numBars; i++) {
    if (FW2MV.pops[i].pos <= 0) {
      if (FW2MV.pops[i].v0 <= 0.01) {
        if (random8() < 2) {
          FW2MV.pops[i].v0 = (double)random((long)(v0_max * 850), (long)(v0_max * 990)) / 1000.0;
          FW2MV.pops[i].v =FW2MV.pops[i].v0;
          FW2MV.pops[i].pos = 0.00001;
          FW2MV.pops[i].prev_pos = 0;
          FW2MV.pops[i].timebase = now;
          FW2MV.pops[i].color_index = get_random_wheel_index(FW2MV.pops[i].color_index); // select a new color, different from the current one
          FW2MV.pops[i].brightness = random8(12, 48);
          FW2MV.pops[i].explodeTime = map(SEG.beat88, (uint16_t)1000, (uint16_t)6000, (uint16_t)80, (uint16_t)180);//60; //random(t_max * 3/5, t_max * 4/5);
          FW2MV.pops[i].v_explode = FW2MV.pops[i].v0 * ((double)random8(1, 50) / 100.0);
        }
      } else {
        float damping = 0.1f / 100.0f;
        if (SEG.damping) {
          if (SEG.damping <= 100) {
            damping = ((float)SEG.damping / 100.0f);
          } else {
            damping = 1.0f;
          }
        }

        FW2MV.pops[i].v0 = 0.001; //pops[i].v0 * damping;

        if (damping < 1.0f)FW2MV.pops[i].v0 -= 0.01;

        FW2MV.pops[i].v = FW2MV.pops[i].v0;
        FW2MV.pops[i].pos = 0.00001;
        FW2MV.pops[i].prev_pos = 0;
        FW2MV.pops[i].timebase = now;
      }
    } else {
      
      uint16_t pos = map((double)FW2MV.pops[i].pos, 0.0, (double)segmentLength, SEG_RT.start * 16.0, SEG_RT.stop * 16.0);
      if (FW2MV.pops[i].v > FW2MV.pops[i].v_explode) {
        if (pos != FW2MV.pops[i].prev_pos) {
          if (pos > FW2MV.pops[i].prev_pos) {
            uint16_t width = max((pos - FW2MV.pops[i].prev_pos) / 16, 2);
            drawFractionalBar(FW2MV.pops[i].prev_pos, width, HeatColors_p, 64, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
            //_currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
          } else {
            uint16_t width = max((FW2MV.pops[i].prev_pos - pos) / 16, 2);
            drawFractionalBar(pos, width, HeatColors_p, 64, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
            //_currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
          }
        } else {
          drawFractionalBar(pos, 2, HeatColors_p, 64, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
          //_currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
        }
      } else {
        uint16_t blendWidth = maxBlendWidth-3;
        if((SEG_RT.stop-pos/16)<blendWidth/2+3)
        {
          blendWidth = (SEG_RT.stop-pos/16) * 2;
        }
        if(pos/16 < blendWidth/2) blendWidth = pos/16 * 2;
        uint8_t br = map(FW2MV.pops[i].brightness, (uint8_t)0, (uint8_t)48, (uint8_t)0, (uint8_t)255);
        if ((FW2MV.pops[i].explodeTime > 10))//> 4 * STRIP_MIN_DELAY))
        {
          FW2MV.pops[i].explodeTime--;//; -=  (FW2MV.pops[i].explodeTime - 4 * STRIP_MIN_DELAY);//(12*STRIP_MIN_DELAY)/10;
          
          
          //leds[pos/16] += ColorFromPalette(_currentPalette, FW2MV.pops[i].color_index, 255, SEG.blendType);
          //leds[pos/16] += 0x202020;
          drawFractionalBar(pos, 3, _currentPalette, FW2MV.pops[i].color_index, 255, true,0);
          CRGB col = leds[pos/16] + CRGB(0x202020);
          drawFractionalBar(pos, 3, CRGBPalette16(col), 0, 255, true, 0);
          blur1d(&leds[pos/16+1], blendWidth, 172);  
          //leds[pos/16] += ColorFromPalette(_currentPalette, FW2MV.pops[i].color_index, 120, SEG.blendType);
          //leds[pos/16] += 0x202020r
          //blur1d(&leds[pos/16], blendWidth, 172);  
        }
        else if (FW2MV.pops[i].explodeTime)
        {
          FW2MV.pops[i].explodeTime--;//; -= (12*STRIP_MIN_DELAY)/10;//STRIP_MIN_DELAY;
          FW2MV.pops[i].brightness = qsub8(FW2MV.pops[i].brightness, 1);
          drawFractionalBar(pos, 3, _currentPalette, FW2MV.pops[i].color_index, br, true,0);
          drawFractionalBar(pos, 3, CRGBPalette16(0x202020), 0, br, true, 0);
          blur1d(&leds[pos/16], blendWidth, 128); 
          //fadeToBlackBy(&leds[pos/16 - blendWidth/4], blendWidth/2, 8);
          //FW2MV.pops[i].brightness = qsub8(FW2MV.pops[i].brightness, 1);
        }
        else
        {
          FW2MV.pops[i].brightness = qsub8(FW2MV.pops[i].brightness, 1);
          drawFractionalBar(pos, 3, _currentPalette, FW2MV.pops[i].color_index, br, true,0);
          drawFractionalBar(pos, 3, CRGBPalette16(0x202020), 0, br, true, 0);
          blur1d(&leds[pos/16], blendWidth, 64); 
          //fadeToBlackBy(&leds[pos/16 - blendWidth/4], blendWidth/2, 8);
        }
        /*
        if (pos != FW2MV.pops[i].prev_pos) {
          if (pos > FW2MV.pops[i].prev_pos) {
            uint16_t width = max((pos - FW2MV.pops[i].prev_pos) / 16, 1);
            drawFractionalBar(FW2MV.pops[i].prev_pos, width, _currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
          } else {
            uint16_t width = max((FW2MV.pops[i].prev_pos - pos) / 16, 1);
            drawFractionalBar(pos, width, _currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
          }
        } else {
          drawFractionalBar(pos, 2, _currentPalette, FW2MV.pops[i].color_index, FW2MV.pops[i].brightness/2, true, 1);// FW2MV.pops[i].color_index, _brightness, true);
        }
        */        
        
        if (!leds[pos / 16] && !leds[pos / 16 + 1])
        {
          FW2MV.pops[i].v0 = 0.001;
          FW2MV.pops[i].pos = 0;
          return STRIP_MIN_DELAY;
        }
      }

      FW2MV.pops[i].prev_pos = pos;

      double mtime = now - FW2MV.pops[i].timebase;

      if (mtime != 0)
      {
        FW2MV.pops[i].pos = (gravity / 2.0f) * (mtime * mtime) + FW2MV.pops[i].v0 * mtime;
        FW2MV.pops[i].v = gravity * mtime + FW2MV.pops[i].v0;
      }
      else
      {
        FW2MV.pops[i].v = 1000;
        FW2MV.pops[i].pos = 0.00001;
      }
    }
  }
  
  return STRIP_MIN_DELAY;
  #undef FW2MV
}

uint16_t WS2812FX::mode_void(void)
{
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG.autoplay = AUTO_MODE_OFF;

  }
  return STRIP_MIN_DELAY;
}

void WS2812FX::draw_sunrise_step(uint16_t sunriseStep)
{
  #define SRMVSR SEG_RT_MV.sunrise_step
  
  
  uint16_t step = sunriseStep;

  // Kind of dithering... lets see
  if(SRMVSR.toggle)
  {
    step +=1;
  }
  SRMVSR.toggle = !SRMVSR.toggle;
  
  fill_solid(leds, SEG_RT.length, calcSunriseColorValue(step));

  EVERY_N_MILLISECONDS(100)
  {
    for(uint16_t i=0; i<SEG_RT.length; i++)
    {
      SRMVSR.nc[i] = random8(0, 185);
    }
  }
  for (uint16_t i = 0; i < SEG_RT.length; i++)
  {
    CRGB col;
    col = leds[i];
    col.nscale8_video(SRMVSR.nc[i]);
    leds[i] = nblend(leds[i], col, 64);
  }
  #undef SRMVSR
}

uint16_t WS2812FX::getSunriseTimeToFinish(void)
{
  float time = (float)((SEG.sunrisetime * 60.0) / DEFAULT_SUNRISE_STEPS);
  if(getMode() == FX_MODE_SUNRISE)
  {
    return (uint16_t)(time * (DEFAULT_SUNRISE_STEPS - SEG_RT_MV.sunrise_step.sunRiseStep));
  }
  else if (getMode() == FX_MODE_SUNSET) 
  {
    return (uint16_t)(time * SEG_RT_MV.sunrise_step.sunRiseStep);
  }
  else
  {
    return 0;
  }
}

CRGB WS2812FX::calcSunriseColorValue(uint16_t step)
{
  double uv = 0.0;
  double red = 0.0;
  double green = 0.0;
  double blue = 0.0;
  double step_d = (double)step;
  if(step_d < SRSS_StartValue)
  {
    return CRGB(SRSS_StartR, SRSS_StartG, SRSS_StartB);
  }
  if(step_d > SRSS_Endvalue)
  {
    return CRGB(SRSS_EndR, SRSS_EndG, SRSS_EndB);
  }
  if(step_d <= SRSS_MidValue)
  {
    uv = (step_d - SRSS_StartValue) / (SRSS_MidValue - SRSS_StartValue);
    red =   (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_StartR) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid1R)  + 
                        (uv * uv                * SRSS_Mid2R))  + 0.5) / 100.0;
    green = (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_StartG) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid1G)  + 
                        (uv * uv                * SRSS_Mid2G))  + 0.5) / 100.0;
    blue =  (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_StartB) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid1B)  + 
                        (uv * uv                * SRSS_Mid2B))  + 0.5) / 100.0;
  }
  else if(step_d <= SRSS_Endvalue)
  {
    uv = (step_d - SRSS_MidValue) / (SRSS_Endvalue - SRSS_MidValue);
    red =   (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_Mid2R) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid3R)  + 
                        (uv * uv                * SRSS_EndR))  + 0.5) / 100.0;
    green = (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_Mid2G) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid3G)  + 
                        (uv * uv                * SRSS_EndG))  + 0.5) / 100.0;
    blue =  (100.0 * (
                       ((1.0 - uv) * (1.0 - uv) * SRSS_Mid2B) +
                        (2  * (1.0 - uv) * uv   * SRSS_Mid3B)  + 
                        (uv * uv                * SRSS_EndB))  + 0.5) / 100.0;
  }
  
  return CRGB((uint8_t)red, (uint8_t)green, (uint8_t)blue);
}

void WS2812FX::m_sunrise_sunset(bool isSunrise)
{
  #define SRMVSR SEG_RT_MV.sunrise_step
  const uint16_t sunriseSteps = DEFAULT_SUNRISE_STEPS;
  uint16_t stepInterval = (uint16_t)(SEG.sunrisetime * ((60 * 1000) / sunriseSteps));

  // We do not need background color during sunrise / sunset.... Lets try to clear these:
  _c_bck_b = 0;

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG.autoplay = AUTO_MODE_OFF;
    SRMVSR.next = millis();
    if (isSunrise)
    {
      SEG.targetBrightness = 255;
      SRMVSR.sunRiseStep = 0;
    }
    else
    {
      uint8_t  Luma       = 255;
      uint32_t LumaSum    = 0;
      SRMVSR.sunRiseStep  = sunriseSteps;
      for(uint16_t i=0; i<SEG_RT.length; i++)
      {
        LumaSum += leds[i].getLuma();
      }
      if(LumaSum)
      {
        Luma = (uint8_t)(LumaSum/SEG_RT.length);
      }
      if(Luma < 96) Luma = 96;
      for(uint16_t i=sunriseSteps; i>0; i--)
      {
        if(Luma > calcSunriseColorValue(i).getLuma())
        {
          SRMVSR.sunRiseStep = i;
          break;
        }
      }
    }
  }
  draw_sunrise_step(SRMVSR.sunRiseStep);
  if (millis() > SRMVSR.next)
  {
    SRMVSR.next = millis() + stepInterval;
    if (isSunrise)
    {
      if(SRMVSR.sunRiseStep < sunriseSteps)
      {
        SRMVSR.sunRiseStep++;
      }
    }
    else
    {
      if(SRMVSR.sunRiseStep > 0)
      {
        SRMVSR.sunRiseStep--;
      }
      else
      {
        // we switch off - this should fix issue #6
        setMode(DEFAULT_MODE);
        //setIsRunning(false);
        setPower(false);
      }
    }
  }
  #undef SRMVSR
}

uint16_t WS2812FX::mode_sunrise(void)
{
  m_sunrise_sunset(true);
  return 0; // should look better if we call this more often.... STRIP_MIN_DELAY;
}
uint16_t WS2812FX::mode_sunset(void)
{
  m_sunrise_sunset(false);
  return 0; // should look better if we call this more often.... STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_ring_ring(void)
{
  const uint16_t onTime  = 50; //(BEAT88_MAX + 10) - SEG.beat88;
  const uint16_t offTime = 100; //2*((BEAT88_MAX + 10) - SEG.beat88);
  const uint16_t runTime = 1500;
  const uint16_t pauseTime = 2000;
  SEG_RT_MV.ring_ring.now = millis();
  if(SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    SEG_RT_MV.ring_ring.isOn = true;
    SEG_RT_MV.ring_ring.nextmillis = 0; 
    SEG_RT_MV.ring_ring.pausemillis = SEG_RT_MV.ring_ring.now + 10;
    SEG_RT_MV.ring_ring.isPause = 0;
  }
  if(SEG_RT_MV.ring_ring.isPause)
  {
    fadeToBlackBy(leds, SEG_RT.length, 32);
    //fill_solid(leds, SEG_RT.length, CRGB::Black);
    if(SEG_RT_MV.ring_ring.now > (SEG_RT_MV.ring_ring.pausemillis + pauseTime))
    {
      SEG_RT_MV.ring_ring.pausemillis = SEG_RT_MV.ring_ring.now;
      SEG_RT_MV.ring_ring.isPause = false;
    }
  }
  else
  {
    if(SEG_RT_MV.ring_ring.isOn)
    {
      fill_palette(leds, SEG_RT.length, SEG_RT.baseHue, max(1, (256 * 100 / (SEG_RT.length * SEG.paletteDistribution))), _currentPalette, 255, SEG.blendType);
      if(SEG_RT_MV.ring_ring.now > (SEG_RT_MV.ring_ring.nextmillis + onTime))
      {
        SEG_RT_MV.ring_ring.nextmillis = SEG_RT_MV.ring_ring.now;
        SEG_RT_MV.ring_ring.isOn = false;
      }
    }
    else
    {
      fill_solid(leds, SEG_RT.length, CRGB::Black);
      if(SEG_RT_MV.ring_ring.now > (SEG_RT_MV.ring_ring.nextmillis + offTime))
      {
        SEG_RT_MV.ring_ring.nextmillis = SEG_RT_MV.ring_ring.now;
        SEG_RT_MV.ring_ring.isOn = true;
      }
    }
    if(SEG_RT_MV.ring_ring.now > (SEG_RT_MV.ring_ring.pausemillis + runTime))
    {
      SEG_RT_MV.ring_ring.pausemillis = SEG_RT_MV.ring_ring.now;
      SEG_RT_MV.ring_ring.isPause = true;
    }
  }
  
  return STRIP_MIN_DELAY;
}


/*
 *
 * Heartbeat Effect based on
 * https://github.com/kitesurfer1404/WS2812FX/blob/master/src/custom/Heartbeat.h
 * https://github.com/kitesurfer1404/WS2812FX/commit/abece9a2a5a23027243851767c55cb8d2e19ff05 
 */
uint16_t WS2812FX::mode_heartbeat(void) {
  #define M_HEARTBEAT_RT SEG_RT_MV.heartBeat
  
  if(SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    M_HEARTBEAT_RT.lastBeat = 0;
    M_HEARTBEAT_RT.secondBeatActive = false; 
    M_HEARTBEAT_RT.msPerBeat = (60000 / (SEG.beat88>20?SEG.beat88 / 20 : 1));
    M_HEARTBEAT_RT.secondBeat = (M_HEARTBEAT_RT.msPerBeat / 3);
    M_HEARTBEAT_RT.size = map(SEG_RT.length, (uint16_t)25, (uint16_t)300, (uint16_t)1, (uint16_t)6);
    M_HEARTBEAT_RT.centerOffset = (SEG_RT.length / 2);
    M_HEARTBEAT_RT.pCount = M_HEARTBEAT_RT.centerOffset - M_HEARTBEAT_RT.size;
  }

  for(uint16_t i = 0; i < M_HEARTBEAT_RT.pCount; i++)
  {
    leds[i] = leds[i + M_HEARTBEAT_RT.size];
    leds[i + M_HEARTBEAT_RT.centerOffset + M_HEARTBEAT_RT.size] = leds[i+M_HEARTBEAT_RT.centerOffset];
  }

  fadeToBlackBy(leds, SEG_RT.length, (SEG.beat88 >> 8) | 32);

  M_HEARTBEAT_RT.beatTimer = millis() - M_HEARTBEAT_RT.lastBeat;
  if((M_HEARTBEAT_RT.beatTimer > M_HEARTBEAT_RT.secondBeat) && !M_HEARTBEAT_RT.secondBeatActive) { // time for the second beat?
    // create the second beat
    fill_solid(&leds[SEG_RT.start + (SEG_RT.length / 2) - M_HEARTBEAT_RT.size], (M_HEARTBEAT_RT.size)*2, ColorFromPaletteWithDistribution(_currentPalette, SEG_RT.baseHue, _brightness, SEG.blendType));
    M_HEARTBEAT_RT.secondBeatActive = true;
  }
  if(M_HEARTBEAT_RT.beatTimer > M_HEARTBEAT_RT.msPerBeat) { // time to reset the beat timer?
    // create the first beat
    fill_solid(&leds[SEG_RT.start + (SEG_RT.length / 2) - M_HEARTBEAT_RT.size], (M_HEARTBEAT_RT.size)*2, ColorFromPaletteWithDistribution(_currentPalette, SEG_RT.baseHue, _brightness, SEG.blendType));
    M_HEARTBEAT_RT.secondBeatActive = false;
    M_HEARTBEAT_RT.lastBeat = millis();
  }
  return STRIP_MIN_DELAY;
  #undef M_HEARTBEAT_RT
}

uint16_t WS2812FX::mode_rain(void)
{
  #define M_RAIN_RT SEG_RT_MV.rain
  if(SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    memset(&(M_RAIN_RT.timebase), SEG.numBars, sizeof(M_RAIN_RT.timebase[0]) * SEG.numBars);
    memset(&(M_RAIN_RT.actives),  SEG.numBars, sizeof(M_RAIN_RT.actives[0])  * SEG.numBars);
    memset(&(M_RAIN_RT.cind),     SEG.numBars, sizeof(M_RAIN_RT.cind[0])     * SEG.numBars);
    M_RAIN_RT.timebase[0] = millis();
    M_RAIN_RT.actives[0] = true;
    M_RAIN_RT.cind[0] = random8();
  }
  EVERY_N_MILLIS(20)
  {
    fadeToBlackBy(leds, SEG_RT.length, map(SEG.beat88, (uint16_t)100, (uint16_t)7968, (uint16_t)3, (uint16_t) 255));
  }
  uint16_t pos16 = 0; 
  for(uint16_t i = 0; i<SEG.numBars; i++)
  {
    if(M_RAIN_RT.actives[i])
    {
      pos16 = map((uint16_t)(beat88(SEG.beat88*3, M_RAIN_RT.timebase[i])), (uint16_t)0, (uint16_t)65535, (uint16_t)(SEG_RT.stop*16), (uint16_t)(SEG_RT.start*16));
      drawFractionalBar(pos16, 4, _currentPalette, SEG_RT.baseHue + M_RAIN_RT.cind[i], 255, true, 0);
      if(!(pos16/16)) M_RAIN_RT.actives[i] = false;
    }
  }
  EVERY_N_MILLIS(100)
  { 
    const uint16_t minDistance = max(SEG_RT.length/12, 1);
    bool distBlack = true;
    for(uint16_t i = 0; i<minDistance; i++)
    {
      if(leds[SEG_RT.stop-i] != CRGB(0x000000))
      {
        distBlack = false;
      }
    }
    for(uint16_t i = 0; i<SEG.numBars; i++)
    {
      if(!M_RAIN_RT.actives[i] && !random8(4) && distBlack) //leds[SEG_RT.stop-10] == CRGB(0x000000)) 
      {
        M_RAIN_RT.actives[i]  = true;
        M_RAIN_RT.timebase[i] = millis();
        M_RAIN_RT.cind[i] = get_random_wheel_index(M_RAIN_RT.cind[i]);

        pos16 = map((uint16_t)(beat88(SEG.beat88*3, M_RAIN_RT.timebase[i])), (uint16_t)0, (uint16_t)65535, (uint16_t)(SEG_RT.stop*16), (uint16_t)(SEG_RT.start*16));
        drawFractionalBar(pos16, 4, _currentPalette, SEG_RT.baseHue + M_RAIN_RT.cind[i], 255, true, 0);
      }
    }
  }
  return STRIP_MIN_DELAY;
  #undef M_RAIN_RT
}

/*
 * A bar changing size, speed and position
 *
 *
 */ 

uint16_t WS2812FX::mode_ease_bar()
{
  const uint8_t minLeds = max(SEG_RT.length/4,10);
  uint8_t b1 = map(SEG.beat88, (uint16_t)BEAT88_MIN, (uint16_t)(BEAT88_MAX/2), (uint16_t)2, (uint16_t)(63));
  uint8_t b2 = map(SEG.beat88, (uint16_t)BEAT88_MIN, (uint16_t)(BEAT88_MAX/2), (uint16_t)3, (uint16_t)(111));
  uint16_t d1, d2;
  static uint8_t cnt = 0;
  

  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;

  }
     
  d1 = beatsin16(b1,0, BEAT88_MAX+BEAT88_MAX/5);
  d2 = beatsin16(b2,0, BEAT88_MAX+BEAT88_MAX/6);
  if(!d1) cnt++;

  uint8_t off = triwave8(cnt);

  uint8_t d3 = 255-beatsin8(max(b2-b1,1));

  uint16_t startLed = beatsin16(b1/2, SEG_RT.start, SEG_RT.stop-minLeds, 0, d1+off);

  uint16_t numLeds  = beatsin16(b2/2, minLeds, SEG_RT.length - startLed, 0, d2);

  numLeds = min(numLeds, (uint16_t)(SEG_RT.length-startLed));
  uint8_t incI = max(1, (256 * 100 / (SEG_RT.length * SEG.paletteDistribution)));
  uint8_t sInd = map(startLed, SEG_RT.start, SEG_RT.stop, (uint16_t)0, (uint16_t)255) + SEG_RT.baseHue; //beatsin8(SEG.beat88/128+1);
  fadeToBlackBy(&leds[SEG_RT.start], SEG_RT.length, 128);
  fill_palette(&leds[startLed], numLeds, sInd, incI, _currentPalette, 255, SEG.blendType);
  for(uint16_t i=SEG_RT.start; i<SEG_RT.stop; i++)
  {
    leds[i].nscale8(beatsin8(max((b2-b1)/2,1),128,255,0,d3+i));
  }

  return STRIP_MIN_DELAY;
}
// #32 pacifica implementation
// disco stobe removed as it looks bad...
uint16_t WS2812FX::mode_pacifica()
{
  #define M_PACIFIC_RT SEG_RT_MV.pacifica
  
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    M_PACIFIC_RT.sCIStart1 = random16();
    M_PACIFIC_RT.sCIStart2 = random16();
    M_PACIFIC_RT.sCIStart3 = random16();
    M_PACIFIC_RT.sCIStart4 = random16();
    M_PACIFIC_RT.sLastms = millis()-STRIP_MIN_DELAY;
  }
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.

  const uint16_t mbeat = max(SEG.beat88,(uint16_t)780);
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - M_PACIFIC_RT.sLastms;
  M_PACIFIC_RT.sLastms = ms;
  uint16_t speedfactor1 = beatsin16(max(mbeat/333,2), 179, 269);
  uint16_t speedfactor2 = beatsin16(max(mbeat/250,2), 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;

  M_PACIFIC_RT.sCIStart1 += (deltams1  * beatsin88(mbeat,10,13));
  M_PACIFIC_RT.sCIStart2 -= (deltams21 * beatsin88((mbeat*2)/3,8,11));
  M_PACIFIC_RT.sCIStart3 -= (deltams1  * beatsin88(mbeat/2,5,7));
  M_PACIFIC_RT.sCIStart4 -= (deltams2  * beatsin88(mbeat/4,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, SEG_RT.length, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_layer( pacifica_palette_p1, M_PACIFIC_RT.sCIStart1, beatsin16( max(mbeat/333,2), 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( max(mbeat/3,2)) );
  pacifica_layer( pacifica_palette_p2, M_PACIFIC_RT.sCIStart2, beatsin16( max(mbeat/240,2),  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( max((mbeat*2)/5,2)) );
  pacifica_layer( pacifica_palette_p3, M_PACIFIC_RT.sCIStart3, max(mbeat/166,2) * 256, beatsin8( 9, 10,38), 0-beat16(max(mbeat/2,2)));
  pacifica_layer( pacifica_palette_p3, M_PACIFIC_RT.sCIStart4, max(mbeat/200,2) * 256, beatsin8( 8, 10,28), beat16(max((mbeat*3)/5,2)));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();

  return STRIP_MIN_DELAY;
  #undef M_PACIFIC_RT
}

uint16_t WS2812FX::mode_color_waves()
{
  #define M_COLORWAVES_RT SEG_RT_MV.colorwaves
  
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    M_COLORWAVES_RT.sHue16 = 0;
    M_COLORWAVES_RT.sLastMillis = millis()-STRIP_MIN_DELAY;
    M_COLORWAVES_RT.sPseudotime = 0;
  }
  
 
  uint8_t brightdepth = beatsin88( max(SEG.beat88/3+5,7)/*341*/, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( max(SEG.beat88/4+2,5)/*203*/, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(max(SEG.beat88/6-9,3)/*147*/, 23, 60);

  uint16_t hue16 = M_COLORWAVES_RT.sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(max(SEG.beat88/8-12,2) /*113*/, 300, 1500);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - M_COLORWAVES_RT.sLastMillis ;
  M_COLORWAVES_RT.sLastMillis  = ms;
  M_COLORWAVES_RT.sPseudotime += deltams * msmultiplier;
  M_COLORWAVES_RT.sHue16 += deltams * beatsin88( max((SEG.beat88*4)/10,8)/*400*/, 5,9);
  uint16_t brightnesstheta16 = M_COLORWAVES_RT.sPseudotime;
  
  for( uint16_t i = 0 ; i < SEG_RT.length; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    uint8_t index = hue8;
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPaletteWithDistribution( _currentPalette, index, bri8, SEG.blendType);

    uint16_t pixelnumber = i;
    pixelnumber = (SEG_RT.length-1) - pixelnumber;
    
    nblend( leds[pixelnumber], newcolor, 128);
  }
  return STRIP_MIN_DELAY;
  #undef M_COLORWAVES_RT
}

uint16_t WS2812FX::mode_twinkle_map()
{
  #define M_TWINKLEMAP_RT SEG_RT_MV.twinklemap
  
  if (SEG_RT.modeinit)
  {
    SEG_RT.modeinit = false;
    memset8(M_TWINKLEMAP_RT.pixelstates, sizeof(M_TWINKLEMAP_RT.pixelstates), 0);
    for(uint16_t i=0; i<SEG_RT.length; i++)
    {
      uint8_t index = map(i, (uint16_t)0, (uint16_t)SEG_RT.length, (uint16_t)0, (uint16_t)255);
      leds[i] = (ColorFromPaletteWithDistribution(_currentPalette, index + SEG_RT.baseHue, 255, SEG.blendType)).nscale8_video(32);
    }
  }
  for(uint16_t i=0; i<SEG_RT.length; i++)
  {
    uint8_t index  = (uint8_t)map(i, (uint16_t)0, (uint16_t)SEG_RT.length, (uint16_t)0, (uint16_t)255);
    uint8_t speedu = ((uint8_t)map((uint16_t)SEG.beat88, (uint16_t)BEAT88_MIN, (uint16_t)BEAT88_MAX , (uint16_t)4, (uint16_t)64));
    uint8_t speedd = (speedu/2);
    CRGB baseColor = ColorFromPaletteWithDistribution(_currentPalette, index + SEG_RT.baseHue, 255, SEG.blendType).nscale8_video(32);
    CRGB peakColor = ColorFromPaletteWithDistribution(_currentPalette, index + SEG_RT.baseHue, 255, SEG.blendType).addToRGB(4);
    if(M_TWINKLEMAP_RT.pixelstates[i] == 0)
    {
      if( random8() < (SEG.twinkleDensity<3?1:SEG.twinkleDensity-2)) {
        M_TWINKLEMAP_RT.pixelstates[i] = 1;
      }
    }
    else if((M_TWINKLEMAP_RT.pixelstates[i]&0x01) == 0x01)
    {
      if( M_TWINKLEMAP_RT.pixelstates[i] == 255) {
        M_TWINKLEMAP_RT.pixelstates[i] = 2;
      } else {
        // otherwise, just keep brightening it:
        M_TWINKLEMAP_RT.pixelstates[i] = 0x01|(qadd8_lim(M_TWINKLEMAP_RT.pixelstates[i], speedu, 255));
        leds[i] = blend(baseColor, peakColor, M_TWINKLEMAP_RT.pixelstates[i]);
      }
    }
    else if((M_TWINKLEMAP_RT.pixelstates[i] & 0x01) == 0x00)
    {
      if( M_TWINKLEMAP_RT.pixelstates[i] == 254 ) {
        leds[i] = baseColor; // reset to exact base color, in case we overshot
        M_TWINKLEMAP_RT.pixelstates[i] = 0;
      } else {
        // otherwise, just keep dimming it down:
        leds[i] = blend(peakColor, baseColor, M_TWINKLEMAP_RT.pixelstates[i]);
        M_TWINKLEMAP_RT.pixelstates[i] = 0xFE&(qadd8_lim(M_TWINKLEMAP_RT.pixelstates[i], speedd, 254));
      }
    }
    //leds[i] = blend(baseColor, peakColor, beatsin88(SEG.beat88,0,255));
  }
  //return STRIP_MIN_DELAY; 
  return max(STRIP_MIN_DELAY, (uint32_t)(BEAT88_MAX-SEG.beat88)/1800);
  #undef M_TWINKLEMAP_RT
}

// Fallback function for effects that have been converted to class-based implementation
uint16_t WS2812FX::mode_class_based_fallback(void) {
    // This should not be called if the effect selection logic works correctly
    // Return minimum delay as a safe fallback
    return STRIP_MIN_DELAY;
}

// Effect system implementation
void WS2812FX::enableClassBasedEffects(bool enable) {
    if (_useClassBasedEffects != enable) {
        // Clean up current effect if switching away from class-based
        if (!enable && _currentEffect) {
            _currentEffect->cleanup();
            delete _currentEffect;
            _currentEffect = nullptr;
        }
        
        _useClassBasedEffects = enable;
        
        // Trigger mode reinitialization to load the appropriate effect
        _segment_runtime.modeinit = true;
    }
}