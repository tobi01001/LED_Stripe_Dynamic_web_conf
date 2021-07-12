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

#include "WS2812FX_FastLED.h"

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
#define SHADE01 0xF0
#define SHADE02 0x80
#define SHADE03 0x40
#define SHADE04 0x20
#define SHADE05 0x10
// Values
#define REDVAL(A) ((A << 16) & 0xff0000)
#define GREENVAL(A) ((A << 8) & 0x00ff00)
#define BLUEVAL(A) ((A << 0) & 0x0000ff)

// Shades of Red
const TProgmemRGBPalette16 Shades_Of_Red_p FL_PROGMEM = {
    REDVAL(SHADE01), REDVAL(SHADE02), REDVAL(SHADE03), REDVAL(SHADE04),
    REDVAL(SHADE05), CRGB::Black, CRGB::Black, REDVAL(SHADE04),
    REDVAL(SHADE03), REDVAL(SHADE02), REDVAL(SHADE01), CRGB::Black,
    CRGB::Black, REDVAL(SHADE02), REDVAL(SHADE03), CRGB::Black};

// Shades of Green
const TProgmemRGBPalette16 Shades_Of_Green_p FL_PROGMEM = {
    GREENVAL(SHADE01), GREENVAL(SHADE02), GREENVAL(SHADE03), GREENVAL(SHADE04),
    GREENVAL(SHADE05), CRGB::Black, CRGB::Black, GREENVAL(SHADE04),
    GREENVAL(SHADE03), GREENVAL(SHADE02), GREENVAL(SHADE01), CRGB::Black,
    CRGB::Black, GREENVAL(SHADE02), GREENVAL(SHADE03), CRGB::Black};

// Shades of Blue
const TProgmemRGBPalette16 Shades_Of_Blue_p FL_PROGMEM = {
    BLUEVAL(SHADE01), BLUEVAL(SHADE02), BLUEVAL(SHADE03), BLUEVAL(SHADE04),
    BLUEVAL(SHADE05), CRGB::Black, CRGB::Black, BLUEVAL(SHADE04),
    BLUEVAL(SHADE03), BLUEVAL(SHADE02), BLUEVAL(SHADE01), CRGB::Black,
    CRGB::Black, BLUEVAL(SHADE02), BLUEVAL(SHADE03), CRGB::Black};

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
  FastLED.show();      // We show once to write the Led data.

  RESET_RUNTIME;

  _segment_runtime.start = 0;
  _segment_runtime.stop = LED_COUNT - 1;
  _segment_runtime.length = LED_COUNT;

  _brightness = 255;

  bool isRunning = _segment.isRunning;
  bool power = _segment.power;

  // initialising segment
  setReverse              (_segment.reverse);
  setInverse              (_segment.inverse);
  setMirror               (_segment.mirror);
  setAutoplay             (_segment.autoplay);
  setAutopal              (_segment.autoPal);
  setBeat88               (_segment.beat88);
  setHuetime              (_segment.hueTime);
  setMilliamps            (_segment.milliamps);
  setAutoplayDuration     (_segment.autoplayDuration);
  setAutopalDuration      (_segment.autoPalDuration);
  setSegments             (_segment.segments);
  setCooling              (_segment.cooling);
  setSparking             (_segment.sparking);
  setTwinkleSpeed         (_segment.twinkleSpeed);
  setTwinkleDensity       (_segment.twinkleDensity);
  setNumBars              (_segment.numBars);
  setMode                 (_segment.mode);
  setMaxFPS               (_segment.fps);
  setDeltaHue             (_segment.deltaHue);
  setBlur                 (_segment.blur);
  setDamping              (_segment.damping);
  setDithering            (_segment.dithering);
  setSunriseTime          (_segment.sunrisetime);
  setTargetBrightness     (_segment.targetBrightness);
  setBlendType            (_segment.blendType);
  setColorTemp            (_segment.colorTemp);
  setTargetPaletteNumber  (_segment.targetPaletteNum);
  setCurrentPaletteNumber (_segment.currentPaletteNum);
  setAddGlitter           (_segment.addGlitter);
  setWhiteGlitter         (_segment.whiteGlitter);
  setOnBlackOnly          (_segment.onBlackOnly);
  setSynchronous          (_segment.synchronous);
  setBckndHue             (_segment.backgroundHue);
  setBckndSat             (_segment.backgroundSat);
  setBckndBri             (_segment.backgroundBri);

  #ifdef HAS_KNOB_CONTROL
  setWiFiDisabled         (_segment.wifiDisabled);
  #endif

  old_segs = 0;

  // should start with tranistion after init
  setTransition();
  setIsRunning(isRunning);
  setPower(power);

}

