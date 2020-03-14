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
  DEBUGPRNT("Initialising existing values...");
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
  setReverse(_segment.reverse );
  setInverse(_segment.inverse );
  setMirror(_segment.mirror );
  setAutoplay(_segment.autoplay );
  setAutopal(_segment.autoPal );
  setBeat88(_segment.beat88 );
  setHuetime(_segment.hueTime );
  setMilliamps(_segment.milliamps );
  setAutoplayDuration(_segment.autoplayDuration );
  setAutopalDuration(_segment.autoPalDuration );
  setSegments(_segment.segments );
  setCooling(_segment.cooling );
  setSparking(_segment.sparking );
  setTwinkleSpeed(_segment.twinkleSpeed );
  setTwinkleDensity(_segment.twinkleDensity );
  setNumBars(_segment.numBars );
  setMode(_segment.mode );
  setMaxFPS(_segment.fps );
  setDeltaHue(_segment.deltaHue );
  setBlur(_segment.blur );
  setDamping(_segment.damping );
  setDithering(_segment.dithering );
  setSunriseTime          (_segment.sunrisetime);
  setTargetBrightness     (_segment.targetBrightness);
  setBlendType            (_segment.blendType);
  setColorTemp            (_segment.colorTemp);
  setTargetPaletteNumber(_segment.targetPaletteNum );
  setCurrentPaletteNumber(_segment.currentPaletteNum );
  setAddGlitter(_segment.addGlitter);
  setWhiteGlitter(_segment.whiteGlitter);
  setOnBlackOnly(_segment.onBlackOnly);
  setChanceOfGlitter(_segment.chanceOfGlitter);

  #ifdef HAS_KNOB_CONTROL
  setWiFiEnabled(_segment.wifiEnabled);
  #endif

  old_segs = 0;

  // should start with tranistion after init
  setTransition();
  setIsRunning(isRunning );
  setPower(power);

}

