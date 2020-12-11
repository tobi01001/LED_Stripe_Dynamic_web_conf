/*
  WS2812FX.h - Library for WS2812 LED effects.

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

#ifndef WS2812FX_FastLED_h
#define WS2812FX_FastLED_h

/* <FastLED implementation> */
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_DMA
#define FASTLED_USE_PROGMEM 1

#pragma message "Needs to be corrected. A library should not depend on project inlcudes!"
// FIXME: Needs to be corrected. A library should not depend on project inlcudes!
#include "../../include/defaults.h"


#ifndef LED_PIN
#define LED_PIN 3
#endif

#define STRIP_MIN_DELAY max((uint32_t)((1000000 - FRAME_CALC_WAIT_MICROINTERVAL) / (_segment.fps * 1000)), (uint32_t)((MIN_LED_WRITE_CYCLE-FRAME_CALC_WAIT_MICROINTERVAL) / 1000))
#define STRIP_DELAY_MICROSEC  ((uint32_t)max((uint32_t)(1000000 / (_segment.fps)), (uint32_t)(MIN_LED_WRITE_CYCLE)))

#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE

/* </FastLED implementation> */

#ifdef SPEED_MAX
#error "SPEED_MAX define is no longer used!"
#endif

#ifdef SPEED_MIN
#error "SPEED_MIN define is no longer used!"
#endif

#define SEGMENT _segment
#define SEGMENT_RUNTIME _segment_runtime
// #define SEGMENT_LENGTH   LED_COUNT //(SEGMENT.stop - SEGMENT.start + 1)
// ToDo: Reset Runtime as inline funtion with new timebase?"
#define RESET_RUNTIME memset(&_segment_runtime, 0, sizeof(_segment_runtime))

// some common colors
#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define YELLOW 0xFFFF00
#define CYAN 0x00FFFF
#define MAGENTA 0xFF00FF
#define PURPLE 0x400080
#define ORANGE 0xFF3000
#define ULTRAWHITE 0xFFFFFFFF

enum MODES
{
  FX_MODE_STATIC,
  FX_MODE_EASE,
  FX_MODE_NOISEMOVER,
  FX_MODE_PLASMA,
  FX_MODE_JUGGLE_PAL,
  //  FX_MODE_CONFETTI,
  FX_MODE_FILL_BEAT,
  FX_MODE_FILL_WAVE,
  FX_MODE_DOT_BEAT,
  FX_MODE_DOT_COL_WIPE,
  FX_MODE_COLOR_WIPE_SAWTOOTH,
  FX_MODE_COLOR_WIPE_SINE,
  FX_MODE_COLOR_WIPE_QUAD,
  FX_MODE_COLOR_WIPE_TRIWAVE,
  FX_MODE_TO_INNER,
  FX_MODE_BREATH,
  FX_MODE_MULTI_DYNAMIC,
  FX_MODE_RAINBOW,
  FX_MODE_RAINBOW_CYCLE,
  FX_MODE_PRIDE,
  FX_MODE_SCAN,
  FX_MODE_DUAL_SCAN,
  FX_MODE_FADE,
  FX_MODE_THEATER_CHASE,
  FX_MODE_THEATER_CHASE_DUAL_P,
  FX_MODE_THEATER_CHASE_RAINBOW,
  FX_MODE_RUNNING_LIGHTS,
  FX_MODE_TWINKLE_FADE,
  FX_MODE_TWINKLE_FOX,
  FX_MODE_FILL_BRIGHT,
  FX_MODE_FIREWORK,
  FX_MODE_FIRE2012,
  FX_MODE_LARSON_SCANNER,
  FX_MODE_COMET,
  FX_MODE_FIRE_FLICKER_INTENSE,
  FX_MODE_BUBBLE_SORT,
  FX_MODE_SHOOTING_STAR,
  FX_MODE_BEATSIN_GLOW,
  FX_MODE_PIXEL_STACK,
  FX_MODE_MOVE_BAR_SIN,
  FX_MODE_MOVE_BAR_QUAD,
  FX_MODE_MOVE_BAR_CUBE,
  FX_MODE_MOVE_BAR_SAWTOOTH,
  FX_MODE_POPCORN,
  FX_MODE_FIREWORKROCKETS,
  FX_MODE_HEARTBEAT,
  FX_MODE_RAIN,
 
  FX_MODE_VOID,
  
  // make sure these are the last ones...
  FX_MODE_RING_RING,
  FX_MODE_SUNRISE,
  FX_MODE_SUNSET,

  // has to be the final entry!
  MODE_COUNT
};

extern const TProgmemRGBPalette16
    Ice_Colors_p,
    Ice_p,
    RetroC9_p,
    Snow_p,
    FairyLight_p,
    BlueWhite_p,
    RedWhite_p,
    Holly_p,
    RedGreenWhite_p,
    Shades_Of_Red_p,
    Shades_Of_Green_p,
    Shades_Of_Blue_p,
    Random_p;

enum PALETTES
{
  RAINBOW_PAL,
  LAVA_PAL,
  ICE_WATER_PAL,
  RAINBOWSTRIPES_PAL,
  FOREST_PAL,
  OCEAN_PAL,
  HEAT_PAL,
  PARTY_PAL,
  CLOUD_PAL,
  ICE_PAL,
  RETROC9_PAL,
  SNOW_PAL,
  FAIRYLIGHT_PAL,
  BLUEWHITE_PAL,
  REDWHITHE_PAL,
  HOLLY_PAL,
  REDGREENWHITE_PAL,
  SHADES_OF_RED_PAL,
  SHADES_OF_GREEN_PAL,
  SHADES_OF_BLUE_PAL,
  RANDOM_PAL,