void WS2812FX::resetDefaults(void)
{
  

  _segment_runtime.start = 0;
  _segment_runtime.stop = LED_COUNT - 1;
  _segment_runtime.length = LED_COUNT;

  fill_solid(_bleds, LED_COUNT, CRGB::Black);
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.clear(true); // During init, all pixels should be black.
  FastLED.show();      // We show once to write the Led data.

  _brightness = 255;

  setIsRunning              (DEFAULT_RUNNING );
  setPower                  (DEFAULT_POWER);
  setReverse                (DEFAULT_REVERSE );
  setInverse                (DEFAULT_INVERTED );
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
  #ifdef HAS_KNOB_CONTROL
  setWiFiDisabled           (DEFAULT_WIFI_DISABLED);
  #endif
  FastLED.setBrightness     (DEFAULT_BRIGHTNESS);
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

  if ((_segment.segments != old_segs))
  {

    _segment_runtime.start = 0;
    _segment_runtime.length = (LED_COUNT / _segment.segments);
    _segment_runtime.stop = _segment_runtime.start + _segment_runtime.length - 1;
    if(_segment.numBars > ((LED_COUNT / _segment.segments) / MAX_NUM_BARS_FACTOR))
    {
      _segment.numBars = max(((LED_COUNT / _segment.segments) / MAX_NUM_BARS_FACTOR),1);
    }
    // 12.04.2019
    // There are artefacts remeianing if the distribution is not equal.
    // as we blend towards the new effect, we will remove the artefacts by clearing the leds array...
    fill_solid(leds, LED_COUNT, CRGB::Black);
    fill_solid(_bleds, LED_COUNT, CRGB::Black);
    
    setTransition();
    
    old_segs = _segment.segments;

    //_c_bck_b = 0;
    //_c_bck_h = 0;
    //_c_bck_s = 0;
  }

  if (_segment_runtime.modeinit)
  {
    fill_solid(leds, LED_COUNT, CRGB::Black);
    setTransition();
  }
  
  if (_segment.power)
  {
    if (_segment.isRunning || _triggered)
    {

      if (now > SEGMENT_RUNTIME.next_time || _triggered)
      {
        SEGMENT_RUNTIME.next_time = now + (uint32_t)(this->*_mode[SEGMENT.mode])();; 
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
    if (now > SEGMENT_RUNTIME.next_time || _triggered)
    {
      SEGMENT_RUNTIME.next_time = now + (uint32_t)(STRIP_DELAY_MICROSEC/1000);
      // no need to write data if nothing is shown (safeguard)
      
      // new version: Will fade rightaway once active led found.
      for(uint16_t i = 0; i < LED_COUNT; i++)
      {
        if(_bleds[i] || leds[i]) {
          fadeToBlackBy(_bleds, LED_COUNT, 16);
          fadeToBlackBy(  leds, LED_COUNT, 16);
          FastLED.show();
          return;
        }
      }
      
    }
    return;
  }
  
  
// check if we fade to a new FX mode.
#define MAXINVERSE 24
  uint8_t l_blend = _segment.blur; // to not overshoot during transitions we fade at max to "_segment.blur" parameter.
  if (_transition)
  {
    l_blend = _blend < _segment.blur ? _blend : _segment.blur;
    EVERY_N_MILLISECONDS(10)
    {
      // quickly blend from "old" to "new"
      _blend = qadd8(_blend, 1);
    }

    // reset once at max...
    // we could reset at _segment.blur as well
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
    uint8_t b = FastLED.getBrightness();
    if (_segment.targetBrightness > b)
    {
      FastLED.setBrightness(b + 1);
    }
    else if (_segment.targetBrightness < b)
    {
      FastLED.setBrightness(b - 1);
    }
  }

  
  // Thanks to some discussions on Github, I do still not use any memmove 
  // but I relaized that I need to nblend from the calculated frames to the led data.
  // this could be simplified within the following nested loop which does now all at once and saves 2 loops + 
  // one nblend over the complete strip data....
  // as the combination of "mirror" and "reverse" is a bit redundant, this could maybe be simplified as well (later)

  for (uint16_t j = 0; j < _segment.segments; j++)
  {
    for (uint16_t i = 0; i < _segment_runtime.length; i++)
    {
      if (_segment.mirror && (j & 0x01))
      {
        if(_segment.reverse)
        {
          nblend(_bleds[j * _segment_runtime.length + i], leds[i], l_blend);
        }
        else
        {
          nblend(_bleds[j * _segment_runtime.length + i], leds[_segment_runtime.stop - i], l_blend);
        }
      }
      else
      {
        if(_segment.reverse)
        {
          nblend(_bleds[j * _segment_runtime.length + i], leds[_segment_runtime.stop - i], l_blend);
        }
        else
        {
          nblend(_bleds[j * _segment_runtime.length + i], leds[i], l_blend);
        }
        
      }
    }
  }

  // Background Color: Good idea, but needs some improvement.
  // TODO: How to mix colors of different RGB values to remove the "glitch" when suddenly background switches to foreground.
  // --> Needs to be done without having to much backgroud at all.

  EVERY_N_MILLISECONDS(20)
  {
    if(_c_bck_b < _segment.backgroundBri)
      _c_bck_b++;
    else if (_c_bck_b > _segment.backgroundBri)
      _c_bck_b--;

    if(_c_bck_s < _segment.backgroundSat)
      _c_bck_s++;
    else if (_c_bck_s > _segment.backgroundSat)
      _c_bck_s--;
    
    if(_c_bck_h < _segment.backgroundHue)
      _c_bck_h++;
    else if (_c_bck_h > _segment.backgroundHue)
      _c_bck_h--;
  }

  CRGB BackGroundColor = CHSV(_c_bck_h, _c_bck_s, _c_bck_b);

  if(_segment.backgroundBri)
  {
    while(!BackGroundColor.getLuma())  BackGroundColor = CHSV(_c_bck_h, _c_bck_s, ++_c_bck_b); // 255);  // 0; //0x100000;
  }
  else
  {
    BackGroundColor = CRGB::Black;
  }
  

  if(_segment.backgroundBri)
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
  if(_segment.addGlitter)
  {
    addSparks(_segment.twinkleDensity, _segment.onBlackOnly, _segment.whiteGlitter, _segment.synchronous);
  }

  // Write the data

  // if there is time left for another service call, we do not write the led data yet...
  // but if there is less than 300 microseconds left, we do write..
  if(micros() < (last_show + STRIP_DELAY_MICROSEC - FRAME_CALC_WAIT_MICROINTERVAL))
  {
    // we have the time for another calc cycle and do nothing.
  }
  else
  {

    while(micros() < (last_show + STRIP_DELAY_MICROSEC)) {
      yield();
    } // just wait until time is "synced"
    
    // for FPS calculation
    _service_Interval_microseconds = (micros() - last_show);
    last_show = micros();
    // Write the data
    FastLED.show();
  }

  // May cause flicker at low speeds?
  /*
  // Fade to avoid artefacts...
  EVERY_N_MILLISECONDS(STRIP_MIN_DELAY) //(10)
  {
    if(_segment.isRunning) fadeToBlackBy(_bleds, LED_COUNT, 1);
  }
  */

  // every "hueTime" we set either the deltaHue (fixed offset)
  // or we increase the offset...
  if (now > SEGMENT_RUNTIME.nextHue)
  {
    if (!SEGMENT.hueTime)
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
    EVERY_N_MILLISECONDS(24)
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
  if (now > SEGMENT_RUNTIME.nextAuto)
  {
    if (!_transition)
    {
      nextMode(_segment.autoplay);
      SEGMENT_RUNTIME.nextAuto = now + SEGMENT.autoplayDuration * 1000;
    }
  }

  if (now > SEGMENT_RUNTIME.nextPalette)
  {
    if (!_transition)
    {
      nextPalette(_segment.autoPal);
      SEGMENT_RUNTIME.nextPalette = now + SEGMENT.autoPalDuration * 1000;
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
  _segment.isRunning = false;
  strip_off();
}

void WS2812FX::trigger()
{
  _triggered = true;
}

void WS2812FX::show()
{
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
  uint8_t old_hue;
  old_hue = hue[0];
  uint8_t delta = 0;
  while(delta < min_distance)
  {
    hue[0] = get_random_wheel_index(hue[15], min_distance);
    if(hue[0] < old_hue)
    {
      delta = old_hue - hue[0];
    }
    else
    {
      delta = hue[0] - old_hue;
    }
  }
  for (uint8_t i = 1; i < 16; i++)
  {
    old_hue = hue[i];
    delta = 0;
    while(delta < min_distance)
    {
      hue[i] = get_random_wheel_index(hue[i-1], min_distance);
      if(hue[i] < old_hue)
      {
        delta = old_hue - hue[i];
      }
      else
      {
        delta = hue[i] - old_hue;
      }
    }
  }
 return CRGBPalette16(
      CHSV(hue[0],  255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[1],  255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[2],  255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[3],  255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[4],  255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[5],  255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[6],  255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[7],  255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[8],  255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[9],  255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[10], 255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[11], 255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[12], 255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[13], 255, random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[14], 255, random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[15], 255, random8(RND_PAL_MIN_BRIGHT, 255)));
}
*/

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
    SEGMENT.colorTemp = Candle;
    break;
  case 1:
    SEGMENT.colorTemp = Tungsten40W;
    break;
  case 2:
    SEGMENT.colorTemp = Tungsten100W;
    break;
  case 3:
    SEGMENT.colorTemp = Halogen;
    break;
  case 4:
    SEGMENT.colorTemp = CarbonArc;
    break;
  case 5:
    SEGMENT.colorTemp = HighNoonSun;
    break;
  case 6:
    SEGMENT.colorTemp = DirectSunlight;
    break;
  case 7:
    SEGMENT.colorTemp = OvercastSky;
    break;
  case 8:
    SEGMENT.colorTemp = ClearBlueSky;
    break;
  /*
  case 9:
    SEGMENT.colorTemp = WarmFluorescent;
    break;
  case 10:
    SEGMENT.colorTemp = StandardFluorescent;
    break;
  case 11:
    SEGMENT.colorTemp = CoolWhiteFluorescent;
    break;
  case 12:
    SEGMENT.colorTemp = FullSpectrumFluorescent;
    break;
  case 13:
    SEGMENT.colorTemp = GrowLightFluorescent;
    break;
  case 14:
    SEGMENT.colorTemp = BlackLightFluorescent;
    break;
  case 15:
    SEGMENT.colorTemp = MercuryVapor;
    break;
  case 16:
    SEGMENT.colorTemp = SodiumVapor;
    break;
  case 17:
    SEGMENT.colorTemp = MetalHalide;
    break;
  case 18:
    SEGMENT.colorTemp = HighPressureSodium;
    break;
  */
  default:
    SEGMENT.colorTemp = UncorrectedTemperature;
    break;
  }
  FastLED.setTemperature(SEGMENT.colorTemp);
}

uint8_t WS2812FX::getColorTemp(void)
{
  switch (SEGMENT.colorTemp)
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


//String WS2812FX::getColorTempName(uint8_t index)
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
      newColor = ColorFromPalette(pal, cindex + (uint8_t)(n * incIndex), bright, SEGMENT.blendType);
    }
    else if (n == width)
    {
      // last pixel in the bar
      bright = lastpixelbrightness;
      newColor = ColorFromPalette(pal, cindex + (uint8_t)(n * incIndex), bright, SEGMENT.blendType);
    }
    else
    {
      // middle pixels
      bright = max_bright;
      mix = false;
      newColor = ColorFromPalette(pal, cindex + (uint8_t)(n * incIndex), bright, SEGMENT.blendType);
      if(incIndex)
      { 
        if(SEGMENT.blendType == LINEARBLEND)
        {
          CRGB prev_col = ColorFromPalette(pal, cindex + (uint8_t)((n-1) * incIndex), bright, SEGMENT.blendType);
          CRGB next_col = ColorFromPalette(pal, cindex + (uint8_t)((n+1) * incIndex), bright, SEGMENT.blendType);
          newColor = nblend(newColor, nblend(prev_col, next_col, firstpixelbrightness), 128);
        }
      }
    }

    if (i <= _segment_runtime.stop && i >= _segment_runtime.start)
    {
      if (mixColors || mix)
      {
        leds[i] |= newColor;
        //newColor = leds[i] | newColor;//ColorFromPalette(pal, cindex, bright, SEGMENT.blendType);
        // we blend based on the "baseBeat"
        //nblend(leds[i], newColor, qadd8(SEGMENT.beat88 >> 8, 24));
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
  _segment.isRunning = false;
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
  const uint16_t activeMax = ((_segment_runtime.length * prob)/100)    + 9;
  uint16_t active = 0;
  EVERY_N_MILLIS(10)
  {
    for(uint16_t i = 0; i<maxSparks; i++)
    {  
      sparks[i].r = qsub8(sparks[i].r, (1 + 10*_segment.twinkleSpeed)); 
      sparks[i].g = qsub8(sparks[i].g, (1 + 10*_segment.twinkleSpeed)); 
      sparks[i].b = qsub8(sparks[i].b, (1 + 10*_segment.twinkleSpeed)); 
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
        for(uint8_t j=0; j<_segment.segments; j++)
        {
          if (_segment.mirror && (j & 0x01))
          {
            _bleds[j * _segment_runtime.length + _segment_runtime.stop - pos[i]] = _bleds[pos[i]];
          }
          else
          {
            _bleds[j * _segment_runtime.length + pos[i]] =  _bleds[pos[i]];
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
          pos[i] = random16(_segment_runtime.start, _segment_runtime.stop);
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
          sparks[i] = ColorFromPalette(_currentPalette, random8(), br, _segment.blendType);
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
  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    leds[i + _segment_runtime.start] = ColorFromPalette(_currentPalette, hues[i], bright, blend);
  }
  return;
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
  //  uint8_t TWINKLE_SPEED = _twinkleSpeed; //map8(SEGMENT.beat88>>8, 2, 8);
  //  Overall twinkle density.
  //  0 (NONE lit) to 8 (ALL lit at once).
  //  Default is 5.
  //  #define TWINKLE_DENSITY _twinkleDensity //6

  uint16_t ticks = *ms >> (8 - SEGMENT.twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + *salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E) / 2) < SEGMENT.twinkleDensity)
  {
    bright = attackDecayWave8(fastcycle8);
  }

#define COOL_LIKE_INCANDESCENT 0

  uint8_t hue = slowcycle8 - *salt;
  CRGB c;
  if (bright > 0)
  {
    c = ColorFromPalette(_currentPalette, hue, bright, SEGMENT.blendType);
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
uint16_t WS2812FX::pride()
{
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.modevars.pride.sPseudotime = 0;
    
    SEGMENT_RUNTIME.modevars.pride.sLastMillis = 0;
    SEGMENT_RUNTIME.modevars.pride.sHue16 = 0;
  }

  uint8_t brightdepth = beatsin88(SEGMENT.beat88 / 3 + 1, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(SEGMENT.beat88 / 5 + 1, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(SEGMENT.beat88 / 7 + 1, 23, 60);

  uint16_t hue16 = SEGMENT_RUNTIME.modevars.pride.sHue16;
  uint16_t hueinc16 = beatsin88(SEGMENT.beat88 / 9 + 1, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - SEGMENT_RUNTIME.modevars.pride.sLastMillis;
  SEGMENT_RUNTIME.modevars.pride.sLastMillis = ms;
  SEGMENT_RUNTIME.modevars.pride.sPseudotime += deltams * msmultiplier;
  SEGMENT_RUNTIME.modevars.pride.sHue16 += deltams * beatsin88((SEGMENT.beat88 / 5) * 2 + 1, 5, 9);
  uint16_t brightnesstheta16 = SEGMENT_RUNTIME.modevars.pride.sPseudotime;

  for (uint16_t i = _segment_runtime.start; i < _segment_runtime.length; i++)
  {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = ColorFromPalette(_currentPalette, hue8, bri8, SEGMENT.blendType); //CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = (_segment_runtime.stop) - i;

    nblend(leds[pixelnumber], newcolor, 64);
  }
  return STRIP_MIN_DELAY;
}

/*
 * fade out function
 * fades out the current segment by dividing each pixel's intensity by 2
 */
void WS2812FX::fade_out(uint8_t fadeB = 32)
{
  fadeToBlackBy(&leds[_segment_runtime.start], _segment_runtime.length, fadeB);
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
  SEGMENT.blendType = t;
}

/*
 * Lets us toggle the Blend type
 */
void WS2812FX::toggleBlendType(void)
{
  SEGMENT.blendType == NOBLEND ? SEGMENT.blendType = LINEARBLEND : SEGMENT.blendType = NOBLEND;
}

/* 
 * Immediately change the cureent palette to 
 * the one provided - this will not blend to the new palette
 */
void WS2812FX::setCurrentPalette(CRGBPalette16 p, String Name = "Custom")
{
  _currentPalette = p;
  //_currentPaletteName = Name;
  _segment.currentPaletteNum = NUM_PALETTES;
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
  _segment.currentPaletteNum = n % NUM_PALETTES;
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
  _segment.targetPaletteNum = NUM_PALETTES;
}

/*
 * Set the palette we slowly fade/blend towards.
 * n: Number of the Palette to be chosen.
 */
void WS2812FX::setTargetPalette(uint8_t n = 0)
{
  if (n >= getPalCount())
  {
    n = 0;
  }
  if (n == RANDOM_PAL)
  {
    _targetPalette = getRandomPalette();
    //_targetPaletteName = _pal_name[n % NUM_PALETTES];
    _segment.targetPaletteNum = n % NUM_PALETTES;
    return;
  }
  _targetPalette = *(_palettes[n % NUM_PALETTES]);
  //_targetPaletteName = _pal_name[n % NUM_PALETTES];
  _segment.targetPaletteNum = n % NUM_PALETTES;
}

/*
 * Change to the mode being provided
 * m: mode number
 */
void WS2812FX::setMode(uint8_t m)
{
  static uint8_t segs = _segment.segments;
  _segment_runtime.modeinit = true;

  if (m == SEGMENT.mode)
    return; // not really a new mode...

  // make sure its a valid mode
  m = constrain(m, 0, MODE_COUNT - 1);

  if (_segment.mode == FX_MODE_VOID && m != FX_MODE_VOID)
  {
    _segment.segments = segs; // restore previous "segments";
  }

  if (!_transition && SEGMENT.mode != FX_MODE_VOID)
  {
    // if we are not currently in a transition phase
    // we clear the led array (the one holding the effect
    // the real LEDs are drawn from _bleds and blended to the leds)
    // we also clear only if we are outside the "void" mode where we do not touch the LED array.
    fill_solid(leds, _segment_runtime.length, CRGB::Black);
  }
  SEGMENT.mode = m;
  // start the transition phase
  _transition = true;
  _blend = 0;
  SEGMENT_RUNTIME.modeinit = true;
  setBlur(_pblur);
  
  if (m == FX_MODE_VOID)
  {
    segs = _segment.segments;
    _segment.segments = 1;
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

  if(_segment.mode >= FX_MODE_VOID)
  {
    return _segment.mode;
  }
  uint8_t newModefx = _segment.mode;
  switch (mode)
  {
  case AUTO_MODE_OFF:
    break;
  case AUTO_MODE_UP:
    if ((_segment.mode + 1) == (FX_MODE_VOID))
    {
      setMode(FX_MODE_STATIC);
    }
    else
    {
      setMode(_segment.mode + 1);
    }
    break;
  case AUTO_MODE_DOWN:
    if (_segment.mode == FX_MODE_STATIC)
    {
      setMode(FX_MODE_VOID - 1);
    }
    else
    {
      setMode(_segment.mode - 1);
    }
    break;
  case AUTO_MODE_RANDOM:
    while (newModefx == _segment.mode)
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
  uint16_t newSpeed = constrain(SEGMENT.beat88 + s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s)
{
  uint16_t newSpeed = constrain(SEGMENT.beat88 - s, BEAT88_MIN, BEAT88_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  setColor(CRGBPalette16(((uint32_t)r << 16) | ((uint32_t)g << 8) | b));
  setBrightness(_brightness);
}

void WS2812FX::setColor(CRGBPalette16 c)
{
  //SEGMENT.cPalette = c;
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
  _segment.targetBrightness = b;
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
    return SEGMENT.mode;
  }
}

uint16_t WS2812FX::getLength(void)
{
  return _segment_runtime.stop - _segment_runtime.start + 1;
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
    for(uint16_t i=0; i<_segment_runtime.length; i++)
    {
      if(/*leds[i] || */_bleds[i])
      {
        leds_on++;
      }
    }
    return (leds_on * _segment.segments);
  }
  return 0;
}

uint32_t WS2812FX::getCurrentPower(void)         
{ 
  double factor = (double)calculate_max_brightness_for_power_mW(FastLED.getBrightness(), _segment.milliamps*5) / 255.0;
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
  return ColorFromPalette(_currentPalette, p_index);
}

const __FlashStringHelper *WS2812FX::getModeName(uint8_t m)
{
  if (m < MODE_COUNT)
  {
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


/*
 * No blinking. Just plain old static light - but mapped on a color palette.
 * Palette ca be "moved" by SEGMENT.baseHue
 * will distribute the palette over the display length
 */
uint16_t WS2812FX::mode_static(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

  }
  fill_palette(&leds[_segment_runtime.start], _segment_runtime.length, SEGMENT_RUNTIME.baseHue, (_segment_runtime.length > 255 ? 1 : (256 / _segment_runtime.length)), _currentPalette, 255, SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles can additionally applied.
 */
uint16_t WS2812FX::mode_ease()
{
  // number of pixels for "antialised" (fractional) bar
  const uint8_t width = 3;
  // pixel position on the strip we make two out of it...
  uint16_t lerpVal = 0;

  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    // need to know if we are in the middle (to smoothly update random beat)
    SEGMENT_RUNTIME.modevars.ease.trigger = false;
    // beat being modified during runtime
    SEGMENT_RUNTIME.modevars.ease.beat = SEGMENT.beat88;
    // to check if beat88 recently changed
    // ToDo (idea) maybe a global runtime flag could help
    // which is recent by the active effect making use of the "beat"
    SEGMENT_RUNTIME.modevars.ease.oldbeat = SEGMENT.beat88;
    // to check if we have movement.
    // maybe easier but works good for now.
    SEGMENT_RUNTIME.modevars.ease.p_lerp = lerpVal;
    SEGMENT_RUNTIME.modevars.ease.timebase = millis();
  }
  // instead of moving the color around (palette wise)
  // we set it to the baseHue. So it can still be changed
  // and also change over time
  uint8_t colorMove = SEGMENT_RUNTIME.baseHue; //= quadwave8(map(beat88(max(SEGMENT.beat88/2,1),SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  // this is the fading tail....
  // we adjust it a bit on the speed (beat)
  fade_out(SEGMENT.beat88 >> 5);

  // now e calculate a sine curve for the led position
  // factor 16 is used for the fractional bar
  lerpVal = beatsin88(SEGMENT_RUNTIME.modevars.ease.beat, _segment_runtime.start * 16, _segment_runtime.stop * 16 - (width * 16), SEGMENT_RUNTIME.modevars.ease.timebase);

  // once we are in the middle
  // we can modify the speed a bit
  if (lerpVal == ((_segment_runtime.length * 16) / 2))
  {
    // the trigger is used because we are more frames in the middle
    // but only one should trigger
    if (SEGMENT_RUNTIME.modevars.ease.trigger)
    {
      // if the changed the base speed (external source)
      // we refesh the values
      if (SEGMENT_RUNTIME.modevars.ease.oldbeat != SEGMENT.beat88)
      {
        SEGMENT_RUNTIME.modevars.ease.beat = SEGMENT.beat88;
        SEGMENT_RUNTIME.modevars.ease.oldbeat = SEGMENT.beat88;
        //SEGMENT_RUNTIME.timebase = millis();
      }
      // reset the trigger
      SEGMENT_RUNTIME.modevars.ease.trigger = false;
      // tiimebase starts fresh in the middle (avoid jumping pixels)
      SEGMENT_RUNTIME.modevars.ease.timebase = millis();
      // we randomly increase or decrease
      // as we work with unsigned values we do this with an offset...
      // smallest value should be 255
      if (SEGMENT_RUNTIME.modevars.ease.beat < 255)
      {
        // avoid roll over to 65535
        SEGMENT_RUNTIME.modevars.ease.beat += 2 * random8();
      }
      else
      {
        // randomly increase or decrease beat
        SEGMENT_RUNTIME.modevars.ease.beat += 2 * (128 - random8());
      }
    }
  }
  else
  {
    // activate trigger if we are moving
    if (lerpVal != SEGMENT_RUNTIME.modevars.ease.p_lerp)
      SEGMENT_RUNTIME.modevars.ease.trigger = true;
  }

  SEGMENT_RUNTIME.modevars.ease.p_lerp = lerpVal;
  // we draw two fractional bars here. for the color mapping we need the overflow and therefore cast to uint8_t
  drawFractionalBar(lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal / 16 - _segment_runtime.start) + colorMove), 255, true, 1);
  drawFractionalBar((_segment_runtime.stop * 16) - lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal / 16 - _segment_runtime.start) + colorMove), 255, true, 1);

  return STRIP_MIN_DELAY;
}


uint16_t WS2812FX::mode_inoise8_mover(void)
{
  uint16_t xscale = _segment_runtime.length; //30;
  uint16_t yscale = 30;
  const uint16_t width = 6; //max(SEGMENT.beat88/256,1);
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.modevars.inoise.dist = 1234;
  }

  uint8_t locn = inoise8(xscale, SEGMENT_RUNTIME.modevars.inoise.dist + yscale);
  uint16_t pixlen = map((uint16_t)locn, (uint16_t)0, (uint16_t)255, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16));

  uint8_t colormove = SEGMENT_RUNTIME.baseHue; // quadwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  fade_out(48);

  drawFractionalBar(pixlen, width, _currentPalette, (uint8_t)((uint8_t)(pixlen / 64) + colormove), 255, true, 1); //, beatsin88(max(SEGMENT.beat88/2,1),200 % _brightness, _brightness, SEGMENT_RUNTIME.timebase));

  SEGMENT_RUNTIME.modevars.inoise.dist += beatsin88(SEGMENT.beat88, 1, 12, SEGMENT_RUNTIME.modevars.inoise.timebase);

  return STRIP_MIN_DELAY;
}

/*
 * Plasma like Effect over the complete strip.
 */
uint16_t WS2812FX::mode_plasma(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.plasma.timebase = millis();
  }

  uint8_t thisPhase = beatsin88(SEGMENT.beat88, 0, 255, _segment_runtime.modevars.plasma.timebase);             // Setting phase change for a couple of waves.
  uint8_t thatPhase = beatsin88((SEGMENT.beat88 * 11) / 10, 0, 255, _segment_runtime.modevars.plasma.timebase); // was int thatPhase = 64 - beatsin88((SEGMENT.beat88*11)/10, 0, 128, SEGMENT_RUNTIME.timebase);

  for (int k = _segment_runtime.start; k < _segment_runtime.stop; k++)
  { // For each of the LED's in the strand, set a brightness based on a wave as follows:

    uint8_t colorIndex = cubicwave8((k * 15) + thisPhase) / 2 + cos8((k * 8) + thatPhase) / 2 + SEGMENT_RUNTIME.baseHue; // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    uint8_t thisBright = qsuba(colorIndex, beatsin88((SEGMENT.beat88 * 12) / 10, 0, 128));                               // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
    CRGB newColor = ColorFromPalette(_currentPalette, colorIndex, thisBright, SEGMENT.blendType);                        // Let's now add the foreground colour.
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

  const uint8_t width = max(_segment_runtime.length / 15, 3);
  uint8_t curhue = 0;
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.modevars.juggle.thishue = 0;
    SEGMENT_RUNTIME.modevars.juggle.timebase = millis();
  }
  curhue = SEGMENT_RUNTIME.modevars.juggle.thishue; // Reset the hue values.
  EVERY_N_MILLISECONDS(100)
  {
    SEGMENT_RUNTIME.modevars.juggle.thishue = random8(curhue, qadd8(curhue, 8));
  }

  fade_out(96);

  for (int i = 0; i < SEGMENT.numBars; i++)
  {
    uint16_t pos = beatsin88(max(SEGMENT.beat88 / 2, 1) + i * (_segment.beat88 / _segment.numBars), _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16, SEGMENT_RUNTIME.modevars.juggle.timebase);
    drawFractionalBar(pos, width, _currentPalette, curhue + (255 / SEGMENT.numBars) * i, 255, true, 1);
    uint8_t delta = random8(9);
    if (delta < 5)
    {
      curhue = curhue - (uint8_t)(delta) + SEGMENT_RUNTIME.baseHue;
    }
    else
    {
      curhue = curhue + (uint8_t)(delta / 2) + SEGMENT_RUNTIME.baseHue;
    }
  }
  return STRIP_MIN_DELAY;
}

/*
 * Fills the strip with waving color and brightness
 */
uint16_t WS2812FX::mode_fill_beat(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.fill_beat.timebase = millis();
  }
  CRGB newColor = CRGB::Black;
  uint8_t br, index;
  for (uint8_t k = _segment_runtime.start; k < _segment_runtime.stop; k++)
  {

    br = beatsin88(SEGMENT.beat88, 20, 255, SEGMENT_RUNTIME.modevars.fill_beat.timebase, k * 2); //= quadwave8(v1);
    index = (uint8_t)((uint8_t)triwave8(beat8(SEGMENT.beat88 >> 8) +
                                        (uint8_t)beatsin8(SEGMENT.beat88 >> 8, 0, 20) +
                                        (uint8_t)map((uint16_t)k, (uint16_t)_segment_runtime.start, (uint16_t)_segment_runtime.stop, (uint16_t)0, (uint16_t)255)));
    newColor = ColorFromPalette(_currentPalette, index, br, SEGMENT.blendType);

    leds[k] = nblend(leds[k], newColor, qadd8(SEGMENT.beat88 >> 8, 24));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Wave Effect over the complete strip.
 */
uint16_t WS2812FX::mode_fill_wave(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.fill_wave.timebase = millis();
  }
  fill_palette(&leds[_segment_runtime.start],
               (_segment_runtime.length),
               SEGMENT_RUNTIME.baseHue + (uint8_t)beatsin88(SEGMENT.beat88 * 2, 0, 255, _segment_runtime.modevars.fill_wave.timebase),
               // SEGMENT_RUNTIME.baseHue + triwave8( (uint8_t)map( beat88( max(  SEGMENT.beat88/4, 2), SEGMENT_RUNTIME.timebase), 0,  65535,  0,  255)),
               max(255 / _segment_runtime.length + 1, 1),
               _currentPalette,
               (uint8_t)beatsin88(max(SEGMENT.beat88 * 1, 1),
                                  _brightness / 10, 255,
                                  _segment_runtime.modevars.fill_wave.timebase),
               SEGMENT.blendType);
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
  #define SRMVDB _segment_runtime.modevars.dot_beat
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;

    SRMVDB.oldVal = SEGMENT.beat88;
    uint32_t tb = millis();
    for(uint8_t i=0; i< SEGMENT.numBars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3, 6)), SEGMENT.beat88);
      SRMVDB.timebases[i] = tb;
      SRMVDB.newBase[i] = false;
      SRMVDB.coff[i] = random8(i*(255/SEGMENT.numBars), (i+1)*SEGMENT.numBars);
    }
  }

  if (SRMVDB.oldVal != SEGMENT.beat88)
  {
    SRMVDB.oldVal = SEGMENT.beat88;
    for(uint8_t i = 0; i<SEGMENT.numBars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3, 6)), SEGMENT.beat88);
    }
  }

  uint16_t cled = 0;
  const uint8_t width = 3; //max(_segment_runtime.length/15, 2);

  fade_out(fade);

  for (uint8_t i = 0; i < SEGMENT.numBars; i++)
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
    cled = map(cled, (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16));

    if (cled == _segment_runtime.start * 16)
    {
      if (SRMVDB.newBase[i])
      {
        SRMVDB.timebases[i] = millis();
        SRMVDB.newBase[i] = false;
      }
      SRMVDB.beats[i] = max((uint16_t)(SRMVDB.beats[i] + (int16_t)((int16_t)256 - (int16_t)random16(0, 512))), SEGMENT.beat88);

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

    cind = SRMVDB.coff[i]; // + map(cled/16, _segment_runtime.start, _segment_runtime.stop , 0, 255);

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

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.col_wipe.npos = get_random_wheel_index(0, 32);
    _segment_runtime.modevars.col_wipe.pnpos = get_random_wheel_index(_segment_runtime.modevars.col_wipe.npos, 32);
    _segment_runtime.modevars.col_wipe.prev = _segment_runtime.start;
    _segment_runtime.modevars.col_wipe.newcolor = true;
    _segment_runtime.modevars.col_wipe.up = true;
    _segment_runtime.modevars.col_wipe.timebase = millis();
  }

  switch (mode)
  {
  case 0:
    i = beatsin16((SEGMENT.beat88 * 2) % 65535, 0, 65535,  _segment_runtime.modevars.col_wipe.timebase);
    break;
  case 1:
    i = triwave16(beat88((SEGMENT.beat88 * 2) % 65535,  _segment_runtime.modevars.col_wipe.timebase));
    break;
  case 2:
    i = quadwave16(beat88((SEGMENT.beat88 * 2) % 65535,  _segment_runtime.modevars.col_wipe.timebase));
    break;
  case 3:
    i = beat88((SEGMENT.beat88 * 4) % 65535,  _segment_runtime.modevars.col_wipe.timebase);
    break;
  default:
    i = _segment_runtime.start;
    fill_solid(leds, _segment_runtime.length, CRGB::Black);
  }
  i = map((uint16_t)i, (uint16_t)0, (uint16_t)65535, (uint16_t)_segment_runtime.start, (uint16_t)(_segment_runtime.stop + 2));

  if (i >= _segment_runtime.stop)
    i = _segment_runtime.stop;

  if((( _segment_runtime.modevars.col_wipe.up && i < _segment_runtime.modevars.col_wipe.prev) || 
     (!_segment_runtime.modevars.col_wipe.up && i > _segment_runtime.modevars.col_wipe.prev))) // direction changed - we've been at the bottom
  {
    _segment_runtime.modevars.col_wipe.up = !_segment_runtime.modevars.col_wipe.up;
    SEGMENT_RUNTIME.modevars.col_wipe.newcolor = true;
  }
  if (SEGMENT_RUNTIME.modevars.col_wipe.newcolor)
  {
    _segment_runtime.modevars.col_wipe.pnpos = _segment_runtime.modevars.col_wipe.npos;
    _segment_runtime.modevars.col_wipe.npos = get_random_wheel_index(_segment_runtime.modevars.col_wipe.npos, 32);
    SEGMENT_RUNTIME.modevars.col_wipe.newcolor = false;
  }

  CRGB Col1 = ColorFromPalette(_currentPalette, _segment_runtime.modevars.col_wipe.npos  + _segment_runtime.baseHue, _brightness, SEGMENT.blendType);
  CRGB Col2 = ColorFromPalette(_currentPalette, _segment_runtime.modevars.col_wipe.pnpos + _segment_runtime.baseHue, _brightness, SEGMENT.blendType);


  if(_segment_runtime.modevars.col_wipe.up)
  {
    if(i > 0)
    { 
      fill_solid(leds, i, Col1);  
    }
    if(i < _segment_runtime.stop)
    { 
      fill_solid(&leds[i], _segment_runtime.stop-i, Col2);
    }
  }
  else
  {
    if(i<_segment_runtime.stop)
    { 
      fill_solid(&leds[i], _segment_runtime.stop-i, Col1);
    }
    if(i>0)
    { 
      fill_solid(leds, i, Col2);  
    }
  }
  _segment_runtime.modevars.col_wipe.prev = i;

  return STRIP_MIN_DELAY;
}