void WS2812FX::resetDefaults(void)
{
  DEBUGPRNT("Resetting to Default values");
  RESET_RUNTIME;

  _segment_runtime.start = 0;
  _segment_runtime.stop = LED_COUNT - 1;
  _segment_runtime.length = LED_COUNT;

  fill_solid(_bleds, LED_COUNT, CRGB::Black);
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.clear(true); // During init, all pixels should be black.
  FastLED.show();      // We show once to write the Led data.

  _brightness = 255;

  setIsRunning(DEFAULT_RUNNING );
  setPower(DEFAULT_POWER);
  setReverse(DEFAULT_REVERSE );
  setInverse(DEFAULT_INVERTED );
  setMirror(DEFAULT_MIRRORED );
  setAutoplay(DEFAULT_AUTOMODE );
  setAutopal(DEFAULT_AUTOCOLOR );
  setBeat88(DEFAULT_SPEED );
  setHuetime(DEFAULT_HUE_INT );
  setMilliamps(DEFAULT_CURRENT );
  setAutoplayDuration(DEFAULT_T_AUTOMODE);
  setAutopalDuration(DEFAULT_T_AUTOCOLOR);
  setSegments(DEFAULT_NUM_SEGS);
  setCooling(DEFAULT_COOLING );
  setSparking(DEFAULT_SPAKRS );
  setTwinkleSpeed(DEFAULT_TWINKLE_S);
  setTwinkleDensity(DEFAULT_TWINKLE_NUM);
  setNumBars(DEFAULT_LED_BARS);
  setMode(DEFAULT_MODE);
  setMaxFPS(STRIP_MAX_FPS);
  setDeltaHue(DEFAULT_HUE_OFFSET );
  setBlur(DEFAULT_BLENDING );
  setDamping(DEFAULT_DAMPING);
  setDithering(DEFAULT_DITHER);
  setTargetPaletteNumber(DEFAULT_PALETTE);
  setCurrentPaletteNumber(DEFAULT_PALETTE);
  setSunriseTime          (DEFAULT_SUNRISETIME);
  setTargetBrightness     (DEFAULT_BRIGHTNESS);
  setBlendType            (DEFAULT_BLEND);
  setColorTemp            (DEFAULT_COLOR_TEMP);
  setAddGlitter(false);
  setWhiteGlitter(true);
  setOnBlackOnly(false);
  setChanceOfGlitter(DEFAULT_GLITTER_CHANCE_NORMAL);
  #ifdef HAS_KNOB_CONTROL
  setWiFiEnabled(DEFAULT_WIFI_ENABLED);
  #endif
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

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


/*
 * the overall service task. To be called as often as possible / useful
 * (at least at the desired frame rate)
 * --> see STRIP_MAX_FPS
 */
void WS2812FX::service()
{
  unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
  static uint32_t last_show = 0;
  if ((_segment.segments != old_segs) || _segment_runtime.modeinit)
  { 
    _segment_runtime.start = 0;
    _segment_runtime.length = (LED_COUNT / _segment.segments);
    _segment_runtime.stop = _segment_runtime.start + _segment_runtime.length - 1;
    if(_segment.numBars > ((LED_COUNT / _segment.segments) / MAX_NUM_BARS_FACTOR))
    {
      _segment.numBars = ((LED_COUNT / _segment.segments) / MAX_NUM_BARS_FACTOR);
    }
    // 12.04.2019
    // There are artefacts remeianing if the distribution is not equal.
    // as we blend towards the new effect, we will remove the artefacts by clearing the leds array...
    fill_solid(leds, LED_COUNT, CRGB::Black);
    setTransition();
    old_segs = _segment.segments;
  }
  if (_segment.power)
  {
    if (_segment.isRunning || _triggered)
    {

      if (now > SEGMENT_RUNTIME.next_time || _triggered)
      {

        uint16_t delay = (this->*_mode[SEGMENT.mode])();


        if(_segment.addGlitter)
        {
          addSparks(_segment.chanceOfGlitter, _segment.onBlackOnly, _segment.whiteGlitter);
        }

        SEGMENT_RUNTIME.next_time = now + (uint32_t)delay; 
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
    EVERY_N_MILLISECONDS(20)
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
  EVERY_N_MILLISECONDS(20)
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
  // this could be simplified within the following nested loop which does now all at once and svaes 2 loops + 
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

  // Fade to avoid artefacts...
  EVERY_N_MILLISECONDS(STRIP_MIN_DELAY) //(10)
  {
    if(_segment.isRunning) fadeToBlackBy(_bleds, LED_COUNT, 1);
  }

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
        _currentPaletteName = _targetPaletteName;
      }
    }
  }
  else
  {
    EVERY_N_MILLISECONDS(24)
    { // Blend towards the target palette

      nblendPaletteTowardPalette(_currentPalette, _targetPalette, 8);
    
      if (_currentPalette == _targetPalette)
      {
        _currentPaletteName = _targetPaletteName;
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
  static uint8_t hue[16];
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
  /*
  return CRGBPalette16(
      CHSV(hue[0],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[1],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[2],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[3],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[4],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[5],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[6],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[7],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[8],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[9],  random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[10], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[11], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[12], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[13], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)),
      CHSV(hue[14], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)), CHSV(hue[15], random8(RND_PAL_MIN_SAT, 255), random8(RND_PAL_MIN_BRIGHT, 255)));
  */
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
  case WarmFluorescent:
    return 9;
  case StandardFluorescent:
    return 10;
  case CoolWhiteFluorescent:
    return 11;
  case FullSpectrumFluorescent:
    return 12;
  case GrowLightFluorescent:
    return 13;
  case BlackLightFluorescent:
    return 14;
  case MercuryVapor:
    return 15;
  case SodiumVapor:
    return 16;
  case MetalHalide:
    return 17;
  case HighPressureSodium:
    return 18;

  default:
    return 19;
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
      F("WarmFluorescent"),
      F("StandardFluorescent"),
      F("CoolWhiteFluorescent"),
      F("FullSpectrumFluorescent"),
      F("GrowLightFluorescent"),
      F("BlackLightFluorescent"),
      F("MercuryVapor"),
      F("SodiumVapor"),
      F("MetalHalide"),
      F("HighPressureSodium"),
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
void WS2812FX::drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, uint8_t cindex, uint8_t max_bright = 255, bool mixColors = true)
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
  uint8_t firstpixelbrightness = 255 - (frac * 16); //map8(15 - (frac), 0, max_bright);

  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness = 255 - firstpixelbrightness; //map8(15 - firstpixelbrightness, 0, max_bright);

  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
  uint8_t bright;
  bool mix = true;
  for (int n = 0; n <= width; n++)
  {
    if (n == 0)
    {
      // first pixel in the bar
      bright = firstpixelbrightness;
    }
    else if (n == width)
    {
      // last pixel in the bar
      bright = lastpixelbrightness;
    }
    else
    {
      // middle pixels
      bright = max_bright;
      mix = false;
    }

    CRGB newColor;
    if (i <= _segment_runtime.stop && i >= _segment_runtime.start)
    {
      if (mixColors || mix)
      {
        newColor = leds[i] | ColorFromPalette(pal, cindex, bright, SEGMENT.blendType);
        // we blend based on the "baseBeat"
        nblend(leds[i], newColor, qadd8(SEGMENT.beat88 >> 8, 24));
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
 * Returns a new, random wheel index with a minimum distance of dist (default = 42) from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos, uint8_t dist = 42)
{
  dist = dist < 85 ? dist : 85; // dist shouldn't be too high (not higher than 85 actually)
  return (pos + random8(dist, 255 - (2 * dist)));
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
void WS2812FX::addSparks(const uint8_t prob = 10, bool onBlackOnly = true, bool white = false)
{
  
  const uint8_t probability = constrain(prob, DEFAULT_GLITTER_CHANCE_MIN, DEFAULT_GLITTER_CHANCE_MAX);

  static uint8_t sparks[LED_COUNT];

  uint16_t active_sparks = 0;
  for(uint16_t i=_segment_runtime.start; i<_segment_runtime.stop; i++)
  {
    if(sparks[i])
    {
      if (leds[i] && onBlackOnly)
      {
        sparks[i] = sparks[i] >> 2;
      }
      active_sparks++;
      CRGB newCol = CRGB::White;
      if (!white)
      {
        newCol = ColorFromPalette(_currentPalette, map(i,_segment_runtime.start, _segment_runtime.stop, 0, 255) + 128, 255, SEGMENT.blendType);
      }
      uint8_t blend = map(sparks[i], 0, _segment.fps, 0, 255);
      nblend(leds[i], newCol, blend);
      sparks[i] = sparks[i] >> 1;
    }
  }
  
  if (random8(DEFAULT_GLITTER_CHANCE_MAX) > probability)
    return;

  if(active_sparks > ((_segment_runtime.length * prob)/100) + 1)
    return;

  uint16_t pos = random16(_segment_runtime.start, _segment_runtime.stop); // Pick an LED at random.
  
  if (leds[pos] && onBlackOnly)
    return;
  
  sparks[pos] = random8(_segment.fps);

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
CRGB WS2812FX::computeOneTwinkle(uint32_t ms, uint8_t salt)
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

  uint16_t ticks = ms >> (8 - SEGMENT.twinkleSpeed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if (((slowcycle8 & 0x0E) / 2) < SEGMENT.twinkleDensity)
  {
    bright = attackDecayWave8(fastcycle8);
  }

#define COOL_LIKE_INCANDESCENT 0

  uint8_t hue = slowcycle8 - salt;
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
uint16_t WS2812FX::pride(bool glitter = false)
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

    // what is this about?
    // lets simplify and correct...
    /*
    uint16_t pixelnumber = i;
    pixelnumber = (_segment_runtime.stop) - pixelnumber;
    */
    uint16_t pixelnumber = (_segment_runtime.stop) - i;

    nblend(leds[pixelnumber], newcolor, 64);
  }
  // TODO: Glitter is obsolete with the global setting
  if (!glitter)
    return STRIP_MIN_DELAY;

  addSparks(10, false, true);

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
  _currentPaletteName = Name;
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
  _currentPaletteName = _pal_name[n % NUM_PALETTES];
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
  _targetPaletteName = Name;
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
    _targetPaletteName = _pal_name[n % NUM_PALETTES];
    _segment.targetPaletteNum = n % NUM_PALETTES;
    return;
  }
  _targetPalette = *(_palettes[n % NUM_PALETTES]);
  _targetPaletteName = _pal_name[n % NUM_PALETTES];
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
      if(leds[i] || _bleds[i])
      {
        leds_on++;
      }
    }
    return (leds_on * _segment.segments);
  }
  return 0;
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
    return F("");
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
 */
uint16_t WS2812FX::mode_ease(void)
{
  return this->mode_ease_func(false);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles will be additionally applied.
 */
// TODO: Could be removed due to global twinkle parameter.... 
// Does not hurt either
uint16_t WS2812FX::mode_twinkle_ease(void)
{
  return this->mode_ease_func(true);
}

/*
 * Two moving "comets" moving in and out with Antialiasing
 * Random Sparkles can additionally applied.
 */
uint16_t WS2812FX::mode_ease_func(bool sparks = true)
{
  // number of pixels for "antialised" (fractional) bar
  const uint8_t width = 1;
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
  lerpVal = beatsin88(SEGMENT_RUNTIME.modevars.ease.beat, _segment_runtime.start * 16, _segment_runtime.stop * 16 - (width * 16), SEGMENT_RUNTIME.timebase);

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
      SEGMENT_RUNTIME.timebase = millis();
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
  drawFractionalBar(lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal / 16 - _segment_runtime.start) + colorMove), _brightness);
  drawFractionalBar((_segment_runtime.stop * 16) - lerpVal, width, _currentPalette, (uint8_t)((uint8_t)(lerpVal / 16 - _segment_runtime.start) + colorMove), _brightness);

  if (sparks)
    addSparks(10, true, false);

  return STRIP_MIN_DELAY;
}

// moves a fractional bar along the stip based on noise
uint16_t WS2812FX::mode_inoise8_mover(void)
{
  return this->mode_inoise8_mover_func(false);
}

// moves a fractional bar along the stip based on noise
// random twinkles are added
uint16_t WS2812FX::mode_inoise8_mover_twinkle(void)
{
  return this->mode_inoise8_mover_func(true);
}

uint16_t WS2812FX::mode_inoise8_mover_func(bool sparks)
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
  uint16_t pixlen = map(locn, 0, 255, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

  uint8_t colormove = SEGMENT_RUNTIME.baseHue; // quadwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)) + SEGMENT_RUNTIME.baseHue;

  fade_out(48);

  drawFractionalBar(pixlen, width, _currentPalette, (uint8_t)((uint8_t)(pixlen / 64) + colormove)); //, beatsin88(max(SEGMENT.beat88/2,1),200 % _brightness, _brightness, SEGMENT_RUNTIME.timebase));

  SEGMENT_RUNTIME.modevars.inoise.dist += beatsin88(SEGMENT.beat88, 1, 6, SEGMENT_RUNTIME.timebase);

  if (sparks)
    addSparks(10, true, false);

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
  }

  uint8_t thisPhase = beatsin88(SEGMENT.beat88, 0, 255, SEGMENT_RUNTIME.timebase);             // Setting phase change for a couple of waves.
  uint8_t thatPhase = beatsin88((SEGMENT.beat88 * 11) / 10, 0, 255, SEGMENT_RUNTIME.timebase); // was int thatPhase = 64 - beatsin88((SEGMENT.beat88*11)/10, 0, 128, SEGMENT_RUNTIME.timebase);

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

  const uint8_t width = max(_segment_runtime.length / 15, 2);
  uint8_t curhue = 0;
  if (SEGMENT_RUNTIME.modeinit)
  {
    SEGMENT_RUNTIME.modeinit = false;
    SEGMENT_RUNTIME.modevars.juggle.thishue = 0;
  }
  curhue = SEGMENT_RUNTIME.modevars.juggle.thishue; // Reset the hue values.
  EVERY_N_MILLISECONDS(100)
  {
    SEGMENT_RUNTIME.modevars.juggle.thishue = random8(curhue, qadd8(curhue, 8));
  }

  fade_out(96);

  for (int i = 0; i < SEGMENT.numBars; i++)
  {
    uint16_t pos = beatsin88(max(SEGMENT.beat88 / 2, 1) + i * (_segment.beat88 / _segment.numBars), _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16, SEGMENT_RUNTIME.timebase);
    drawFractionalBar(pos, width, _currentPalette, curhue + (255 / SEGMENT.numBars) * i, _brightness);
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
  }
  CRGB newColor = CRGB::Black;
  uint8_t br, index;
  for (uint8_t k = _segment_runtime.start; k < _segment_runtime.stop; k++)
  {

    br = beatsin88(SEGMENT.beat88, 20, 255, SEGMENT_RUNTIME.timebase, k * 2); //= quadwave8(v1);
    index = (uint8_t)((uint8_t)triwave8(beat8(SEGMENT.beat88 >> 8) +
                                        (uint8_t)beatsin8(SEGMENT.beat88 >> 8, 0, 20) +
                                        (uint8_t)map(k, _segment_runtime.start, _segment_runtime.stop, 0, 255)));
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
  }
  fill_palette(&leds[_segment_runtime.start],
               (_segment_runtime.length),
               SEGMENT_RUNTIME.baseHue + (uint8_t)beatsin88(SEGMENT.beat88 * 2, 0, 255, SEGMENT_RUNTIME.timebase),
               // SEGMENT_RUNTIME.baseHue + triwave8( (uint8_t)map( beat88( max(  SEGMENT.beat88/4, 2), SEGMENT_RUNTIME.timebase), 0,  65535,  0,  255)),
               max(255 / _segment_runtime.length + 1, 1),
               _currentPalette,
               (uint8_t)beatsin88(max(SEGMENT.beat88 * 1, 1),
                                  _brightness / 10, 255,
                                  SEGMENT_RUNTIME.timebase),
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
  if (SEGMENT_RUNTIME.modeinit || (SRMVDB.num_bars != _segment.numBars))
  {
    SEGMENT_RUNTIME.modeinit = false;

    SRMVDB.num_bars = _segment.numBars;
    SRMVDB.oldVal = SEGMENT.beat88;
    uint32_t tb = millis();
    for(uint8_t i=0; i< SRMVDB.num_bars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3, 6)), SEGMENT.beat88);
      SRMVDB.timebases[i] = tb;
      SRMVDB.newBase[i] = false;
      SRMVDB.coff[i] = random8(i*(255/SRMVDB.num_bars), (i+1)*SRMVDB.num_bars);
    }
  }

  if (SRMVDB.oldVal != SEGMENT.beat88)
  {
    SRMVDB.oldVal = SEGMENT.beat88;
    for(uint8_t i = 0; i<SRMVDB.num_bars; i++)
    {
      SRMVDB.beats[i] = max((uint16_t)((SEGMENT.beat88 / random8(1, 3)) * random8(3, 6)), SEGMENT.beat88);
    }
  }

  uint16_t cled = 0;
  const uint8_t width = 2; //max(_segment_runtime.length/15, 2);

  fade_out(fade);

  for (uint8_t i = 0; i < SRMVDB.num_bars; i++)
  {
    uint8_t cind = 0;
    
    uint8_t sw = i%3;
    switch (sw)
    {
    case 0:
      cled = map(triwave16(beat88(SRMVDB.beats[i], SRMVDB.timebases[i])), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

      break;
    case 1:
      cled = map(quadwave16(beat88(SRMVDB.beats[i], SRMVDB.timebases[i])), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

      break;
    case 2:
      cled = map(cubicwave16(beat88(SRMVDB.beats[i], SRMVDB.timebases[i])), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

      break;
    default:
      cled = map(quadwave16(beat88(SRMVDB.beats[i], SRMVDB.timebases[i])), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

      break;
    }

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

      SRMVDB.coff[i] = get_random_wheel_index(SRMVDB.coff[i]); //random8(coff[i], 255) + rnd_hue;
    }
    else
    {
      SRMVDB.newBase[i] = true;
    }

    cind = SRMVDB.coff[i]; // + map(cled/16, _segment_runtime.start, _segment_runtime.stop , 0, 255);

    drawFractionalBar(cled, width, _currentPalette, cind, _brightness, false);
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
    _segment_runtime.modevars.col_wipe.npos = 0;
    _segment_runtime.modevars.col_wipe.prev = _segment_runtime.start;
    SEGMENT_RUNTIME.modevars.col_wipe.newcolor = false;
  }

  switch (mode)
  {
  case 0:
    i = beatsin16((SEGMENT.beat88 * 2) % 65535, _segment_runtime.start, _segment_runtime.stop, SEGMENT_RUNTIME.timebase);
    break;
  case 1:
    i = triwave16(beat88((SEGMENT.beat88 * 2) % 65535, SEGMENT_RUNTIME.timebase));
    i = map(i, 0, 65534, _segment_runtime.start, _segment_runtime.stop + 2);
    break;
  case 2:
    i = quadwave16(beat88((SEGMENT.beat88 * 2) % 65535, SEGMENT_RUNTIME.timebase));
    i = map(i, 0, 65535, _segment_runtime.start, _segment_runtime.stop + 2);
    break;
  case 3:
    i = beat88((SEGMENT.beat88 * 4) % 65535, SEGMENT_RUNTIME.timebase);
    i = map(i, 0, 65535, _segment_runtime.start, _segment_runtime.stop + 2);
    break;
  default:
    i = _segment_runtime.start;
    fill_solid(leds, _segment_runtime.length, CRGB::Black);
  }

  if (i >= _segment_runtime.stop)
    i = _segment_runtime.stop;

  if (i == _segment_runtime.start || i == _segment_runtime.stop)
  {
    SEGMENT_RUNTIME.modevars.col_wipe.newcolor = true;
  }
  else
  {
    if (SEGMENT_RUNTIME.modevars.col_wipe.newcolor)
    {
      _segment_runtime.modevars.col_wipe.npos = get_random_wheel_index(_segment_runtime.modevars.col_wipe.npos, 16);
      SEGMENT_RUNTIME.modevars.col_wipe.newcolor = false;
    }
  }
  if (_segment_runtime.modevars.col_wipe.prev > i)
  {
    fill_solid(&leds[i], _segment_runtime.modevars.col_wipe.prev - i + 1, ColorFromPalette(_currentPalette, _segment_runtime.modevars.col_wipe.npos, _brightness, SEGMENT.blendType));
  }
  else if (_segment_runtime.modevars.col_wipe.prev < i)
  {
    fill_solid(&leds[_segment_runtime.modevars.col_wipe.prev], i - _segment_runtime.modevars.col_wipe.prev + 1, ColorFromPalette(_currentPalette, _segment_runtime.modevars.col_wipe.npos, _brightness, SEGMENT.blendType));
  }
  else
  {
    leds[i] = ColorFromPalette(_currentPalette, _segment_runtime.modevars.col_wipe.npos, _brightness, SEGMENT.blendType);
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
  }
  uint16_t led_up_to = (((_segment_runtime.length) / 2 + 1) + _segment_runtime.start);
  uint8_t fade = SEGMENT.beat88 * 5 <= 16320 ? (SEGMENT.beat88 * 5) >> 6 : 255;
  SEGMENT.blur = max(fade, (uint8_t)16);
  fade_out(max(fade, (uint8_t)16)); //(64);

  fill_palette(&leds[_segment_runtime.start],
               beatsin88(
                   SEGMENT.beat88 < 13107 ? SEGMENT.beat88 * 5 : 65535,
                   0, led_up_to, SEGMENT_RUNTIME.timebase),
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
  }
  fill_palette(&leds[_segment_runtime.start], _segment_runtime.length, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, beatsin88(SEGMENT.beat88 * 2, 10, 255, SEGMENT_RUNTIME.timebase), SEGMENT.blendType);
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
  }
  fill_palette(&leds[_segment_runtime.start], (_segment_runtime.length), beat88(max((SEGMENT.beat88 / 128), 2), SEGMENT_RUNTIME.timebase),
               max(255 / _segment_runtime.length + 1, 1), _currentPalette, beatsin88(max(SEGMENT.beat88 / 32, 1), 10, 255, SEGMENT_RUNTIME.timebase), SEGMENT.blendType);
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_firework(void)
{
  const uint8_t dist = max(_segment_runtime.length / 20, 2);

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    fill_solid(leds, LED_COUNT, CRGB::Black);

    memset(_segment_runtime.modevars.firework.colors, 0x0, _segment_runtime.length * sizeof(uint8_t));
    memset(_segment_runtime.modevars.firework.keeps, 0x0, _segment_runtime.length * sizeof(uint8_t));
  }

  if (_segment_runtime.modevars.firework.colors == NULL || _segment_runtime.modevars.firework.keeps == NULL)
    return STRIP_MIN_DELAY;

  blur1d(&leds[_segment_runtime.start], _segment_runtime.length, qadd8(255-(SEGMENT.beat88 >> 8), 32)%172); // 172); //qadd8(255-(SEGMENT.beat88 >> 8), 32)%172); //was 2 instead of 16 before!

  for (uint16_t i = _segment_runtime.start; i < _segment_runtime.length; i++)
  {
    if (_segment_runtime.modevars.firework.keeps[i])
    {
      _segment_runtime.modevars.firework.keeps[i]--;
      nblend(leds[i], ColorFromPalette(_currentPalette, _segment_runtime.modevars.firework.colors[i], 255, SEGMENT.blendType), 196);
      //leds[i] = ColorFromPalette(_currentPalette, colors[i]  , 255, SEGMENT.blendType);
    }
  }

  if (random8(max(6, _segment_runtime.length / 7)) <= max(3, _segment_runtime.length / 14))
  {
    uint16_t lind = random16(dist + _segment_runtime.start, _segment_runtime.stop - dist);
    uint8_t cind = random8() + SEGMENT_RUNTIME.baseHue;
    for (int8_t i = 0 - dist; i <= dist; i++)
    {
      if (lind + i >= _segment_runtime.start && lind + i < _segment_runtime.stop)
      {
        if (!(leds[lind + i] == CRGB(0x0)))
          return STRIP_MIN_DELAY;
      }
    }
    _segment_runtime.modevars.firework.colors[lind] = cind;
    leds[lind] = ColorFromPalette(_currentPalette, cind, 255, SEGMENT.blendType);
    _segment_runtime.modevars.firework.keeps[lind] = random8(10, 30);
  }

  //addSparks(100, true, true);

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
  }

  fill_palette(&(leds[_segment_runtime.start]), _segment_runtime.length, 0 + SEGMENT_RUNTIME.baseHue, 5, _currentPalette, map8(triwave8(map(beat88(SEGMENT.beat88 * 10, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 24, 255), SEGMENT.blendType);

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
  }
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, _segment_runtime.start*16, _segment_runtime.stop*16);
  const uint16_t width = 2; // max(2, _segment_runtime.length/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase)), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

  // maybe we change this to fade?
  fill_solid(&(leds[_segment_runtime.start]), _segment_runtime.length, CRGB(0, 0, 0));

  drawFractionalBar(_segment_runtime.start * 16 + led_offset, width, _currentPalette, led_offset / 16 + SEGMENT_RUNTIME.baseHue, _brightness);

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
  }
  //uint16_t led_offset = map(triwave8(map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255)), 0, 255, _segment_runtime.start*16, _segment_runtime.stop*16);
  const uint16_t width = 2; // max(2, _segment_runtime.length/50)
  uint16_t led_offset = map(triwave16(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase)), 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

  fill_solid(&(leds[_segment_runtime.start]), _segment_runtime.length, CRGB(0, 0, 0));

  drawFractionalBar(_segment_runtime.stop * 16 - led_offset, width, _currentPalette, 255 - led_offset / 16 + SEGMENT_RUNTIME.baseHue, _brightness);
  drawFractionalBar(_segment_runtime.start * 16 + led_offset, width, _currentPalette, led_offset / 16 + SEGMENT_RUNTIME.baseHue, _brightness);

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
  }

  fill_solid(&leds[_segment_runtime.start], _segment_runtime.length, ColorFromPalette(_currentPalette, map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255), _brightness, SEGMENT.blendType)); /*CHSV(beat8(max(SEGMENT.beat/2,1), SEGMENT_RUNTIME.timebase)*/ //_brightness));
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
  }

  fill_palette(&leds[_segment_runtime.start],
               _segment_runtime.length,
               map(beat88(SEGMENT.beat88,
                          SEGMENT_RUNTIME.timebase),
                   0, 65535, 0, 255),
               (_segment_runtime.length > 255 ? 1 : (256 / _segment_runtime.length)),
               _currentPalette,
               255, SEGMENT.blendType);

  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pride(void)
{
  return pride(false);
}