  NUM_PALETTES

};

enum AUTOPLAYMODES
{
  AUTO_MODE_OFF,
  AUTO_MODE_UP,
  AUTO_MODE_DOWN,
  AUTO_MODE_RANDOM
};

#define qsubd(x, b) ((x > b) ? b : 0)     // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b) ((x > b) ? x - b : 0) // Analog Unsigned subtraction macro. if result <0, then => 0

class WS2812FX
{
  //class WS2812FX : public Adafruit_NeoPixel {

  typedef uint16_t (WS2812FX::*mode_ptr)(void);

  // segment parameters
public:
  typedef struct segment
  {
    uint16_t CRC;
    uint16_t beat88;

    ColorTemperature colorTemp;

    uint16_t hueTime;
    uint16_t milliamps;
    uint16_t autoplayDuration;
    uint16_t autoPalDuration;
    uint8_t segments;
    uint8_t cooling;
    uint8_t sparking;
    uint8_t twinkleSpeed;
    uint8_t twinkleDensity; 
    uint8_t numBars;
    uint8_t mode;
    uint8_t fps;
    uint8_t deltaHue;
    uint8_t blur;
    uint8_t damping;
    uint8_t dithering;
    uint8_t sunrisetime;
    uint8_t targetBrightness;
    uint8_t targetPaletteNum;
    uint8_t currentPaletteNum;
    uint8_t backgroundHue;
    uint8_t backgroundSat;
    uint8_t backgroundBri;
    bool power;
    bool isRunning;
    bool reverse;
    bool inverse;
    bool mirror;
    bool addGlitter;
    bool whiteGlitter;
    bool onBlackOnly;
    #ifdef HAS_KNOB_CONTROL
    bool wifiDisabled;
    #endif
    TBlendType blendType;
    AUTOPLAYMODES autoplay;
    AUTOPLAYMODES autoPal;
  } segment;

  // segment runtime parameters

  typedef struct {
    uint32_t timebase;
    double pos;
    double v0;
    double v;
    double v_explode;
    uint16_t prev_pos;
    uint16_t explodeTime;
    uint8_t damp;
    uint8_t color_index;
    uint8_t brightness;

  } pKernel;

  typedef union 
  {
    struct pride
    {
      uint16_t sPseudotime;
      uint16_t sLastMillis;
      uint16_t sHue16;
    } pride;
    struct ease
    {
      uint32_t timebase;
      uint16_t beat;
      uint16_t oldbeat;
      uint16_t p_lerp;
      bool trigger;
    } ease;
    struct inoise
    {
      uint32_t timebase;
      uint16_t dist;
    } inoise;
    struct plasma
    {
      uint32_t timebase;
    } plasma;
    struct fill_beat
    {
      uint32_t timebase;
    } fill_beat;
    struct fade
    {
      uint32_t timebase;
    } fade;
    struct scan
    {
      uint32_t timebase;
    } scan;
    struct dual_scan
    {
      uint32_t timebase;
    } dual_scan;
    struct rainbow
    {
      uint32_t timebase;
    } rainbow;
    struct rainbow_cycle
    {
      uint32_t timebase;
    } rainbow_cycle;
    struct fill_wave
    {
      uint32_t timebase;
    } fill_wave;
    struct juggle
    {
      uint32_t timebase;
      uint8_t thishue;
    } juggle;
    struct dot_beat
    {
      uint16_t oldVal;
      uint32_t timebases[MAX_NUM_BARS];
      uint16_t beats[MAX_NUM_BARS];
      uint8_t  coff[MAX_NUM_BARS];
      bool     newBase[MAX_NUM_BARS];
    } dot_beat;
    struct col_wipe
    {
      uint32_t timebase;
      uint16_t prev;
      uint8_t npos;
      bool newcolor;      
    } col_wipe;
    struct to_inner
    {
      uint32_t timebase;
    } to_inner;
    struct breath
    {
      uint32_t timebase;
    } breath;
    struct fill_bright
    {
      uint32_t timebase;
    } fill_bright;
    struct running_lights
    {
      uint32_t timebase;
    } running_lights;
    struct larson_scanner
    {
      uint32_t timebase;
    } larson_scanner;
    struct comet
    {
      uint32_t timebase;
    } comet;
    struct multi_dyn
    {
      uint8_t last_index;
      uint32_t last;
    } multi_dyn;
    struct firework
    {
      uint16_t  fireworks[MAX_NUM_BARS];
      uint8_t   colIndex[MAX_NUM_BARS];
      uint8_t   isBurning[MAX_NUM_BARS];
      //uint8_t colors[LED_COUNT];
      //uint8_t keeps [LED_COUNT];
    } firework;
    struct theater_chase
    {
      uint32_t counter_mode_step;
      uint32_t timebase;
    } theater_chase;
    struct bubble_sort
    {
      uint8_t hues[LED_COUNT];
      bool movedown;
      uint16_t ci;
      uint16_t co;
      uint16_t cd;
    } bubble_sort;
    struct fire2012
    {
      byte heat[LED_COUNT];
    } fire2012;
    struct soft_twinkle
    {
      uint8_t directionFlags [(LED_COUNT + 7)/8];
    } soft_twinkle;
    struct shooting
    {
      uint16_t basebeat;
      uint16_t delta_b[MAX_NUM_BARS];
      uint8_t  cind[MAX_NUM_BARS];
      bool     new_cind[MAX_NUM_BARS];
    } shooting;
    struct beatsin
    {
      uint16_t beats[MAX_NUM_BARS];
      uint16_t theta[MAX_NUM_BARS];
      int16_t  prev[MAX_NUM_BARS];
      uint32_t times[MAX_NUM_BARS];
      uint8_t  cinds[MAX_NUM_BARS];
      bool     newval[MAX_NUM_BARS];
    } beatsin;
    struct pixel_stack
    {
      bool up;
      int16_t leds_moved;
      uint16_t ppos16;
    } pixel_stack;
    struct ring_ring {
      uint32_t nextmillis;
      uint32_t pausemillis;
      uint32_t now;
      bool     isOn;
      bool     isPause;
    } ring_ring;
    struct sunrise
    {
      uint32_t next;
      
      bool toggle;

      uint8_t nc[LED_COUNT];
    } sunrise_step;
    struct heartBeat
    {
      uint32_t beatTimer;
      uint32_t lastBeat;
      
      uint16_t centerOffset;
      uint16_t pCount;
      uint16_t msPerBeat;
      uint16_t secondBeat;
      
      uint8_t  size;

      bool     secondBeatActive;
    } heartBeat;
    struct twinkle_fade
    {
      uint16_t numsparks;
    } twinkle_fade;
    struct pops
    {
      pKernel pops[MAX_NUM_BARS];
    } pops;
    struct rain
    {
      uint32_t timebase[MAX_NUM_BARS];
      uint8_t actives[MAX_NUM_BARS];
    } rain;
  } mode_variables;