/*
 * Pulsing to the inner middle from both ends..
 */
uint16_t WS2812FX::mode_to_inner(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.to_inner.timebase = millis();
  }
  uint16_t led_up_to = (((_segment_runtime.length) / 2 + 1) + _segment_runtime.start);
  uint8_t fade = SEGMENT.beat88 * 5 <= 16320 ? (SEGMENT.beat88 * 5) >> 6 : 255;
 // SEGMENT.blur = max(fade, (uint8_t)16);
  fade_out(max(fade, (uint8_t)16)); //(64);

  fill_palette(&leds[_segment_runtime.start],
               beatsin88(
                   SEGMENT.beat88 < 13107 ? SEGMENT.beat88 * 5 : 65535,
                   0, led_up_to, _segment_runtime.modevars.to_inner.timebase),
               SEGMENT_RUNTIME.baseHue, 5, _currentPalette, 255, SEGMENT.blendType);
  for (uint16_t i = (_segment_runtime.length) - 1; i >= ((_segment_runtime.length) - led_up_to); i--)
  {
    if (((_segment_runtime.length) - i) >= 0 && ((_segment_runtime.length) - i) < (_segment_runtime.length))
    {
      leds[i] = leds[(_segment_runtime.length) - i];
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.breath.timebase = millis();
  }
  fill_palette(&leds[_segment_runtime.start], _segment_runtime.length, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, beatsin88(SEGMENT.beat88 * 2, 10, 255, _segment_runtime.modevars.breath.timebase), SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void)
{
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.modevars.multi_dyn.last = 0;
  }
  if (millis() > SEGMENT_RUNTIME.modevars.multi_dyn.last)
  {

    for (uint16_t i = _segment_runtime.start; i <= _segment_runtime.stop; i++)
    {

      SEGMENT_RUNTIME.modevars.multi_dyn.last_index = get_random_wheel_index(SEGMENT_RUNTIME.modevars.multi_dyn.last_index, 32);
      leds[i] = ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.modevars.multi_dyn.last_index, _brightness, SEGMENT.blendType);
    }
    SEGMENT_RUNTIME.modevars.multi_dyn.last = millis() + ((BEAT88_MAX - SEGMENT.beat88) >> 6);
  }

  return STRIP_MIN_DELAY;
}