uint16_t WS2812FX::mode_pride_glitter(void)
{
  return pride(true);
}

/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(CRGBPalette16 color1, CRGBPalette16 color2)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }
  uint16_t off = map(beat88(SEGMENT.beat88 / 2, SEGMENT_RUNTIME.timebase), 0, 65535, 0, 255) % 3;

  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    uint8_t pal_index = map(i, 0, _segment_runtime.length - 1, 0, 255) + SEGMENT_RUNTIME.baseHue;
    if ((i % 3) == off)
    {
      leds[_segment_runtime.start + i] = ColorFromPalette(color1, pal_index, _brightness, SEGMENT.blendType);
    }
    else
    {
      leds[_segment_runtime.start + i] = ColorFromPalette(color2, 128 + pal_index, _brightness / 10, SEGMENT.blendType);
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
  return theater_chase(_currentPalette, CRGBPalette16(CRGB::Black));
}

uint16_t WS2812FX::mode_theater_chase_dual_pal(void)
{
  return theater_chase(_currentPalette, _currentPalette);
}

/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void)
{
  SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step = (SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step + 1) & 0xFF;
  return theater_chase(CRGBPalette16(ColorFromPalette(_currentPalette, SEGMENT_RUNTIME.modevars.theater_chase.counter_mode_step)), CRGBPalette16(CRGB::Black));
}