  // to save some memory, all the "static" variables are now in unions
  // terrible to read but saving quite some ram.
  // other option would be to have dynamic effects (effect classes)
  // but thats risky because of memory leaks....
  typedef struct segment_runtime
  {
    bool modeinit;
    uint8_t baseHue;
    uint16_t start;
    uint16_t stop;
    uint16_t length;
    uint16_t sunRiseStep;
    // uint32_t timebase;
    uint32_t nextHue;
    uint32_t nextAuto;
    uint32_t nextPalette;
    uint32_t next_time;
    mode_variables modevars;
  } segment_runtime;

public:
  WS2812FX(const uint8_t volt = 5,
           const LEDColorCorrection colc = TypicalLEDStrip)
  {


    _bleds = new CRGB[LED_COUNT];
    leds = new CRGB[LED_COUNT];

    FastLED.addLeds<WS2812, LED_PIN, GRB>(_bleds, LED_COUNT);
    FastLED.setCorrection(colc); //TypicalLEDStrip);


    _mode[FX_MODE_STATIC]                 = &WS2812FX::mode_static;
    //_mode[FX_MODE_TWINKLE_EASE]           = &WS2812FX::mode_twinkle_ease;
    _mode[FX_MODE_EASE]                   = &WS2812FX::mode_ease;
    _mode[FX_MODE_MULTI_DYNAMIC]          = &WS2812FX::mode_multi_dynamic;
    _mode[FX_MODE_RAINBOW]                = &WS2812FX::mode_rainbow;
    _mode[FX_MODE_RAINBOW_CYCLE]          = &WS2812FX::mode_rainbow_cycle;
    _mode[FX_MODE_PRIDE]                  = &WS2812FX::mode_pride;
    //_mode[FX_MODE_PRIDE_GLITTER]          = &WS2812FX::mode_pride_glitter;
    _mode[FX_MODE_SCAN]                   = &WS2812FX::mode_scan;
    _mode[FX_MODE_DUAL_SCAN]              = &WS2812FX::mode_dual_scan;
    _mode[FX_MODE_FADE]                   = &WS2812FX::mode_fade;
    _mode[FX_MODE_THEATER_CHASE]          = &WS2812FX::mode_theater_chase;
    _mode[FX_MODE_THEATER_CHASE_DUAL_P]   = &WS2812FX::mode_theater_chase_dual_pal;
    _mode[FX_MODE_THEATER_CHASE_RAINBOW]  = &WS2812FX::mode_theater_chase_rainbow;
    _mode[FX_MODE_TWINKLE_FADE]           = &WS2812FX::mode_twinkle_fade;
    _mode[FX_MODE_TWINKLE_FOX]            = &WS2812FX::mode_twinkle_fox;
    //      _mode[FX_MODE_SOFTTWINKLES]            = &WS2812FX::mode_softtwinkles; // FIXME: Broken
    _mode[FX_MODE_LARSON_SCANNER]         = &WS2812FX::mode_larson_scanner;
    _mode[FX_MODE_COMET]                  = &WS2812FX::mode_comet;
    //_mode[FX_MODE_FIRE_FLICKER]           = &WS2812FX::mode_fire_flicker;
    //_mode[FX_MODE_FIRE_FLICKER_SOFT]      = &WS2812FX::mode_fire_flicker_soft;
    _mode[FX_MODE_FIRE_FLICKER_INTENSE]   = &WS2812FX::mode_fire_flicker_intense;
    _mode[FX_MODE_BREATH]                 = &WS2812FX::mode_breath;
    _mode[FX_MODE_RUNNING_LIGHTS]         = &WS2812FX::mode_running_lights;
    //_mode[FX_MODE_TWINKLE_NOISEMOVER]     = &WS2812FX::mode_inoise8_mover_twinkle;
    _mode[FX_MODE_NOISEMOVER]             = &WS2812FX::mode_inoise8_mover;
    _mode[FX_MODE_PLASMA]                 = &WS2812FX::mode_plasma;
    _mode[FX_MODE_JUGGLE_PAL]             = &WS2812FX::mode_juggle_pal;
    _mode[FX_MODE_FILL_BEAT]              = &WS2812FX::mode_fill_beat;
    _mode[FX_MODE_DOT_BEAT]               = &WS2812FX::mode_dot_beat;
    _mode[FX_MODE_DOT_COL_WIPE]           = &WS2812FX::mode_dot_col_move;
    _mode[FX_MODE_COLOR_WIPE_SAWTOOTH]    = &WS2812FX::mode_col_wipe_sawtooth;
    _mode[FX_MODE_COLOR_WIPE_SINE]        = &WS2812FX::mode_col_wipe_sine;
    _mode[FX_MODE_COLOR_WIPE_QUAD]        = &WS2812FX::mode_col_wipe_quad;
    _mode[FX_MODE_COLOR_WIPE_TRIWAVE]     = &WS2812FX::mode_col_wipe_triwave;
    _mode[FX_MODE_TO_INNER]               = &WS2812FX::mode_to_inner;
    _mode[FX_MODE_FILL_BRIGHT]            = &WS2812FX::mode_fill_bright;
    _mode[FX_MODE_FIREWORK]               = &WS2812FX::mode_firework;
    _mode[FX_MODE_FIRE2012]               = &WS2812FX::mode_fire2012WithPalette;
    _mode[FX_MODE_FILL_WAVE]              = &WS2812FX::mode_fill_wave;
    _mode[FX_MODE_BUBBLE_SORT]            = &WS2812FX::mode_bubble_sort;
    _mode[FX_MODE_SHOOTING_STAR]          = &WS2812FX::mode_shooting_star;
    _mode[FX_MODE_BEATSIN_GLOW]           = &WS2812FX::mode_beatsin_glow;
    _mode[FX_MODE_PIXEL_STACK]            = &WS2812FX::mode_pixel_stack;
    _mode[FX_MODE_MOVE_BAR_SIN]           = &WS2812FX::mode_move_bar_sin;
    _mode[FX_MODE_MOVE_BAR_QUAD]          = &WS2812FX::mode_move_bar_quad;
    _mode[FX_MODE_MOVE_BAR_CUBE]          = &WS2812FX::mode_move_bar_cubic;
    _mode[FX_MODE_MOVE_BAR_SAWTOOTH]      = &WS2812FX::mode_move_bar_sawtooth;
    _mode[FX_MODE_POPCORN]                = &WS2812FX::mode_popcorn;
    _mode[FX_MODE_FIREWORKROCKETS]        = &WS2812FX::mode_firework2;
    _mode[FX_MODE_RING_RING]              = &WS2812FX::mode_ring_ring;
    _mode[FX_MODE_HEARTBEAT]              = &WS2812FX::mode_heartbeat;
    _mode[FX_MODE_RAIN]                   = &WS2812FX::mode_rain;
    _mode[FX_MODE_VOID]                   = &WS2812FX::mode_void;
    _mode[FX_MODE_SUNRISE]                = &WS2812FX::mode_sunrise;
    _mode[FX_MODE_SUNSET]                 = &WS2812FX::mode_sunset;

    _name[FX_MODE_STATIC]                 = F("Static");
    _name[FX_MODE_EASE]                   = F("Ease");
    //_name[FX_MODE_TWINKLE_EASE]           = F("Ease Twinkle");
    _name[FX_MODE_BREATH]                 = F("Breath");
    _name[FX_MODE_NOISEMOVER]             = F("iNoise8");
    //_name[FX_MODE_TWINKLE_NOISEMOVER]     = F("Twinkle iNoise8 Mover");
    _name[FX_MODE_PLASMA]                 = F("Plasma");
    _name[FX_MODE_JUGGLE_PAL]             = F("Juggle Pixels");
    _name[FX_MODE_FILL_BEAT]              = F("Color Fill");
    _name[FX_MODE_DOT_BEAT]               = F("Dots");
    _name[FX_MODE_DOT_COL_WIPE]           = F("Dots Color Wipe");
    _name[FX_MODE_COLOR_WIPE_SAWTOOTH]    = F("Wipe Sawtooth");
    _name[FX_MODE_COLOR_WIPE_SINE]        = F("Wipe Sine");
    _name[FX_MODE_COLOR_WIPE_QUAD]        = F("Wipe Quad");
    _name[FX_MODE_COLOR_WIPE_TRIWAVE]     = F("Wipe Triwave");
    _name[FX_MODE_MULTI_DYNAMIC]          = F("Dynamic");
    _name[FX_MODE_RAINBOW]                = F("Rainbow");
    _name[FX_MODE_RAINBOW_CYCLE]          = F("Rainbow Cycle");
    _name[FX_MODE_PRIDE]                  = F("Pride");
    //_name[FX_MODE_PRIDE_GLITTER]          = F("Pride Glitter");
    _name[FX_MODE_SCAN]                   = F("Scan");
    _name[FX_MODE_DUAL_SCAN]              = F("Dual Scan");
    _name[FX_MODE_FADE]                   = F("Fade");
    _name[FX_MODE_THEATER_CHASE]          = F("Theater Chase");
    _name[FX_MODE_THEATER_CHASE_DUAL_P]   = F("Theater Chase Dual palette");
    _name[FX_MODE_THEATER_CHASE_RAINBOW]  = F("Theater Chase Rainbow");
    _name[FX_MODE_RUNNING_LIGHTS]         = F("Running Lights");
    _name[FX_MODE_TO_INNER]               = F("Centering");
    _name[FX_MODE_FILL_BRIGHT]            = F("Wave Bright");
    _name[FX_MODE_TWINKLE_FADE]           = F("Twinkle Fade");
    _name[FX_MODE_TWINKLE_FOX]            = F("Twinkle Fox");
    //      _name[FX_MODE_SOFTTWINKLES]               = F("Soft Twinkles"); // FIXME: Broken...
    _name[FX_MODE_FIREWORK]               = F("Firework");
    _name[FX_MODE_FIRE2012]               = F("Fire 2012");
    _name[FX_MODE_FILL_WAVE]              = F("FILL Wave");
    _name[FX_MODE_LARSON_SCANNER]         = F("Larson Scanner");
    _name[FX_MODE_COMET]                  = F("Comet");
    //_name[FX_MODE_FIRE_FLICKER]           = F("Fire Flicker");
    //_name[FX_MODE_FIRE_FLICKER_SOFT]      = F("Fire Flicker (soft)");
    _name[FX_MODE_FIRE_FLICKER_INTENSE]   = F("Fire Flicker");
    _name[FX_MODE_BUBBLE_SORT]            = F("Bubble Sort");
    _name[FX_MODE_SHOOTING_STAR]          = F("Shooting Star");
    _name[FX_MODE_BEATSIN_GLOW]           = F("Sine glows");
    _name[FX_MODE_PIXEL_STACK]            = F("Pixel Stack");
    _name[FX_MODE_MOVE_BAR_SIN]           = F("1/2 Bar sine");
    _name[FX_MODE_MOVE_BAR_QUAD]          = F("1/2 Bar²");
    _name[FX_MODE_MOVE_BAR_CUBE]          = F("1/2 Bar³");
    _name[FX_MODE_MOVE_BAR_SAWTOOTH]      = F("1/2 Bar");
    _name[FX_MODE_POPCORN]                = F("Popcorn");
    _name[FX_MODE_FIREWORKROCKETS]        = F("Firework Rocket");
    _name[FX_MODE_RING_RING]              = F("Phone Ring");
    _name[FX_MODE_HEARTBEAT]              = F("Heart Beat");
    _name[FX_MODE_RAIN]                   = F("Meteor Shower");
    _name[FX_MODE_VOID]                   = F("Void");
    _name[FX_MODE_SUNRISE]                = F("Sunrise");
    _name[FX_MODE_SUNSET]                 = F("Sunset");

    _pal_name[RAINBOW_PAL]            = F("Rainbow");
    _pal_name[LAVA_PAL]               = F("Lava");
    _pal_name[ICE_WATER_PAL]          = F("Iced Water");
    _pal_name[RAINBOWSTRIPES_PAL]     = F("RainbowStripe");
    _pal_name[FOREST_PAL]             = F("Forest");
    _pal_name[OCEAN_PAL]              = F("Ocean");
    _pal_name[HEAT_PAL]               = F("Heat");
    _pal_name[PARTY_PAL]              = F("Party");
    _pal_name[CLOUD_PAL]              = F("Cloud");
    _pal_name[ICE_PAL]                = F("Ice");
    _pal_name[RETROC9_PAL]            = F("Retro");
    _pal_name[SNOW_PAL]               = F("Snow");
    _pal_name[FAIRYLIGHT_PAL]         = F("Fairy Light");
    _pal_name[BLUEWHITE_PAL]          = F("Blue White");
    _pal_name[REDWHITHE_PAL]          = F("Red White");
    _pal_name[HOLLY_PAL]              = F("Holly");
    _pal_name[REDGREENWHITE_PAL]      = F("Red Green White");
    _pal_name[SHADES_OF_RED_PAL]      = F("Red Shades");
    _pal_name[SHADES_OF_GREEN_PAL]    = F("Green Shades");
    _pal_name[SHADES_OF_BLUE_PAL]     = F("Blue Shades");
    _pal_name[RANDOM_PAL]             = F("Random");

    _new_mode = 255;
    _volts = 5;

    FastLED.setBrightness(DEFAULT_BRIGHTNESS);

    resetDefaults();
  }