/*
 * Waving brightness over the complete strip.
 */
uint16_t WS2812FX::mode_fill_bright(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.fill_bright.timebase = millis();
  }
  fill_palette(&leds[_segment_runtime.start], (_segment_runtime.length), beat88(max((SEGMENT.beat88 / 128), 2), _segment_runtime.modevars.fill_bright.timebase),
               max(255 / _segment_runtime.length + 1, 1), _currentPalette, beatsin88(max(SEGMENT.beat88 / 32, 1), 10, 255, _segment_runtime.modevars.fill_bright.timebase), SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_firework(void)
{
  #define FW1MV _segment_runtime.modevars.firework
  const uint8_t dist = max(_segment_runtime.length / 20, 2);
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    fill_solid(leds, LED_COUNT, CRGB::Black);
    memset(FW1MV.fireworks, 0x0, MAX_NUM_BARS * sizeof(uint16_t));
    memset(FW1MV.colIndex,  0x0, MAX_NUM_BARS * sizeof( uint8_t));
    memset(FW1MV.isBurning, 0x0, MAX_NUM_BARS * sizeof( uint8_t));
    //memset(FW1MV.colors, 0x0, _segment_runtime.length * sizeof(uint8_t));
    //memset(FW1MV.keeps, 0x0, _segment_runtime.length * sizeof(uint8_t));
  }

  blur1d(&leds[_segment_runtime.start], _segment_runtime.length, qadd8(255-(SEGMENT.beat88 >> 8), 32)%172); // 172); //qadd8(255-(SEGMENT.beat88 >> 8), 32)%172); //was 2 instead of 16 before!

  for (uint16_t i = 0; i<MAX_NUM_BARS; i++)
  {
    if (FW1MV.isBurning[i])
    {
      FW1MV.isBurning[i]--;
      nblend(leds[FW1MV.fireworks[i]], ColorFromPalette(_currentPalette, FW1MV.colIndex[i], 255, SEGMENT.blendType), 196);
      //leds[i] = ColorFromPalette(_currentPalette, colors[i]  , 255, SEGMENT.blendType);
    }
  }

  if (random8(max(6, _segment_runtime.length / 7)) <= max(3, _segment_runtime.length / 14))
  {
    uint16_t lind = random16(dist + _segment_runtime.start, _segment_runtime.stop - dist);
    for (int8_t i = 0 - dist; i <= dist; i++)
    {
      if (lind + i >= _segment_runtime.start && lind + i < _segment_runtime.stop)
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
      leds[lind] = ColorFromPalette(_currentPalette, FW1MV.colIndex [barPos], random8(192, 255), SEGMENT.blendType);
    }
  }

  return STRIP_MIN_DELAY; // (BEAT88_MAX - SEGMENT.beat88) / 256; // STRIP_MIN_DELAY;
}