/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }
  for (uint16_t i = 0; i < _segment_runtime.length; i++)
  {
    uint8_t lum = qsub8(sin8_C(map(i, 0, _segment_runtime.length - 1, 0, 255)), 2);
    uint16_t offset = map(beat88(SEGMENT.beat88, SEGMENT_RUNTIME.timebase), 0, 65535, 0, _segment_runtime.length - 1);
    offset = (offset + i) % _segment_runtime.length;

    CRGB newColor = CRGB::Black;

    newColor = ColorFromPalette(_currentPalette, map(offset, 0, _segment_runtime.length - 1, 0, 255) + SEGMENT_RUNTIME.baseHue, lum, SEGMENT.blendType);
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
  fade_out(qadd8(SEGMENT.beat88 >> 8, 12));
  addSparks(4, true, false);
  return STRIP_MIN_DELAY;
}

/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void)
{
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
  }

  const uint16_t width = max(1, _segment_runtime.length / 15);
  fade_out(96);

  uint16_t pos = triwave16(beat88(SEGMENT.beat88 * 4, SEGMENT_RUNTIME.timebase));

  pos = map(pos, 0, 65535, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16);

  drawFractionalBar(pos,
                    width,
                    _currentPalette,
                    SEGMENT_RUNTIME.baseHue + map(pos, _segment_runtime.start * 16, _segment_runtime.stop * 16 - width * 16, 0, 255));

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
  }
  const uint16_t width = max(1, _segment_runtime.length / 15);
  fade_out(96);

  uint16_t pos = map(beat88(SEGMENT.beat88 * 4, SEGMENT_RUNTIME.timebase), 0, 65535, 0, _segment_runtime.length * 16);

  drawFractionalBar((_segment_runtime.start * 16 + pos),
                    width,
                    _currentPalette,
                    map(_segment_runtime.start * 16 + pos, _segment_runtime.start * 16, _segment_runtime.stop * 16, 0, 255) + SEGMENT_RUNTIME.baseHue);

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
    leds[i] = ColorFromPalette(_currentPalette, map(i, _segment_runtime.start, _segment_runtime.stop, 0, 255) + SEGMENT_RUNTIME.baseHue, _brightness, SEGMENT.blendType);
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
  const uint16_t framedelay = map(_segment.beat88, 10000, 0, 0, 50) + map(_segment_runtime.length, 300, 0, 0, 25);

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
    CRGB c = computeOneTwinkle(myclock30, myunique8);

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
  EVERY_N_MILLIS(speed)
  {
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
  
  if (_segment_runtime.modeinit || SEGMENT.numBars != SRMVSS.numBars || SRMVSS.basebeat != SEGMENT.beat88)
  {
    _segment_runtime.modeinit = false;
    if (SEGMENT.numBars >= (LED_COUNT / SEGMENT.segments / 10))
      SEGMENT.numBars = max((LED_COUNT / SEGMENT.segments / 10), 2);
    
    SRMVSS.numBars = SEGMENT.numBars;
    SRMVSS.basebeat = SEGMENT.beat88;

    
    for (uint8_t i = 0; i < SRMVSS.numBars; i++)
    {
      SRMVSS.delta_b[i] = (65535 / SRMVSS.numBars) * i;
      if (i > 0)
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[i - 1], 16);
      else
        SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SRMVSS.numBars - 1], 16);

      SRMVSS.new_cind[i] = false;
    }
  }

  fadeToBlackBy(leds, _segment_runtime.length > 8 ? _segment_runtime.length - 8 : _segment_runtime.length, (SEGMENT.beat88 >> 8) | 0x60);
  if (_segment_runtime.length > 8)
    blur1d(&leds[_segment_runtime.stop - 7], 8, 120);

  for (uint8_t i = 0; i < SRMVSS.numBars; i++)
  {
    uint16_t beat = beat88(SEGMENT.beat88) + SRMVSS.delta_b[i];

    double_t q_beat = (beat / 100) * (beat / 100);
    pos = map(static_cast<uint32_t>(q_beat + 0.5), 0, 429484, _segment_runtime.start * 16, _segment_runtime.stop * 16);

    //we use the fractional bar and 16 brghtness values per pixel
    drawFractionalBar(pos, 2, _currentPalette, SRMVSS.cind[i], _brightness);

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
          SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[i - 1], 16);
        else
          SRMVSS.cind[i] = get_random_wheel_index(SRMVSS.cind[SRMVSS.numBars - 1], 16);
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

  if (_segment_runtime.modeinit || SRMVGB.num_bars != SEGMENT.numBars || SRMVGB.basebeat != SEGMENT.beat88)
  {
    _segment_runtime.modeinit = false;
    _transition = true;
    _blend = 0;
    if (SEGMENT.numBars >= (LED_COUNT / SEGMENT.segments / 10))
      SEGMENT.numBars = max((LED_COUNT / SEGMENT.segments / 10), 2);
    SRMVGB.num_bars = SEGMENT.numBars;
    SRMVGB.basebeat = SEGMENT.beat88;
    
    for (uint8_t i = 0; i < SRMVGB.num_bars; i++)
    {
      SRMVGB.beats[i] = SEGMENT.beat88 + lim / 2 - random16(lim);
      SRMVGB.theta[i] = (65535 / SRMVGB.num_bars) * i + (65535 / (4 * SRMVGB.num_bars)) - random16(65535 / (2 * SRMVGB.num_bars));
      uint8_t temp = random8(255 / (2 * SRMVGB.num_bars));
      if (temp & 0x01)
      {
        SRMVGB.cinds[i] = (255 / SRMVGB.num_bars) * i - temp;
      }
      else
      {
        SRMVGB.cinds[i] = (255 / SRMVGB.num_bars) * i + temp;
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
    drawFractionalBar(pos, 2, _currentPalette, SRMVGB.cinds[i] + i * (255 / SRMVGB.num_bars), _brightness, true);
  }
  #undef SRMVGB
  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_pixel_stack(void)
{
  #define SRMVPS _segment_runtime.modevars.pixel_stack
  const uint16_t framedelay = map(_segment.beat88, 10000, 0, 0, 50);// + map(_segment_runtime.length, 300, 0, 0, 25);

  const uint16_t cStartPos = _segment_runtime.length / 2 - 1;
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

    SRMVPS.up = true;
    SRMVPS.leds_moved = 0;
    SRMVPS.pos = cStartPos;

    fill_solid(leds, _segment_runtime.length, CRGB::Black);
    fill_palette(leds, _segment_runtime.length / 2, _segment_runtime.baseHue, max(255 / (_segment_runtime.length / 2), 1), _currentPalette, 255, _segment.blendType);
  }
  if (SRMVPS.up)
  {
    if (SRMVPS.pos == (_segment_runtime.stop - SRMVPS.leds_moved))
    {
      SRMVPS.leds_moved++;
      if (SRMVPS.leds_moved > cStartPos)
      {
        SRMVPS.up = false;
        SRMVPS.leds_moved = 0;
        SRMVPS.pos = cStartPos + 1;
        return 0;
      }
      SRMVPS.pos = cStartPos - SRMVPS.leds_moved;
    }
    if (SRMVPS.pos < _segment_runtime.stop && SRMVPS.pos >= 0 && !leds[SRMVPS.pos + 1])
    {
      leds[SRMVPS.pos + 1] = leds[SRMVPS.pos];
      leds[SRMVPS.pos] = CRGB::Black;
    }
    SRMVPS.pos++;
  }
  else
  {
    if (SRMVPS.pos < _segment_runtime.start + SRMVPS.leds_moved)
    {
      SRMVPS.leds_moved++;
      if (SRMVPS.leds_moved > cStartPos + 1)
      {
        SRMVPS.up = true;
        SRMVPS.leds_moved = 0;
        SRMVPS.pos = cStartPos;
        return 0;
      }
      SRMVPS.pos = cStartPos + 1 + SRMVPS.leds_moved;
    }
    if (SRMVPS.pos <= _segment_runtime.stop && SRMVPS.pos > 0 && !leds[SRMVPS.pos - 1])
    {
      leds[SRMVPS.pos - 1] = leds[SRMVPS.pos];
      leds[SRMVPS.pos] = CRGB::Black;
    }
    SRMVPS.pos--;
  }

  return framedelay;
}

uint16_t WS2812FX::mode_popcorn(void)
{
#define LEDS_PER_METER 60
  const double segmentLength = ((double)_segment_runtime.length / (double)LEDS_PER_METER) * (double)1000.0; // physical length in mm
  const double gravity = (double)_segment.beat88 / (double)-1019367.99184506;                                          // -0.00981; // gravity in mm per ms²
  const double v0_max = sqrt(-2 * gravity * segmentLength);

  

  fade_out(96);

  uint32_t now = millis();

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

    for (uint16_t i = 0; i < _segment.numBars; i++)
    {
      _segment_runtime.pops[i].timebase = now;
      _segment_runtime.pops[i].color_index = 256 / _segment.numBars * i; //get_random_wheel_index(pops[i].color_index);
      _segment_runtime.pops[i].pos = 0;
      _segment_runtime.pops[i].prev_pos = 0;
      _segment_runtime.pops[i].v0 = 0;
    }
  }

  for (uint16_t i = 0; i < _segment.numBars; i++)
  {
    if (_segment_runtime.pops[i].pos <= 0)
    {
      if (_segment_runtime.pops[i].v0 <= 0.01)
      {
        if (random8() < 1)
        {
          _segment_runtime.pops[i].v0 = (double)random((long)(v0_max * 850), (long)(v0_max * 1000)) / 1000.0; //v0_max - random(v0_max*100)/400;
          _segment_runtime.pops[i].pos = 0.00001;
          _segment_runtime.pops[i].prev_pos = 0;
          _segment_runtime.pops[i].timebase = now;
          _segment_runtime.pops[i].color_index = get_random_wheel_index(_segment_runtime.pops[i].color_index);
        }
      }
      else
      {
        float damping = 0.1f / 100.0f;
        if (_segment.damping)
        {
          if (_segment.damping <= 100)
          {
            damping = ((float)_segment.damping / 100.0f);
          }
          else
          {
            damping = 1.0f;
          }
        }

        _segment_runtime.pops[i].v0 = _segment_runtime.pops[i].v0 * damping;
        if (damping < 1.0f)
          _segment_runtime.pops[i].v0 -= 0.01;
        _segment_runtime.pops[i].pos = 0.00001;
        _segment_runtime.pops[i].prev_pos = 0;
        _segment_runtime.pops[i].timebase = now;
      }
    }
    else
    {
      uint16_t pos = map((long)_segment_runtime.pops[i].pos, 0, (long)segmentLength, _segment_runtime.start * 16, _segment_runtime.stop * 16);
      if (pos != _segment_runtime.pops[i].prev_pos)
      {
        if (pos > _segment_runtime.pops[i].prev_pos)
        {
          uint16_t width = max((pos - _segment_runtime.pops[i].prev_pos) / 16, 1);
          drawFractionalBar(_segment_runtime.pops[i].prev_pos, width, _currentPalette, _segment_runtime.pops[i].color_index, _brightness, true);
        }
        else
        {
          uint16_t width = max((_segment_runtime.pops[i].prev_pos - pos) / 16, 1);
          drawFractionalBar(pos, width, _currentPalette, _segment_runtime.pops[i].color_index, _brightness, true);
        }
      }
      else
      {
        drawFractionalBar(pos, 1, _currentPalette, _segment_runtime.pops[i].color_index, _brightness, true);
      }

      _segment_runtime.pops[i].prev_pos = pos;

      double mtime = now - _segment_runtime.pops[i].timebase; //beat88(_segment.beat88, pops[i].timebase) & max_time; //max(beat8(max(_segment.beat88/255,1), pops[i].timebase), (uint8_t)1); //((now/1000-pops[i].timebase/1000)&0xff);

      if (mtime != 0)
      {
        _segment_runtime.pops[i].pos = (gravity / 2.0f) * (mtime * mtime) + _segment_runtime.pops[i].v0 * mtime;
      }
      else
      {
        _segment_runtime.pops[i].pos = 0.00001;
      }
    }
  }

  return STRIP_MIN_DELAY;
}