  ~WS2812FX()
  {
    delete leds;
    delete _bleds;
  }

  CRGB *leds;
  CRGB *_bleds;

  void
    init                  (void),
    resetDefaults         (void),
    service               (void),
    start                 (void),
    stop                  (void),
    show                  (void),
    setCurrentPalette     (CRGBPalette16 p, String Name),
    setCurrentPalette     (uint8_t n),
    setTargetPalette      (CRGBPalette16 p, String Name),
    setTargetPalette      (uint8_t n),
    map_pixels_palette    (uint8_t *hues, uint8_t bright, TBlendType blend),
    setMode               (uint8_t m),
    increaseSpeed         (uint8_t s),
    decreaseSpeed         (uint8_t s),
    setColor              (uint8_t r, uint8_t g, uint8_t b),
    setColor              (uint32_t c),
    setColor              (CRGBPalette16 c),
    setBrightness         (uint8_t b),
    increaseBrightness    (uint8_t s),
    decreaseBrightness    (uint8_t s),
    trigger               (void),
    setBlendType          (TBlendType t),
    setColorTemperature   (uint8_t index),
    toggleBlendType       (void);

  /*
   * _segment set functions
   */ 
  // setters
  inline void setCRC                  (uint16_t CRC)    { _segment.CRC = CRC; }
  inline void setIsRunning            (bool isRunning)  { _segment.isRunning = isRunning; if(isRunning) { _transition = true; _blend = 0; } }
  inline void setPower                (bool power)      { _segment.power = power; setTransition(); } // this should fix reopened issue #6
  inline void setReverse              (bool rev)        { _segment.reverse = rev; setTransition(); }
  inline void setInverse              (bool inv)        { _segment.inverse = inv; setTransition(); }
  inline void setMirror               (bool mirror)     { _segment.mirror = mirror; setTransition(); }
  inline void setAddGlitter           (bool addGlitter) { _segment.addGlitter = addGlitter; }
  inline void setWhiteGlitter         (bool whiteGlitter) { _segment.whiteGlitter = whiteGlitter; }
  inline void setOnBlackOnly          (bool onBlackOnly){ _segment.onBlackOnly = onBlackOnly; }
  #ifdef HAS_KNOB_CONTROL
  inline void setWiFiDisabled          (bool wifiDisabled){ _segment.wifiDisabled = wifiDisabled; }
  #endif
  inline void setAutoplay             (AUTOPLAYMODES m) { _segment.autoplay = m; }
  inline void setAutopal              (AUTOPLAYMODES p) { _segment.autoPal = p; }
  inline void setBeat88               (uint16_t b)      { _segment.beat88 = constrain(b, BEAT88_MIN, BEAT88_MAX); }
  inline void setSpeed                (uint16_t s)      { setBeat88(s); }
  inline void setHuetime              (uint16_t t)      { _segment.hueTime = t; SEGMENT_RUNTIME.nextHue = 0; }
  inline void setMilliamps            (uint16_t m)      { _segment.milliamps = constrain(m, 100, DEFAULT_CURRENT_MAX); FastLED.setMaxPowerInVoltsAndMilliamps(_volts, _segment.milliamps); }
  inline void setAutoplayDuration     (uint16_t t)      { _segment.autoplayDuration = t; SEGMENT_RUNTIME.nextAuto = 0; }
  inline void setAutopalDuration      (uint16_t t)      { _segment.autoPalDuration = t; SEGMENT_RUNTIME.nextPalette = 0; }
  inline void setSegments             (uint8_t s)       { _segment.segments = constrain(s, 1, max(MAX_NUM_SEGMENTS, 1)); }
  inline void setCooling              (uint8_t cool)    { _segment.cooling = constrain(cool, DEFAULT_COOLING_MIN, DEFAULT_COOLING_MAX); }
  inline void setSparking             (uint8_t spark)   { _segment.sparking = constrain(spark, DEFAULT_SPARKING_MIN, DEFAULT_SPARKING_MAX); }
  inline void setTwinkleSpeed         (uint8_t speed)   { _segment.twinkleSpeed = constrain(speed, DEFAULT_TWINKLE_S_MIN, DEFAULT_TWINKLE_S_MAX); }
  inline void setTwinkleDensity       (uint8_t density) { _segment.twinkleDensity = constrain(density, DEFAULT_TWINKLE_NUM_MIN, DEFAULT_TWINKLE_NUM_MAX); }
  inline void setNumBars              (uint8_t numBars) { _segment.numBars = constrain(numBars, 1, max((LED_COUNT / _segment.segments) / MAX_NUM_BARS_FACTOR, 1)); setTransition(); }
  // setMode --> treated separately...
  inline void setMaxFPS               (uint8_t fps)     { _segment.fps = constrain(fps, 10, STRIP_MAX_FPS); /*FastLED.setMaxRefreshRate(fps);*/ }
  inline void setDeltaHue             (uint8_t dh)      { _segment.deltaHue = dh; }
  inline void setBlur                 (uint8_t b)       { _segment.blur = b; _pblur = b; }
  inline void setDamping              (uint8_t d)       { _segment.damping = constrain(d, DEFAULT_DAMPING_MIN, DEFAULT_DAMPING_MAX); }
  inline void setDithering            (uint8_t dither)  { _segment.dithering = dither; FastLED.setDither(dither); }
  inline void setSunriseTime          (uint8_t t)       { _segment.sunrisetime = constrain(t, DEFAULT_SUNRISETIME_MIN, DEFAULT_SUNRISETIME_MAX); }
  inline void setTargetBrightness     (uint8_t b)       { setBrightness(b); }
  inline void setTargetPaletteNumber  (uint8_t p)       { setTargetPalette(p); }
  inline void setCurrentPaletteNumber (uint8_t p)       { setCurrentPalette(p); }
  inline void setColorTemp            (uint8_t c)       { setColorTemperature(c); }
  inline void setBckndSat             (uint8_t s)       { _segment.backgroundSat = s; }
  inline void setBckndHue             (uint8_t h)       { _segment.backgroundHue = h; }
  inline void setBckndBri             (uint8_t b)       { _segment.backgroundBri = constrain(b, BCKND_MIN_BRI, BCKND_MAX_BRI); }