/*
 * Fades the LEDs on and (almost) off again.
 */
uint16_t WS2812FX::mode_fade(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.fade.timebase = millis();
  }

  fill_palette(&(leds[_segment_runtime.start]), _segment_runtime.length, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, map8(triwave8(map(beat88(SEGMENT.beat88 * 10,  _segment_runtime.modevars.fade.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255)), 24, 255), SEGMENT.blendType);

  return STRIP_MIN_DELAY;
}

/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
     _segment_runtime.modevars.scan.timebase = millis();
  }
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, _segment_runtime.start*16, _segment_runtime.stop*16);
  const uint16_t width = 2; // max(2, _segment_runtime.length/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88,  _segment_runtime.modevars.scan.timebase)), (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16));

  // maybe we change this to fade?
  fill_solid(&(leds[_segment_runtime.start]), _segment_runtime.length, CRGB(0, 0, 0));

  drawFractionalBar(_segment_runtime.start * 16 + led_offset, width, _currentPalette, led_offset / 16 + SEGMENT_RUNTIME.baseHue, 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
     _segment_runtime.modevars.dual_scan.timebase = millis();
  }
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, _segment_runtime.start*16, _segment_runtime.stop*16);
  const uint16_t width = 2; // max(2, _segment_runtime.length/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88,  _segment_runtime.modevars.dual_scan.timebase)), (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16));

  fill_solid(&(leds[_segment_runtime.start]), _segment_runtime.length, CRGB(0, 0, 0));

  drawFractionalBar(_segment_runtime.stop * 16 - led_offset, width, _currentPalette, 255 - led_offset / 16 + SEGMENT_RUNTIME.baseHue, 255, true, 1);
  drawFractionalBar(_segment_runtime.start * 16 + led_offset, width, _currentPalette, led_offset / 16 + SEGMENT_RUNTIME.baseHue, 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.rainbow.timebase = millis();
  }

  fill_solid(&leds[_segment_runtime.start], _segment_runtime.length, ColorFromPalette(_currentPalette, map(beat88(SEGMENT.beat88, _segment_runtime.modevars.rainbow.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255), _brightness, SEGMENT.blendType)); /*CHSV(beat8(max(SEGMENT.beat/2,1), SEGMENT_RUNTIME.timebase)*/ //_brightness));
  //SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 2) & 0xFF;
  return STRIP_MIN_DELAY;
}

/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.rainbow_cycle.timebase = millis();
  }

  fill_palette(&leds[_segment_runtime.start],
               _segment_runtime.length,
               map(beat88(SEGMENT.beat88,
                           _segment_runtime.modevars.rainbow_cycle.timebase),
                   (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255),
               (_segment_runtime.length > 255 ? 1 : (256 / _segment_runtime.length)),
               _currentPalette,
               255, SEGMENT.blendType);

  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pride(void)
{
  return pride();
}

/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(CRGB color)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.theater_chase.timebase = millis();
  }
  uint16_t off = map(beat88(SEGMENT.beat88 / 2,  _segment_runtime.modevars.theater_chase.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255) % 3;

  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    
    if ((i % 3) == off)
    {
      leds[_segment_runtime.start + i] = color;
    }
    else
    {
      leds[_segment_runtime.start + i] = CRGB::Black;
    }
  }
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::theater_chase(bool dual)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.theater_chase.timebase = millis();
  }
  uint16_t off = map(beat88(SEGMENT.beat88 / 2,  _segment_runtime.modevars.theater_chase.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)255) % 3;

  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    uint8_t pal_index = map(i, (uint16_t)0, (uint16_t)(_segment_runtime.length - 1), (uint16_t)0, (uint16_t)255) + SEGMENT_RUNTIME.baseHue;
    if ((i % 3) == off)
    {
      leds[_segment_runtime.start + i] = ColorFromPalette(_currentPalette, pal_index, 255, SEGMENT.blendType);
    }
    else
    {
      if(dual)
        leds[_segment_runtime.start + i] = ColorFromPalette(_currentPalette, 128 + pal_index, 32, SEGMENT.blendType);
      else
        leds[_segment_runtime.start + i] = CRGB::Black;
    }
  }
  return STRIP_MIN_DELAY;
} 


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void)
{
  return theater_chase(false);
}

uint16_t WS2812FX::mode_theater_chase_dual_pal(void)
{
  return theater_chase(true);
}

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void)
{
  SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step = (SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step + 1) & 0xFF;
  return theater_chase(ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step, 255, _segment.blendType));
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.running_lights.timebase = millis();
  }
  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    uint8_t lum = qsub8(sin8_C(map(i, (uint16_t)0, (uint16_t)(_segment_runtime.length - 1), (uint16_t)0, (uint16_t)255)), 2);
    uint16_t offset = map(beat88(SEGMENT.beat88, _segment_runtime.modevars.running_lights.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(_segment_runtime.length * 10)); //map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, _segment_runtime.length - 1);
    offset = (offset + i) % _segment_runtime.length;

    CRGB newColor = CRGB::Black;

    newColor = ColorFromPalette(_currentPalette, map(offset, (uint16_t)0, (uint16_t)(_segment_runtime.length - 1), (uint16_t)0, (uint16_t)255) + SEGMENT_RUNTIME.baseHue, lum, SEGMENT.blendType);
    nblend(leds[_segment_runtime.start + offset], newColor, qadd8(SEGMENT.beat88 >> 8, 16));
  }
  return STRIP_MIN_DELAY;
}

/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }

  EVERY_N_MILLISECONDS(STRIP_MIN_DELAY)
  {
    fade_out(qadd8(SEGMENT.beat88 >> 8, 12));
  }
  
  uint16_t numsparks = 0;
  for(uint16_t i=0; i<_segment_runtime.length; i++)
  {
    if(leds[i])
    {
      numsparks++;
    }
  }
  uint16_t maxsparks = map((uint16_t)_segment.twinkleDensity, (uint16_t)0, (uint16_t)8, (uint16_t)0, (uint16_t)_segment_runtime.length);
  
  if(numsparks < maxsparks)
  {
    uint16_t i = random16(_segment_runtime.length);
    if(!leds[i]) 
    {
      leds[i] = ColorFromPalette(_currentPalette, random8(), random8(128,255), _segment.blendType);
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.larson_scanner.timebase = millis();
  }

  const uint16_t width = max(1, _segment_runtime.length / 15);
  fade_out(96);

  uint16_t pos = triwave16(beat88(SEGMENT.beat88 * 4, _segment_runtime.modevars.larson_scanner.timebase));

  pos = map(pos, (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16));

  drawFractionalBar(pos,
                    width,
                    _currentPalette,
                    SEGMENT_RUNTIME.baseHue + map(pos, (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16 - width * 16), (uint16_t)0, (uint16_t)255), 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.comet.timebase = millis();
  }
  const uint16_t width = max(1, _segment_runtime.length / 15);
  fade_out(96);

  uint16_t pos = map(beat88(SEGMENT.beat88 * 4, _segment_runtime.modevars.comet.timebase), (uint16_t)0, (uint16_t)65535, (uint16_t)0, (uint16_t)(_segment_runtime.length * 16));

  drawFractionalBar((_segment_runtime.start * 16 + pos),
                    width,
                    _currentPalette,
                    map((uint16_t)(_segment_runtime.start * 16 + pos), (uint16_t)(_segment_runtime.start * 16), (uint16_t)(_segment_runtime.stop * 16), (uint16_t)0, (uint16_t)255) + SEGMENT_RUNTIME.baseHue, 255, true, 1);

  return STRIP_MIN_DELAY;
}