uint16_t WS2812FX::mode_firework2(void)
{

  const double segmentLength = ((double)(_segment_runtime.length-BLENDWIDTH) / (double)LEDS_PER_METER) * (double)1000.0; // physical length in mm
  const double gravity = (double)_segment.beat88 / (double)-1019367.99184506;                                          // -0.00981; // gravity in mm per ms²
  const double v0_max = sqrt(-2 * gravity * segmentLength);

  uint32_t now = millis();

  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;

    for (uint16_t i = 0; i < _segment.numBars; i++)
    {
      fill_solid(_segment_runtime.pops[i].dist, BLENDWIDTH, CRGB::Black);
      _segment_runtime.pops[i].timebase = now;
      _segment_runtime.pops[i].color_index = 256 / _segment.numBars * i; //get_random_wheel_index(pops[i].color_index);
      _segment_runtime.pops[i].pos = 0;
      _segment_runtime.pops[i].prev_pos = 0;
      _segment_runtime.pops[i].v0 = 0;
      _segment_runtime.pops[i].v = 0;
      //_segment_runtime.pops[i].ignite = false;
      _segment_runtime.pops[i].P_ignite = random(100, 500);
      _segment_runtime.pops[i].v_explode = _segment_runtime.pops[i].v0 * ((double)random8(0, 10) / 100.0);
    }
  }
  fade_out(64);
  for (uint16_t i = 0; i < _segment.numBars; i++) {
    if (_segment_runtime.pops[i].pos <= 0) {
      if (_segment_runtime.pops[i].v0 <= 0.01) {
        if (random8() < 2) {
          _segment_runtime.pops[i].v0 = (double)random((long)(v0_max * 750), (long)(v0_max * 990)) / 1000.0;
          _segment_runtime.pops[i].v = _segment_runtime.pops[i].v0;
          _segment_runtime.pops[i].pos = 0.00001;
          _segment_runtime.pops[i].prev_pos = 0;
          _segment_runtime.pops[i].timebase = now;
          _segment_runtime.pops[i].color_index = get_random_wheel_index(_segment_runtime.pops[i].color_index);
          //_segment_runtime.pops[i].ignite = false;
          _segment_runtime.pops[i].P_ignite = random(100, 500);
          uint16_t t_max = (_segment_runtime.pops[i].v0 / (-1 * gravity)) + 10 * STRIP_MIN_DELAY;
          _segment_runtime.pops[i].explodeTime = random(t_max / 2, t_max * 4 / 5);
          _segment_runtime.pops[i].v_explode = _segment_runtime.pops[i].v0 * ((double)random8(0, 10) / 100.0);
          fill_solid(_segment_runtime.pops[i].dist, 20, CRGB::Black);
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

        _segment_runtime.pops[i].v0 = 0.001; //pops[i].v0 * damping;

        if (damping < 1.0f) _segment_runtime.pops[i].v0 -= 0.01;

        _segment_runtime.pops[i].v = _segment_runtime.pops[i].v0;
        _segment_runtime.pops[i].pos = 0.00001;
        _segment_runtime.pops[i].prev_pos = 0;
        _segment_runtime.pops[i].timebase = now;
      }
    } else {
      

      if ((_segment_runtime.pops[i].explodeTime > STRIP_MIN_DELAY) && (_segment_runtime.pops[i].prev_pos > _segment_runtime.length/2)) {
        _segment_runtime.pops[i].explodeTime -= STRIP_MIN_DELAY/2; //STRIP_MIN_DELAY;
        //_segment_runtime.pops[i].ignite = true;
        _segment_runtime.pops[i].dist[BLENDWIDTH / 2-1] += ColorFromPalette(_currentPalette, _segment_runtime.pops[i].color_index);
        _segment_runtime.pops[i].dist[BLENDWIDTH / 2+1] += ColorFromPalette(_currentPalette, _segment_runtime.pops[i].color_index);
      } else {
        //_segment_runtime.pops[i].ignite = false;
      }
      
      uint16_t pos = map((long)_segment_runtime.pops[i].pos, 0, (long)segmentLength, _segment_runtime.start * 16, _segment_runtime.stop * 16);
      if (_segment_runtime.pops[i].v > _segment_runtime.pops[i].v_explode) {
        //fade_out(64);
        if (pos != _segment_runtime.pops[i].prev_pos) {
          if (pos > _segment_runtime.pops[i].prev_pos) {
            uint16_t width = max((pos - _segment_runtime.pops[i].prev_pos) / 16, 1);
            drawFractionalBar(_segment_runtime.pops[i].prev_pos, width, _currentPalette, 0, 255, true);// _segment_runtime.pops[i].color_index, _brightness, true);
          } else {
            uint16_t width = max((_segment_runtime.pops[i].prev_pos - pos) / 16, 1);
            drawFractionalBar(pos, width, _currentPalette, 0, 255, true);// _segment_runtime.pops[i].color_index, _brightness, true);
          }
        } else {
          drawFractionalBar(pos, 2, _currentPalette, 0, 255, true);// _segment_runtime.pops[i].color_index, _brightness, true);
        }
      } else {
        //fade_out(4);
        blur1d(_segment_runtime.pops[i].dist, BLENDWIDTH, 192);
        fadeToBlackBy(_segment_runtime.pops[i].dist, BLENDWIDTH, 64);
        if ((pos / 16 + 1) >= (_segment_runtime.start + BLENDWIDTH / 2) && (pos / 16 + 1) <= (_segment_runtime.stop - BLENDWIDTH / 2))
        {
          nblend(&leds[pos / 16 + 1 - BLENDWIDTH / 2], _segment_runtime.pops[i].dist, BLENDWIDTH, 32);
        }
        else if ((pos / 16 + 1) > (_segment_runtime.stop - BLENDWIDTH / 2))
        {
          nblend(&leds[pos / 16 + 1 - BLENDWIDTH / 2], _segment_runtime.pops[i].dist, _segment_runtime.stop - pos / 16 + 1, 32);
        }
        else
        {
          nblend(&leds[_segment_runtime.start], &_segment_runtime.pops[i].dist[BLENDWIDTH / 2 - pos / 16 + 1], BLENDWIDTH / 2 + (BLENDWIDTH / 2 - pos / 16 + 1), 32);
        }
        if (!leds[pos / 16] && !leds[pos / 16 + 1])
        {
          _segment_runtime.pops[i].v0 = 0.001;
          _segment_runtime.pops[i].pos = 0;
          return STRIP_MIN_DELAY;
        }
      }

      _segment_runtime.pops[i].prev_pos = pos;

      double mtime = now - _segment_runtime.pops[i].timebase;

      if (mtime != 0)
      {
        _segment_runtime.pops[i].pos = (gravity / 2.0f) * (mtime * mtime) + _segment_runtime.pops[i].v0 * mtime;
        _segment_runtime.pops[i].v = gravity * mtime + _segment_runtime.pops[i].v0;
      }
      else
      {
        _segment_runtime.pops[i].v = 1000;
        _segment_runtime.pops[i].pos = 0.00001;
      }
    }
  }

  return STRIP_MIN_DELAY;
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
  
  fill_solid(leds, getStripLength(), calcSunriseColorValue(step));

  EVERY_N_MILLISECONDS(100)
  {
    for(uint16_t i=0; i<getStripLength(); i++)
    {
      SRMVSR.nc[i] = random8(0, 185);//step < 171 ? (step / 2) : 85); //(255 - step-5));
    }
  }
  for (uint16_t i = 0; i < getStripLength(); i++)
  {
    CRGB col;
    col = leds[i];
    col.nscale8_video(SRMVSR.nc[i]);  //random8(step < 172 ? (step / 2) : (255 - step))
    leds[i] = nblend(leds[i], col, 64);
  }
  #undef SRMVSR
}