  inline void setTransition           (void)            { _transition = true; _segment_runtime.modeinit = true; _blend = 0; }
  
  // getters
  inline size_t         getCRCsize(void)          { return sizeof(_segment.CRC); }
    
  inline uint16_t       getCRC(void)                  { return _segment.CRC; }
  inline bool           isRunning(void)               { return _segment.isRunning; }
  inline bool           getPower(void)                { return _segment.power; }
  inline bool           getReverse(void)              { return _segment.reverse; }
  inline bool           getInverse(void)              { return _segment.inverse; }
  inline bool           getMirror(void)               { return _segment.mirror; }
  inline bool           getAddGlitter(void)           { return _segment.addGlitter; }
  inline bool           getWhiteGlitter(void)         { return _segment.whiteGlitter; }
  inline bool           getOnBlackOnly(void)          { return _segment.onBlackOnly; }
  #ifdef HAS_KNOB_CONTROL
  inline bool           getWiFiDisabled(void)         { return _segment.wifiDisabled; }
  #endif
  inline AUTOPLAYMODES  getAutoplay(void)             { return _segment.autoplay; }
  inline AUTOPLAYMODES  getAutopal(void)              { return _segment.autoPal; }
  inline uint16_t       getSpeed(void)                { return getBeat88(); }
  inline uint16_t       getBeat88(void)               { return _segment.beat88; }
  inline uint16_t       getHueTime(void)              { return _segment.hueTime; }
  inline uint16_t       getMilliamps(void)            { return _segment.milliamps; }
  inline uint16_t       getAutoplayDuration(void)     { return _segment.autoplayDuration; }
  inline uint16_t       getAutopalDuration(void)      { return _segment.autoPalDuration; }
  inline uint8_t        getSegments(void)             { return _segment.segments; }
  inline uint8_t        getCooling(void)              { return _segment.cooling; }
  inline uint8_t        getSparking(void)             { return _segment.sparking; }
  inline uint8_t        getTwinkleSpeed(void)         { return _segment.twinkleSpeed; }  
  inline uint8_t        getTwinkleDensity(void)       { return _segment.twinkleDensity; }
  inline uint8_t        getNumBars(void)              { return _segment.numBars; }
  // getmode -> see below
  inline uint8_t        getMaxFPS(void)               { return _segment.fps; }
  inline uint8_t        getDeltaHue(void)             { return _segment.deltaHue; }
  inline uint8_t        getBlurValue(void)            { return _segment.blur; }
  inline uint8_t        getDamping(void)              { return _segment.damping; }
  inline uint8_t        getDithering(void)            { return _segment.dithering; }
  inline uint8_t        getSunriseTime(void)          { return _segment.sunrisetime; }
  inline uint8_t        getBrightness(void)           { return getTargetBrightness(); }
  inline uint8_t        getTargetBrightness(void)     { return _segment.targetBrightness; }
  inline uint8_t        getTargetPaletteNumber(void)  { return _segment.targetPaletteNum; }
  inline uint8_t        getCurrentPaletteNumber(void) { return _segment.currentPaletteNum; }
  inline uint8_t        getBckndSat(void)             { return _segment.backgroundSat; }
  inline uint8_t        getBckndHue(void)             { return _segment.backgroundHue; }
  inline uint8_t        getBckndBri(void)             { return _segment.backgroundBri; }