/*
 * Fire flicker function
 * 
 * TODO: Could add parameter for the intensity (instead of 3 different modes?)
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }
  byte lum = 255 / rev_intensity;

  for (uint16_t i = _segment_runtime.start; i <= _segment_runtime.stop; i++)
  {
    leds[i] = ColorFromPalette(_currentPalette, map(i, _segment_runtime.start, _segment_runtime.stop, (uint16_t)0, (uint16_t)255) + SEGMENT_RUNTIME.baseHue, _brightness, SEGMENT.blendType);
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
  const uint16_t framedelay = map(_segment.beat88, (uint16_t)10000, (uint16_t)0, (uint16_t)0, (uint16_t)50) + map(_segment_runtime.length, (uint16_t)300, (uint16_t)0, (uint16_t)0, (uint16_t)25);

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.bubble_sort.movedown = false;
    _segment_runtime.modevars.bubble_sort.ci = _segment_runtime.modevars.bubble_sort.co = _segment_runtime.modevars.bubble_sort.cd = 0;
    for (uint16_t i = 0; i < _segment_runtime.length; i++)
    {
      if (i == 0)
      {
        _segment_runtime.modevars.bubble_sort.hues[i] = random8();
      }
      else
      {
        _segment_runtime.modevars.bubble_sort.hues[i] = get_random_wheel_index(_segment_runtime.modevars.bubble_sort.hues[i - 1], 48);
      }
    }
    map_pixels_palette(_segment_runtime.modevars.bubble_sort.hues, 32, SEGMENT.blendType);
    _segment_runtime.modevars.bubble_sort.co = 0;
    _segment_runtime.modevars.bubble_sort.ci = 0;
    return STRIP_MIN_DELAY;
  }
  if (!_segment_runtime.modevars.bubble_sort.movedown)
  {
    if (_segment_runtime.modevars.bubble_sort.co <= _segment_runtime.length)
    {
      if (_segment_runtime.modevars.bubble_sort.ci <= _segment_runtime.length)
      {
        if (_segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci] > _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.co])
        {
          uint8_t tmp = _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci];
          _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci] = _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.co];
          _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.co] = tmp;
          _segment_runtime.modevars.bubble_sort.cd = _segment_runtime.modevars.bubble_sort.ci;
          _segment_runtime.modevars.bubble_sort.movedown = true;
        }
        _segment_runtime.modevars.bubble_sort.ci++;
      }
      else
      {
        _segment_runtime.modevars.bubble_sort.co++;
        _segment_runtime.modevars.bubble_sort.ci = _segment_runtime.modevars.bubble_sort.co;
      }
    }
    else
    {
      if (_transition)
        _segment_runtime.modeinit = true;
      return STRIP_MIN_DELAY;
    }
    map_pixels_palette(_segment_runtime.modevars.bubble_sort.hues, 32, SEGMENT.blendType);
    leds[_segment_runtime.modevars.bubble_sort.ci + _segment_runtime.start] = ColorFromPalette(_currentPalette, _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci], _brightness, SEGMENT.blendType); //CRGB::Green;
    leds[_segment_runtime.modevars.bubble_sort.co + _segment_runtime.start] = ColorFromPalette(_currentPalette, _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci], _brightness, SEGMENT.blendType); // CRGB::Red;
  }
  else
  {
    map_pixels_palette(_segment_runtime.modevars.bubble_sort.hues, 32, SEGMENT.blendType);
    leds[_segment_runtime.modevars.bubble_sort.co + _segment_runtime.start] = ColorFromPalette(_currentPalette, _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.ci], _brightness, SEGMENT.blendType); // CRGB::Red;
    leds[_segment_runtime.modevars.bubble_sort.cd + _segment_runtime.start] = ColorFromPalette(_currentPalette, _segment_runtime.modevars.bubble_sort.hues[_segment_runtime.modevars.bubble_sort.cd], _brightness, SEGMENT.blendType); // +=CRGB::Green;
    if (_segment_runtime.modevars.bubble_sort.cd == _segment_runtime.modevars.bubble_sort.co)
    {
      _segment_runtime.modevars.bubble_sort.movedown = false;
    }
    _segment_runtime.modevars.bubble_sort.cd--;
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    memset(_segment_runtime.modevars.fire2012.heat, 0x0, _segment_runtime.length * sizeof(byte));
  }

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < _segment_runtime.length; i++)
  {
    _segment_runtime.modevars.fire2012.heat[i] = qsub8(_segment_runtime.modevars.fire2012.heat[i], random8(0, ((SEGMENT.cooling * 10) / _segment_runtime.length) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = _segment_runtime.length - 1; k >= 2; k--)
  {
    _segment_runtime.modevars.fire2012.heat[k] = 
      (_segment_runtime.modevars.fire2012.heat[k - 1] + 
       _segment_runtime.modevars.fire2012.heat[k - 2] + 
       _segment_runtime.modevars.fire2012.heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SEGMENT.sparking)
  {
    int y = random8(7);
    _segment_runtime.modevars.fire2012.heat[y] = qadd8(_segment_runtime.modevars.fire2012.heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < _segment_runtime.length; j++)
  {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8(_segment_runtime.modevars.fire2012.heat[j], 240);
    CRGB color = ColorFromPalette(_currentPalette, colorindex);

    leds[j + _segment_runtime.start] = color;
  }
  return STRIP_MIN_DELAY;
}

/*
 * TwinleFox Implementation
 */