uint16_t WS2812FX::getSunriseTimeToFinish(void)
{
  float time = (float)((_segment.sunrisetime * 60.0) / DEFAULT_SUNRISE_STEPS);
  if(getMode() == FX_MODE_SUNRISE)
  {
    return (uint16_t)(time * (DEFAULT_SUNRISE_STEPS - _segment_runtime.sunRiseStep));
  }
  else if (getMode() == FX_MODE_SUNSET) 
  {
    return (uint16_t)(time * _segment_runtime.sunRiseStep);
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
  #ifdef DEBUG
    EVERY_N_MILLISECONDS(500)
    {
      Serial.print("SRSS_Step: ");
      Serial.print(step);
      Serial.print("\tuv ");
      Serial.print(uv);
      Serial.print("\tred ");
      Serial.print(red);
      Serial.print("\tgreen ");
      Serial.print(green);
      Serial.print("\tblue ");
      Serial.print(blue);
      Serial.printf("\tr %d\tg %d\t b %d\n", (uint8_t)red, (uint8_t)green, (uint8_t)blue);
    }
    
  #endif
  return CRGB((uint8_t)red, (uint8_t)green, (uint8_t)blue);
}

void WS2812FX::m_sunrise_sunset(bool isSunrise)
{
  #define SRMVSR _segment_runtime.modevars.sunrise_step
  const uint16_t sunriseSteps = DEFAULT_SUNRISE_STEPS;
  uint16_t stepInterval = (uint16_t)(_segment.sunrisetime * ((60 * 1000) / sunriseSteps));
  if (_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment.autoplay = AUTO_MODE_OFF;
    if (isSunrise)
    {
      _segment.targetBrightness = 255;
      _segment_runtime.sunRiseStep = 0;
      //setTargetPalette(HeatColor(255), F("Sunrise End"));
    }
    else
    {
      _segment_runtime.sunRiseStep = sunriseSteps;
    }
  }
  draw_sunrise_step(_segment_runtime.sunRiseStep);
  if (millis() > SRMVSR.next)
  {
    SRMVSR.next = millis() + stepInterval;
    if (isSunrise)
    {
      if(_segment_runtime.sunRiseStep < sunriseSteps)
      {
        _segment_runtime.sunRiseStep++;
      }
    }
    else
    {
      if(_segment_runtime.sunRiseStep > 0)
      {
        _segment_runtime.sunRiseStep--;
      }
      else
      {
        // we switch off - this should fix issue #6
        setMode(DEFAULT_MODE);
        setIsRunning(false);
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
  uint32_t now = millis();
  if(_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    _segment_runtime.modevars.ring_ring.isOn = true;
    _segment_runtime.modevars.ring_ring.nextmillis = 0; 
    _segment_runtime.modevars.ring_ring.pausemillis = 0;
    _segment_runtime.modevars.ring_ring.isPause = 0;
  }
  if(_segment_runtime.modevars.ring_ring.isPause)
  {
    fadeToBlackBy(leds, _segment_runtime.length, 32);
    //fill_solid(leds, _segment_runtime.length, CRGB::Black);
    if(now > (_segment_runtime.modevars.ring_ring.pausemillis + pauseTime))
    {
      _segment_runtime.modevars.ring_ring.pausemillis = now;
      _segment_runtime.modevars.ring_ring.isPause = false;
    }
  }
  else
  {
    if(_segment_runtime.modevars.ring_ring.isOn)
    {
      fill_palette(leds, _segment_runtime.length, SEGMENT_RUNTIME.baseHue, (_segment_runtime.length > 255 ? 1 : (256 / _segment_runtime.length)), _currentPalette, 255, SEGMENT.blendType);
      if(now > (_segment_runtime.modevars.ring_ring.nextmillis + onTime))
      {
        _segment_runtime.modevars.ring_ring.nextmillis = now;
        _segment_runtime.modevars.ring_ring.isOn = false;
      }
    }
    else
    {
      fill_solid(leds, _segment_runtime.length, CRGB::Black);
      if(now > (_segment_runtime.modevars.ring_ring.nextmillis + offTime))
      {
        _segment_runtime.modevars.ring_ring.nextmillis = now;
        _segment_runtime.modevars.ring_ring.isOn = true;
      }
    }
    if(now > (_segment_runtime.modevars.ring_ring.pausemillis + runTime))
    {
      _segment_runtime.modevars.ring_ring.pausemillis = now;
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
  
  #define BEATS_PER_MINUTE (_segment.beat88>20?_segment.beat88 / 20 : 1)
  #define MS_PER_BEAT (60000 / BEATS_PER_MINUTE)
  #define SECOND_BEAT (MS_PER_BEAT / 3)
  #define M_HEARTBEAT_RT _segment_runtime.modevars.heartBeat
  
  if(_segment_runtime.modeinit)
  {
    _segment_runtime.modeinit = false;
    M_HEARTBEAT_RT.lastBeat = 0;
    M_HEARTBEAT_RT.secondBeatActive = false; 
  }

  
  // Get and translate the segment's size option
  uint8_t size = map(_segment_runtime.length, 25, 300, 1, 6);
  //uint8_t size = map(_segment.damping, 10, 100, 2, 32);
  // copy pixels from the middle of the segment to the edges
  uint16_t centerOffset = (_segment_runtime.length / 2);
  uint16_t pCount = centerOffset - size;
  for(uint16_t i = 0; i < pCount; i++)
  {
    leds[i] = leds[i + size];
    leds[i + centerOffset + size] = leds[i+centerOffset];
  }

  fadeToBlackBy(leds, _segment_runtime.length, (SEGMENT.beat88 >> 8) | 32);

  unsigned long beatTimer = millis() - M_HEARTBEAT_RT.lastBeat;
  if((beatTimer > SECOND_BEAT) && !M_HEARTBEAT_RT.secondBeatActive) { // time for the second beat?
    mode_heartbeat_beatIt(size, _segment_runtime.baseHue); // create the second beat
    M_HEARTBEAT_RT.secondBeatActive = true;
  }
  if(beatTimer > MS_PER_BEAT) { // time to reset the beat timer?
    mode_heartbeat_beatIt(size, _segment_runtime.baseHue); // create the first beat
    M_HEARTBEAT_RT.secondBeatActive = false;
    M_HEARTBEAT_RT.lastBeat = millis();
  }
  
  
  return STRIP_MIN_DELAY;

  #undef BEATS_PER_MINUTE
  #undef MS_PER_BEAT 
  #undef SECOND_BEAT 
  #undef M_HEARTBEAT_RT

}

// light up ('size' * 2) LEDs in the middle of the segment (starts a beat)
void WS2812FX::mode_heartbeat_beatIt(uint8_t size, uint8_t col_index) {
  
  uint16_t startLed = _segment_runtime.start + (_segment_runtime.length / 2) - size;
  fill_solid(&leds[startLed], size*2, ColorFromPalette(_currentPalette, col_index));
}