  inline TBlendType     getBlendType(void)            { return _segment.blendType; }
         uint8_t        getColorTemp(void);
  inline ColorTemperature getColorTemperature(void)   { return _segment.colorTemp; }
         uint32_t       getCurrentPower(void) ;
 
  // return a pointer to the complete segment structure
  inline WS2812FX::segment *getSegment(void) { return &_segment; }
  inline size_t getSegmentSize(void) { return sizeof(_segment); }

  inline uint16_t getCurrentSunriseStep(void) { return _segment_runtime.sunRiseStep; }
  inline uint16_t getFPS(void) { if(_service_Interval_microseconds > 0) { return ((1000000 / _service_Interval_microseconds) + 1) ; } else { return 65535; } }

  uint8_t
  getMode(void),
      getModeCount(void),
      getPalCount(void),
      nextMode(AUTOPLAYMODES mode),
      nextPalette(AUTOPLAYMODES mode),
      qadd8_lim(uint8_t i, uint8_t j, uint8_t lim);

  uint16_t old_segs;

  inline uint8_t getVoltage(void) {
    return _volts; }

  uint16_t
      getSunriseTimeToFinish(void),
      getStripLength(void),
      getLedsOn(void),
      getLength(void);

  static unsigned int calc_CRC16(unsigned int crc, unsigned char *buf, int len);