uint16_t WS2812FX::mode_twinkle_fox(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
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

  for (uint16_t i = 0; i < _segment_runtime.length; i++)
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
      leds[i + _segment_runtime.start] = c;
    }
    else if (deltabright > 0)
    {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      leds[+_segment_runtime.start] = blend(bg, c, deltabright * 8);
    }
    else
    {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      leds[i + _segment_runtime.start] = bg;
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
  for (uint16_t i = 0; i < _segment_runtime.length; i++)
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
  const uint16_t num_flags = (_segment_runtime.length + 7)/8;

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    memset(_segment_runtime.modevars.soft_twinkle.directionFlags, 0x0, num_flags);
  }
  if (_segment_runtime.modevars.soft_twinkle.directionFlags == NULL)
    return STRIP_MIN_DELAY;

  uint16_t speed = (10001 - _segment.beat88 % 10000) / 250 + 1;
  EVERY_N_MILLISECONDS_I(timerObj, speed)
  {
    timerObj.setPeriod(speed);
    // Make each pixel brighter or darker, depending on
    // its 'direction' flag.
    brightenOrDarkenEachPixel(_segment.twinkleSpeed * 8 + 2, _segment.twinkleSpeed * 5 + 1, _segment_runtime.modevars.soft_twinkle.directionFlags);

    // Now consider adding a new random twinkle
    if (random8() < _segment.twinkleDensity * 32)
    {
      int pos = random16(_segment_runtime.length);
      if (!leds[pos])
      {
        leds[pos] = ColorFromPalette(_currentPalette, random8(), _segment.sparking / 2, _segment.blendType);
        setPixelDirection(pos, 1, _segment_runtime.modevars.soft_twinkle.directionFlags);
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

  #define SRMVSS _segment_runtime.modevars.shooting
  
  if (_segment_runtime.modeinit || SRMVSS.basebeat != SEGMENT.beat88)
  {
    _segment_runtime.modeinit = false;
    SRMVSS.basebeat = SEGMENT.beat88;
    for (uint8_t i = 0; i < SEGMENT.numBars; i++)
    {
      SRMVSS.delta_b[i] = (65535 / SEGMENT.numBars) * i;
      if (i > 0)
      {
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[i - 1], 32);
      }
      else
      {
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SEGMENT.numBars - 1], 32);
      }
      SRMVSS.new_cind[i] = false;
    }
  }

  fadeToBlackBy(leds, _segment_runtime.length > 8 ? _segment_runtime.length - 8 : _segment_runtime.length, (SEGMENT.beat88 >> 8) | 0x60);
  if (_segment_runtime.length > 8)
    blur1d(&leds[_segment_runtime.stop - 7], 8, 120);

  for (uint8_t i = 0; i < SEGMENT.numBars; i++)
  {
    uint16_t beat = beat88(SEGMENT.beat88) + SRMVSS.delta_b[i];

    double_t q_beat = (beat / 100) * (beat / 100);
    pos = map(static_cast<uint32_t>(q_beat + 0.5), (uint32_t)0, (uint32_t)429484, (uint32_t)(_segment_runtime.start * 16), (uint32_t)(_segment_runtime.stop * 16));

    //we use the fractional bar and 16 brghtness values per pixel
    drawFractionalBar(pos, 2, _currentPalette, SRMVSS.cind[i], 255, true, 1);

    if (pos / 16 > (_segment_runtime.stop - 4)) // 6
    {
      uint8_t tmp = 0;
      CRGB led = ColorFromPalette(_currentPalette, SRMVSS.cind[i], _brightness, _segment.blendType); //leds[pos/16];
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
          SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SEGMENT.numBars - 1], 32);
      }
      SRMVSS.new_cind[i] = false;
    }
  }
  #undef SRMVSS
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_beatsin_glow(void)
{
  #define SRMVGB _segment_runtime.modevars.beatsin
  const uint16_t lim = (SEGMENT.beat88 * 10) / 50;

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _transition = true;
    _blend = 0;
    
    for (uint8_t i = 0; i < SEGMENT.numBars; i++)
    {
      SRMVGB.beats[i] = SEGMENT.beat88 + lim / 2 - random16(lim);
      SRMVGB.theta[i] = (65535 / SEGMENT.numBars) * i + (65535 / (4 * SEGMENT.numBars)) - random16(65535 / (2 * SEGMENT.numBars));
      uint8_t temp = random8(255 / (2 * SEGMENT.numBars));
      if (temp & 0x01)
      {
        SRMVGB.cinds[i] = (255 / SEGMENT.numBars) * i - temp;
      }
      else
      {
        SRMVGB.cinds[i] = (255 / SEGMENT.numBars) * i + temp;
      }
      SRMVGB.times[i] = millis() + random8();
      SRMVGB.prev[i] = 0;
      SRMVGB.newval[i] = false;
    }
  }

  fadeToBlackBy(leds, _segment_runtime.length, (SEGMENT.beat88 >> 8) | 32);

  uint16_t pos = 0;

  for (uint8_t i = 0; i < SEGMENT.numBars; i++)
  {
    uint16_t beatval = beat88(SRMVGB.beats[i], SRMVGB.times[i] + SRMVGB.theta[i]);
    int16_t si = sin16(beatval); // + theta[i]);

    if (si > -2 && si < 2 && SRMVGB.prev[i] < si) //si >= 32640 || si <= -32640)
    {
      const uint8_t rand_delta = 64;
      SRMVGB.beats[i] = SRMVGB.beats[i] + (SEGMENT.beat88 * 10) / 50 - random16((SEGMENT.beat88 * 10) / 25); //+= (random8(128)%2)?1:-1; // = beats[i] + (SEGMENT.beat88*10)/200 - random16((SEGMENT.beat88*10)/100); //
      if (SRMVGB.beats[i] < (SEGMENT.beat88 / 2))
        SRMVGB.beats[i] = SEGMENT.beat88 / 2;
      if (SRMVGB.beats[i] > (SEGMENT.beat88 + SEGMENT.beat88 / 2))
        SRMVGB.beats[i] = SEGMENT.beat88 + SEGMENT.beat88 / 2;
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
    pos = map((65535 >> 1) + si, 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16);
    drawFractionalBar(pos, 2, _currentPalette, SRMVGB.cinds[i] + i * (255 / SEGMENT.numBars), _brightness, true, 1);
  }
  #undef SRMVGB
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pixel_stack(void)
{
  #define SRMVPS _segment_runtime.modevars.pixel_stack
  // the beat88 translated to a effect speed (in relation to the number of segments)
  const uint16_t sp = map((uint16_t)(_segment.beat88>(20000/_segment.segments)?(20000/_segment.segments):_segment.beat88), (uint16_t)0, (uint16_t)(20000/_segment.segments), (uint16_t)0, (uint16_t)65535);
  // The number of leds for the effect (half of the strip)
  const uint16_t nLeds = _segment_runtime.length / 2;
  // the base hue (the hue can move during the effect)
  const uint8_t sI = _segment_runtime.baseHue;

  // init the effect
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

    SRMVPS.up = true;
    SRMVPS.leds_moved = 0;
    SRMVPS.ppos16 = 0;
    
  }
  // We fade everything to black (a speed dependent amount) - leds will be rewritten later
  fadeToBlackBy(leds, _segment_runtime.length, max(2, sp>>8));
  // write each active led with the color from the palette (without the one currently moving)
  for(uint16_t i = 0; i < nLeds; i++)
  {
    if(i<nLeds-SRMVPS.leds_moved)
    {
      // lower half (half the strip minus the number already moved)
      leds[i] = ColorFromPalette(_currentPalette, sI + map(i, (uint16_t)0, (uint16_t)(nLeds-1), (uint16_t)0, (uint16_t)255));
    }
    if(i < SRMVPS.leds_moved)
    {
      // upper half - the number of leds moved from the end
      leds[_segment_runtime.length-1-i] = ColorFromPalette(_currentPalette, sI + map((uint16_t)(nLeds - i), (uint16_t)0, (uint16_t)(nLeds-1), (uint16_t)0, (uint16_t)255));
    }
  }
  // a value within the 0..65535 range (sawtooth) depending on the speed sp
  uint16_t p16 = beat88(sp);
  if(SRMVPS.up)
  {
    // mapping the p16 value to a (16 bit position) between the active leds
    uint16_t pos16 = map(p16, (uint16_t)0, (uint16_t)65535, (uint16_t)(16*(nLeds - SRMVPS.leds_moved)), (uint16_t)(16*(_segment_runtime.length - 1 - SRMVPS.leds_moved)-16));
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
    uint16_t pos16 = map(p16, (uint16_t)0, (uint16_t)65535, (uint16_t)(16*(_segment_runtime.length - 1 - SRMVPS.leds_moved)-16), (uint16_t)(16*(nLeds - SRMVPS.leds_moved)));
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }
  const uint16_t width = _segment_runtime.length/2;
  const uint16_t sp = map(_segment.beat88>(20000/_segment.segments)?(20000/_segment.segments):_segment.beat88, 0, (20000/_segment.segments), 0, 65535);


  //fadeToBlackBy(leds, _segment_runtime.length, map(sp, 0, 65535, 0, 255)); 
  fadeToBlackBy(leds, _segment_runtime.length, map(sp, (uint16_t)0, (uint16_t)65535, (uint16_t)64, (uint16_t)255)); 


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
  drawFractionalBar(pos16, width, _currentPalette, _segment_runtime.baseHue, 255, false, inc);

  //fill_palette(&leds[pos16], width, _segment_runtime.baseHue, constrain(255/width, 1, 255), _currentPalette, 255, _segment.blendType);
  //blur1d(leds, LED_COUNT, map(sp, 0, 65535, 172, 32));
  

  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_popcorn(void)
{
  #define POPCMV _segment_runtime.modevars.pops
  #define LEDS_PER_METER 60
  // Length in 16 LEDs (close to 60 Leds per Meter in millimeters)
  // e.g. 60 leds / 1 meter ~~ (60*16 = 960 mm)
  // const double segLength = ((double)_segment_runtime.length * 16.0); -> directly used in v0_max calc

  // Gravity 9.81 mm/ms² to 9810 mm/ms² (eq. earth 9.81 m/s²)
  const double gravity = -(map((double)_segment.beat88, 0.0, 10000.0, 9.810, 9810.0)/(1000.0*1000.0)); // mm/ms²
  
  // speed required to reach the end of the strip (segment). Everything lower will be before
  const double v0_max  = sqrt(-2.0*gravity*((double)_segment_runtime.length * 16.0));
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    for(uint16_t i=0; i<_segment.numBars; i++)
    {
      POPCMV.pops[i].v0 = v0_max / (double)(i+2.0); 
      POPCMV.pops[i].color_index = get_random_wheel_index((255/_segment.numBars)*i, 32);
      POPCMV.pops[i].timebase = millis(); 
      POPCMV.pops[i].damp = 1; 
      POPCMV.pops[i].prev_pos = 0;
    }
  }

  fill_solid(leds, _segment_runtime.length, CRGB::Black);

  double pos = 0;
  for(uint16_t i=0; i<_segment.numBars; i++)
  {
    double dT = (double)(millis()-POPCMV.pops[i].timebase);
    pos = (gravity/2 * dT + POPCMV.pops[i].v0)*dT;
    if(pos < 0)
    {
      if(_segment.damping < 100)
      {
        POPCMV.pops[i].v0 = POPCMV.pops[i].v0*((double)POPCMV.pops[i].damp/100.0)-0.02;  
      }
      POPCMV.pops[i].timebase = millis();
      if(POPCMV.pops[i].v0 < 0.01 && random8()<1)
      {
        POPCMV.pops[i].v0 = ((double)random8(80,100)/100.0) * v0_max;
        POPCMV.pops[i].color_index = get_random_wheel_index(POPCMV.pops[i].color_index, 32);
        POPCMV.pops[i].damp = _segment.damping<100?(uint8_t)((random8(90,100) * _segment.damping)/100):100;
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
  #define FW2MV _segment_runtime.modevars.pops
  
  const uint16_t maxBlendWidth = _segment_runtime.length/2>40?40:_segment_runtime.length/2;
  const double segmentLength = ((double)(_segment_runtime.length-maxBlendWidth/2) / (double)60) * (double)1000.0; // physical length in mm
  const double gravity = (double)_segment.beat88 / (double)-1019367.99184506;                                          // -0.00981; // gravity in mm per ms²
  const double v0_max = sqrt(-2 * gravity * segmentLength);


  uint32_t now = millis();

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

    for (uint16_t i = 0; i < _segment.numBars; i++)
    {
      FW2MV.pops[i].timebase = now;
      FW2MV.pops[i].color_index = 256 / _segment.numBars * i; //get_random_wheel_index(pops[i].color_index);
      FW2MV.pops[i].pos = 0;
      FW2MV.pops[i].prev_pos = 0;
      FW2MV.pops[i].v0 = 0;
      FW2MV.pops[i].v = 0;
      //FW2MV.pops[i].ignite = false;
      //FW2MV.pops[i].P_ignite = random(100, 500);
      FW2MV.pops[i].v_explode = FW2MV.pops[i].v0 * ((double)random8(0, 10) / 100.0);
    }
  }
  fade_out(map(_segment.beat88, (uint16_t)0, (uint16_t)6000, (uint16_t)24, (uint16_t)255));
  for (uint16_t i = 0; i < _segment.numBars; i++) {
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
          FW2MV.pops[i].explodeTime = map(_segment.beat88, (uint16_t)1000, (uint16_t)6000, (uint16_t)80, (uint16_t)180);//60; //random(t_max * 3/5, t_max * 4/5);
          FW2MV.pops[i].v_explode = FW2MV.pops[i].v0 * ((double)random8(1, 50) / 100.0);
        }
      } else {
        float damping = 0.1f / 100.0f;
        if (_segment.damping) {
          if (_segment.damping <= 100) {
            damping = ((float)_segment.damping / 100.0f);
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
      
      uint16_t pos = map((double)FW2MV.pops[i].pos, 0.0, (double)segmentLength, _segment_runtime.start * 16.0, _segment_runtime.stop * 16.0);
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
        if((_segment_runtime.stop-pos/16)<blendWidth/2+3)
        {
          blendWidth = (_segment_runtime.stop-pos/16) * 2;
        }
        if(pos/16 < blendWidth/2) blendWidth = pos/16 * 2;
        uint8_t br = map(FW2MV.pops[i].brightness, (uint8_t)0, (uint8_t)48, (uint8_t)0, (uint8_t)255);
        if ((FW2MV.pops[i].explodeTime > 10))//> 4 * STRIP_MIN_DELAY))
        {
          FW2MV.pops[i].explodeTime--;//; -=  (FW2MV.pops[i].explodeTime - 4 * STRIP_MIN_DELAY);//(12*STRIP_MIN_DELAY)/10;
          
          
          //leds[pos/16] += ColorFromPalette(_currentPalette, FW2MV.pops[i].color_index, 255, _segment.blendType);
          //leds[pos/16] += 0x202020;
          drawFractionalBar(pos, 3, _currentPalette, FW2MV.pops[i].color_index, 255, true,0);
          CRGB col = leds[pos/16] + CRGB(0x202020);
          drawFractionalBar(pos, 3, CRGBPalette16(col), 0, 255, true, 0);
          blur1d(&leds[pos/16+1], blendWidth, 172);  
          //leds[pos/16] += ColorFromPalette(_currentPalette, FW2MV.pops[i].color_index, 120, _segment.blendType);
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment.autoplay = AUTO_MODE_OFF;

  }
  return STRIP_MIN_DELAY;
}

void WS2812FX::draw_sunrise_step(uint16_t sunriseStep)
{
  #define SRMVSR _segment_runtime.modevars.sunrise_step
  
  
  uint16_t step = sunriseStep;

  // Kind of dithering... lets see
  if(SRMVSR.toggle)
  {
    step +=1;
  }
  SRMVSR.toggle = !SRMVSR.toggle;
  
  fill_solid(leds, _segment_runtime.length, calcSunriseColorValue(step));

  EVERY_N_MILLISECONDS(100)
  {
    for(uint16_t i=0; i<_segment_runtime.length; i++)
    {
      SRMVSR.nc[i] = random8(0, 185);
    }
  }
  for (uint16_t i = 0; i < _segment_runtime.length; i++)
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
  float time = (float)((_segment.sunrisetime * 60.0) / DEFAULT_SUNRISE_STEPS);
  if(getMode() == FX_MODE_SUNRISE)
  {
    return (uint16_t)(time * (DEFAULT_SUNRISE_STEPS - _segment_runtime.modevars.sunrise_step.sunRiseStep));
  }
  else if (getMode() == FX_MODE_SUNSET) 
  {
    return (uint16_t)(time * _segment_runtime.modevars.sunrise_step.sunRiseStep);
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
  #define SRMVSR _segment_runtime.modevars.sunrise_step
  const uint16_t sunriseSteps = DEFAULT_SUNRISE_STEPS;
  uint16_t stepInterval = (uint16_t)(_segment.sunrisetime * ((60 * 1000) / sunriseSteps));

  // We do not need background color during sunrise / sunset.... Lets try to clear these:
  _c_bck_b = 0;

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment.autoplay = AUTO_MODE_OFF;
    SRMVSR.next = millis();
    if (isSunrise)
    {
      _segment.targetBrightness = 255;
      SRMVSR.sunRiseStep = 0;
    }
    else
    {
      SRMVSR.sunRiseStep = sunriseSteps;
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
  const uint16_t onTime  = 50; //(BEAT88_MAX + 10) - _segment.beat88;
  const uint16_t offTime = 100; //2*((BEAT88_MAX + 10) - _segment.beat88);
  const uint16_t runTime = 1500;
  const uint16_t pauseTime = 2000;
  _segment_runtime.modevars.ring_ring.now = millis();
  if(_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.ring_ring.isOn = true;
    _segment_runtime.modevars.ring_ring.nextmillis = 0; 
    _segment_runtime.modevars.ring_ring.pausemillis = _segment_runtime.modevars.ring_ring.now + 10;
    _segment_runtime.modevars.ring_ring.isPause = 0;
  }
  if(_segment_runtime.modevars.ring_ring.isPause)
  {
    fadeToBlackBy(leds, _segment_runtime.length, 32);
    //fill_solid(leds, _segment_runtime.length, CRGB::Black);
    if(_segment_runtime.modevars.ring_ring.now > (_segment_runtime.modevars.ring_ring.pausemillis + pauseTime))
    {
      _segment_runtime.modevars.ring_ring.pausemillis = _segment_runtime.modevars.ring_ring.now;
      _segment_runtime.modevars.ring_ring.isPause = false;
    }
  }
  else
  {
    if(_segment_runtime.modevars.ring_ring.isOn)
    {
      fill_palette(leds, _segment_runtime.length, SEGMENT_RUNTIME.baseHue, (_segment_runtime.length > 255 ? 1 : (256 / _segment_runtime.length)), _currentPalette, 255, SEGMENT.blendType);
      if(_segment_runtime.modevars.ring_ring.now > (_segment_runtime.modevars.ring_ring.nextmillis + onTime))
      {
        _segment_runtime.modevars.ring_ring.nextmillis = _segment_runtime.modevars.ring_ring.now;
        _segment_runtime.modevars.ring_ring.isOn = false;
      }
    }
    else
    {
      fill_solid(leds, _segment_runtime.length, CRGB::Black);
      if(_segment_runtime.modevars.ring_ring.now > (_segment_runtime.modevars.ring_ring.nextmillis + offTime))
      {
        _segment_runtime.modevars.ring_ring.nextmillis = _segment_runtime.modevars.ring_ring.now;
        _segment_runtime.modevars.ring_ring.isOn = true;
      }
    }
    if(_segment_runtime.modevars.ring_ring.now > (_segment_runtime.modevars.ring_ring.pausemillis + runTime))
    {
      _segment_runtime.modevars.ring_ring.pausemillis = _segment_runtime.modevars.ring_ring.now;
      _segment_runtime.modevars.ring_ring.isPause = true;
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
  #define M_HEARTBEAT_RT _segment_runtime.modevars.heartBeat
  
  if(_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    M_HEARTBEAT_RT.lastBeat = 0;
    M_HEARTBEAT_RT.secondBeatActive = false; 
    M_HEARTBEAT_RT.msPerBeat = (60000 / (_segment.beat88>20?_segment.beat88 / 20 : 1));
    M_HEARTBEAT_RT.secondBeat = (M_HEARTBEAT_RT.msPerBeat / 3);
    M_HEARTBEAT_RT.size = map(_segment_runtime.length, (uint16_t)25, (uint16_t)300, (uint16_t)1, (uint16_t)6);
    M_HEARTBEAT_RT.centerOffset = (_segment_runtime.length / 2);
    M_HEARTBEAT_RT.pCount = M_HEARTBEAT_RT.centerOffset - M_HEARTBEAT_RT.size;
  }

  for(uint16_t i = 0; i < M_HEARTBEAT_RT.pCount; i++)
  {
    leds[i] = leds[i + M_HEARTBEAT_RT.size];
    leds[i + M_HEARTBEAT_RT.centerOffset + M_HEARTBEAT_RT.size] = leds[i+M_HEARTBEAT_RT.centerOffset];
  }

  fadeToBlackBy(leds, _segment_runtime.length, (SEGMENT.beat88 >> 8) | 32);

  M_HEARTBEAT_RT.beatTimer = millis() - M_HEARTBEAT_RT.lastBeat;
  if((M_HEARTBEAT_RT.beatTimer > M_HEARTBEAT_RT.secondBeat) && !M_HEARTBEAT_RT.secondBeatActive) { // time for the second beat?
    // create the second beat
    fill_solid(&leds[_segment_runtime.start + (_segment_runtime.length / 2) - M_HEARTBEAT_RT.size], (M_HEARTBEAT_RT.size)*2, ColorFromPalette(_currentPalette, _segment_runtime.baseHue));
    M_HEARTBEAT_RT.secondBeatActive = true;
  }
  if(M_HEARTBEAT_RT.beatTimer > M_HEARTBEAT_RT.msPerBeat) { // time to reset the beat timer?
    // create the first beat
    fill_solid(&leds[_segment_runtime.start + (_segment_runtime.length / 2) - M_HEARTBEAT_RT.size], (M_HEARTBEAT_RT.size)*2, ColorFromPalette(_currentPalette, _segment_runtime.baseHue));
    M_HEARTBEAT_RT.secondBeatActive = false;
    M_HEARTBEAT_RT.lastBeat = millis();
  }
  return STRIP_MIN_DELAY;
  #undef M_HEARTBEAT_RT
}

uint16_t WS2812FX::mode_rain(void)
{
  #define M_RAIN_RT _segment_runtime.modevars.rain
  if(_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    memset(&(M_RAIN_RT.timebase), _segment.numBars, sizeof(M_RAIN_RT.timebase[0]) * _segment.numBars);
    memset(&(M_RAIN_RT.actives),  _segment.numBars, sizeof(M_RAIN_RT.actives[0])  * _segment.numBars);
    memset(&(M_RAIN_RT.cind),     _segment.numBars, sizeof(M_RAIN_RT.cind[0])     * _segment.numBars);
    M_RAIN_RT.timebase[0] = millis();
    M_RAIN_RT.actives[0] = true;
    M_RAIN_RT.cind[0] = random8();
  }
  EVERY_N_MILLIS(20)
  {
    fadeToBlackBy(leds, _segment_runtime.length, map(_segment.beat88, (uint16_t)100, (uint16_t)7968, (uint16_t)3, (uint16_t) 255));
  }
  uint16_t pos16 = 0; 
  for(uint16_t i = 0; i<_segment.numBars; i++)
  {
    if(M_RAIN_RT.actives[i])
    {
      pos16 = map((uint16_t)(beat88(_segment.beat88*3, M_RAIN_RT.timebase[i])), (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.stop*16), (uint16_t)(_segment_runtime.start*16));
      drawFractionalBar(pos16, 4, _currentPalette, _segment_runtime.baseHue + M_RAIN_RT.cind[i], 255, true, 0);
      if(!(pos16/16)) M_RAIN_RT.actives[i] = false;
    }
  }
  EVERY_N_MILLIS(100)
  { 
    const uint16_t minDistance = max(_segment_runtime.length/12, 1);
    bool distBlack = true;
    for(uint16_t i = 0; i<minDistance; i++)
    {
      if(leds[_segment_runtime.stop-i] != CRGB(0x000000))
      {
        distBlack = false;
      }
    }
    for(uint16_t i = 0; i<_segment.numBars; i++)
    {
      if(!M_RAIN_RT.actives[i] && !random8(4) && distBlack) //leds[_segment_runtime.stop-10] == CRGB(0x000000)) 
      {
        M_RAIN_RT.actives[i]  = true;
        M_RAIN_RT.timebase[i] = millis();
        M_RAIN_RT.cind[i] = get_random_wheel_index(M_RAIN_RT.cind[i]);

        pos16 = map((uint16_t)(beat88(_segment.beat88*3, M_RAIN_RT.timebase[i])), (uint16_t)0, (uint16_t)65535, (uint16_t)(_segment_runtime.stop*16), (uint16_t)(_segment_runtime.start*16));
        drawFractionalBar(pos16, 4, _currentPalette, _segment_runtime.baseHue + M_RAIN_RT.cind[i], 255, true, 0);
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
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

  }
  uint16_t d1, d2, d3;

  d1 = beatsin16(_segment.beat88/1000+1);
  d2 = beatsin16(_segment.beat88/500+1);
  d3 = beatsin8 (_segment.beat88/1024+1);
  

  uint16_t startLed = beatsin16(_segment.beat88/1000+1, _segment_runtime.start, _segment_runtime.stop-1, 0, d1);
  uint16_t numLeds = beatsin16(_segment.beat88/500+1, 10, _segment_runtime.length - startLed, 0, d2);
  numLeds = min(numLeds, (uint16_t)(_segment_runtime.length-startLed));
  uint8_t incI = 1; // (numLeds > 255 ? 1 : (256 / numLeds));
  uint8_t sInd = d3 + _segment_runtime.baseHue; //beatsin8(_segment.beat88/128+1);
  fadeToBlackBy(&leds[_segment_runtime.start], _segment_runtime.length, 64);
  fill_palette(&leds[startLed], numLeds, sInd, incI, _currentPalette, 255, SEGMENT.blendType);

  return STRIP_MIN_DELAY;
}