  uint32_t
  getColor(uint8_t p_index);

  const __FlashStringHelper *
  getModeName(uint8_t m);

  const __FlashStringHelper *
  getPalName(uint8_t p);

  const __FlashStringHelper * getColorTempName(uint8_t index);

  CRGBPalette16* getCurrentPalette(void) { return &_currentPalette; };
  CRGBPalette16* getTargetPalette (void) { return &_targetPalette; };

  //String* getCurrentPaletteName(void)    { return &_currentPaletteName; };
  //String* getTargetPaletteName (void)    { return &_targetPaletteName; };

  template <typename T> T map(T x, T x1, T x2, T y1, T y2);


private:
  void
      strip_off(void),
      fade_out(uint8_t fadeB),
      drawFractionalBar(int pos16, int width, const CRGBPalette16 &pal, uint8_t cindex, uint8_t max_bright, bool mixColor, uint8_t incindex),
      coolLikeIncandescent(CRGB &c, uint8_t phase),
      setPixelDirection(uint16_t i, bool dir, uint8 *directionFlags),
      brightenOrDarkenEachPixel(fract8 fadeUpAmount, fract8 fadeDownAmount, uint8_t *directionFlags),
      draw_sunrise_step(uint16_t step),
      m_sunrise_sunset(bool isSunrise),
      addSparks(const uint8_t probability, const bool onBlackOnly, const bool white);

  uint8_t attackDecayWave8(uint8_t i);

  CRGB calcSunriseColorValue(uint16_t step);

  uint16_t
      mode_ease(void),
      mode_twinkle_ease(void),
      mode_plasma(void),
      mode_fill_wave(void),
      mode_fill_bright(void),
      mode_to_inner(void),
      mode_dot_beat(void),
      mode_dot_beat_base(uint8_t fade),
      mode_dot_col_move(void),
      mode_col_wipe_sawtooth(void),
      mode_col_wipe_sine(void),
      mode_col_wipe_quad(void),
      mode_col_wipe_triwave(void),
      mode_col_wipe_func(uint8_t mode),
      mode_fill_beat(void),
      mode_confetti(void),
      mode_juggle_pal(void),
      mode_inoise8_mover(void),
      mode_firework(void),
      mode_bubble_sort(void),
      mode_static(void),
      color_wipe(uint32_t, uint32_t, bool),
      mode_multi_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      theater_chase(CRGB color1),
      theater_chase(bool dual),
      mode_theater_chase(void),
      mode_theater_chase_dual_pal(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      pride(void),
      mode_pride(void),
      mode_running_lights(void),
      mode_twinkle_fade(void),
      mode_sparkle(void),
      mode_larson_scanner(void),
      mode_comet(void),
      fireworks(uint32_t),
      mode_fire_flicker(void),
      mode_fire_flicker_soft(void),
      mode_fire_flicker_intense(void),
      fire_flicker(int),
      mode_fire2012WithPalette(void),
      mode_twinkle_fox(void),
      mode_softtwinkles(void),
      mode_shooting_star(void),
      mode_beatsin_glow(void),
      mode_pixel_stack(void),
      mode_move_bar_sin(void),
      mode_move_bar_cubic(void),
      mode_move_bar_quad(void),
      mode_move_bar_sawtooth(void),
      mode_move_bar(uint8_t mode),
      mode_popcorn(void),
      mode_firework2(void),
      mode_void(void),
      mode_sunrise(void),
      mode_sunset(void),
      mode_ring_ring(void),
      mode_heartbeat(void),
      mode_rain(void);
//      quadbeat(uint16_t in);

  CRGB
      computeOneTwinkle(uint32_t *ms, uint8_t *salt),
      makeBrighter(const CRGB &color, fract8 howMuchBrighter),
      makeDarker(const CRGB &color, fract8 howMuchDarker);

  bool
      getPixelDirection(uint16_t i, uint8 *directionFlags);

  inline uint16_t
      triwave16(uint16_t in),
      quadwave16(uint16_t in),
      cubicwave16(uint16_t in),
      ease16InOutQuad(uint16_t i),
      ease16OutQuad(uint16_t i),
      ease16InQuad(uint16_t i),
      ease16InOutCubic(uint16_t i);

  CRGBPalette16 _currentPalette;
  CRGBPalette16 _targetPalette;

  CRGBPalette16 getRandomPalette(void);

  //String _currentPaletteName;
  //String _targetPaletteName;

  const TProgmemRGBPalette16 *_palettes[NUM_PALETTES] =
      {
          &RainbowColors_p,
          &LavaColors_p,
          &Ice_Colors_p,
          &RainbowStripeColors_p,
          &ForestColors_p,
          &OceanColors_p,
          &HeatColors_p,
          &PartyColors_p,
          &CloudColors_p,
          &Ice_p,
          &RetroC9_p,
          &Snow_p,
          &FairyLight_p,
          &BlueWhite_p,
          &RedWhite_p,
          &Holly_p,
          &RedGreenWhite_p,
          &Shades_Of_Red_p,
          &Shades_Of_Green_p,
          &Shades_Of_Blue_p};

  const __FlashStringHelper *_pal_name[NUM_PALETTES];

  bool
      _transition,
      _triggered;

  uint8_t
      get_random_wheel_index(uint8_t, uint8_t),
      _new_mode,
      _volts,
      _blend,
      _pblur,
      _c_bck_h = 0,
      _c_bck_s = 0,
      _c_bck_b = 0,
      _brightness;

  uint16_t 
      _service_Interval_microseconds = 0;

  const __FlashStringHelper *
      _name[MODE_COUNT]; // SRAM footprint: 2 bytes per element

  mode_ptr
      _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

  segment _segment;

  segment_runtime _segment_runtime; // SRAM footprint: 14 bytes per element
};

#endif // WS2812FX_FastLED_h
