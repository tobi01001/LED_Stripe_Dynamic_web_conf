#ifndef H_00__MODES_H
#define H_00__MODES_H

#define DEF_FX_MODE_STATIC
#define DEF_FX_MODE_EASE
#define DEF_FX_MODE_NOISEMOVER
#define DEF_FX_MODE_PLASMA
#define DEF_FX_MODE_JUGGLE_PAL
#define DEF_FX_MODE_FILL_BEAT
#define DEF_FX_MODE_FILL_WAVE
#define DEF_FX_MODE_DOT_BEAT
#define DEF_FX_MODE_DOT_COL_WIPE
#define DEF_FX_MODE_COLOR_WIPE_SAWTOOTH
#define DEF_FX_MODE_COLOR_WIPE_SINE
#define DEF_FX_MODE_COLOR_WIPE_QUAD
#define DEF_FX_MODE_COLOR_WIPE_TRIWAVE
#define DEF_FX_MODE_TO_INNER
#define DEF_FX_MODE_BREATH
#define DEF_FX_MODE_MULTI_DYNAMIC
#define DEF_FX_MODE_RAINBOW
#define DEF_FX_MODE_RAINBOW_CYCLE
#define DEF_FX_MODE_PRIDE
#define DEF_FX_MODE_SCAN
#define DEF_FX_MODE_DUAL_SCAN
#define DEF_FX_MODE_FADE
#define DEF_FX_MODE_THEATER_CHASE
#define DEF_FX_MODE_THEATER_CHASE_DUAL_P
#define DEF_FX_MODE_THEATER_CHASE_RAINBOW
#define DEF_FX_MODE_RUNNING_LIGHTS
#define DEF_FX_MODE_TWINKLE_FADE
#define DEF_FX_MODE_TWINKLE_FOX
#define DEF_FX_MODE_FILL_BRIGHT
#define DEF_FX_MODE_FIREWORK
#define DEF_FX_MODE_FIRE2012
#define DEF_FX_MODE_LARSON_SCANNER
#define DEF_FX_MODE_COMET
#define DEF_FX_MODE_FIRE_FLICKER_INTENSE
#define DEF_FX_MODE_BUBBLE_SORT
#define DEF_FX_MODE_SHOOTING_STAR
#define DEF_FX_MODE_BEATSIN_GLOW
#define DEF_FX_MODE_PIXEL_STACK
#define DEF_FX_MODE_MOVE_BAR_SIN
#define DEF_FX_MODE_MOVE_BAR_QUAD
#define DEF_FX_MODE_MOVE_BAR_CUBE
#define DEF_FX_MODE_MOVE_BAR_SAWTOOTH
#define DEF_FX_MODE_POPCORN
//#define DEF_FX_MODE_FIREWORKROCKETS
#define DEF_FX_MODE_FIREWORKROCKETSIMPLE
#define DEF_FX_MODE_HEARTBEAT
#define DEF_FX_MODE_RAIN
#define DEF_FX_MODE_EASE_BAR
#define DEF_FX_MODE_PACIFICA
#define DEF_FX_MODE_COLOR_WAVES
#define DEF_FX_MODE_TWINKLE_MAP
#define DEF_FX_MODE_VOID
#define DEF_FX_MODE_RING_RING
#define DEF_FX_MODE_SUNRISE
#define DEF_FX_MODE_SUNSET



enum MODES
{
  #ifdef DEF_FX_MODE_STATIC
  FX_MODE_STATIC, // #include StaticEffect.h - the static effect
  #endif
  #ifdef DEF_FX_MODE_EASE
  FX_MODE_EASE, // #include EaseEffect.h - the ease effect
  #endif
  #ifdef DEF_FX_MODE_NOISEMOVER
  FX_MODE_NOISEMOVER, // #include NoiseMoverEffect.h - the noise mover effect
  #endif
  #ifdef DEF_FX_MODE_PLASMA
  FX_MODE_PLASMA, // #include PlasmaEffect.h - the plasma effect
  #endif
  #ifdef DEF_FX_MODE_JUGGLE_PAL
  FX_MODE_JUGGLE_PAL, // #include JugglePalEffect.h - the juggle palette effect
  #endif
  #ifdef DEF_FX_MODE_FILL_BEAT
  FX_MODE_FILL_BEAT, // #include FillBeatEffect.h - the fill beat effect
  #endif
  #ifdef DEF_FX_MODE_FILL_WAVE
  FX_MODE_FILL_WAVE, // #include FillWaveEffect.h - the fill wave effect
  #endif
  #ifdef DEF_FX_MODE_DOT_BEAT
  FX_MODE_DOT_BEAT, // #include DotBeatEffect.h - the dot beat effect
  #endif
  #ifdef DEF_FX_MODE_DOT_COL_WIPE
  FX_MODE_DOT_COL_WIPE, // #include DotColWipeEffect.h - the dot color wipe effect
  #endif
  #ifdef DEF_FX_MODE_COLOR_WIPE_SAWTOOTH
  FX_MODE_COLOR_WIPE_SAWTOOTH, // #include ColorWipeSawtoothEffect.h - the color wipe sawtooth effect
  #endif
  #ifdef DEF_FX_MODE_COLOR_WIPE_SINE
  FX_MODE_COLOR_WIPE_SINE, // #include ColorWipeSineEffect.h - the color wipe sine effect
  #endif
  #ifdef DEF_FX_MODE_COLOR_WIPE_QUAD
  FX_MODE_COLOR_WIPE_QUAD, // #include ColorWipeQuadEffect.h - the color wipe quad effect
  #endif
  #ifdef DEF_FX_MODE_COLOR_WIPE_TRIWAVE
  FX_MODE_COLOR_WIPE_TRIWAVE, // #include ColorWipeTriwaveEffect.h - the color wipe triwave effect
  #endif
  #ifdef DEF_FX_MODE_TO_INNER
  FX_MODE_TO_INNER, // #include ToInnerEffect.h - the to inner effect
  #endif
  #ifdef DEF_FX_MODE_BREATH
  FX_MODE_BREATH, // #include BreathEffect.h - the breath effect
  #endif
  #ifdef DEF_FX_MODE_MULTI_DYNAMIC
  FX_MODE_MULTI_DYNAMIC, // #include MultiDynamicEffect.h - the multi dynamic effect
  #endif
  
  #ifdef DEF_FX_MODE_RAINBOW
  FX_MODE_RAINBOW, // #include RainbowEffect.h - the rainbow effect
  #endif
  #ifdef DEF_FX_MODE_RAINBOW_CYCLE
  FX_MODE_RAINBOW_CYCLE, // #include RainbowCycleEffect.h - the rainbow cycle effect
  #endif
  #ifdef DEF_FX_MODE_PRIDE
  FX_MODE_PRIDE, // #include PrideEffect.h - the pride effect
  #endif
  #ifdef DEF_FX_MODE_SCAN
  FX_MODE_SCAN, // #include ScanEffect.h - the scan effect
  #endif
  #ifdef DEF_FX_MODE_DUAL_SCAN
  FX_MODE_DUAL_SCAN, // #include DualScanEffect.h - the dual scan effect
  #endif
  #ifdef DEF_FX_MODE_FADE
  FX_MODE_FADE, // #include FadeEffect.h - the fade effect
  #endif
  #ifdef DEF_FX_MODE_THEATER_CHASE
  FX_MODE_THEATER_CHASE, // #include TheaterChaseEffect.h - the theater chase effect
  #endif
  #ifdef DEF_FX_MODE_THEATER_CHASE_DUAL_P
  FX_MODE_THEATER_CHASE_DUAL_P, // #include TheaterChaseDualPEffect.h - the theater chase dual palette effect
  #endif
  #ifdef DEF_FX_MODE_THEATER_CHASE_RAINBOW
  FX_MODE_THEATER_CHASE_RAINBOW, // #include TheaterChaseRainbowEffect.h - the theater chase rainbow effect
  #endif
  #ifdef DEF_FX_MODE_RUNNING_LIGHTS
  FX_MODE_RUNNING_LIGHTS, // #include RunningLightsEffect.h - the running lights effect
  #endif
  #ifdef DEF_FX_MODE_TWINKLE_FADE
  FX_MODE_TWINKLE_FADE, // #include TwinkleFadeEffect.h - the twinkle fade effect
  #endif
  #ifdef DEF_FX_MODE_TWINKLE_FOX
  FX_MODE_TWINKLE_FOX, // #include TwinkleFoxEffect.h - the twinkle fox effect
  #endif
  #ifdef DEF_FX_MODE_FILL_BRIGHT
  FX_MODE_FILL_BRIGHT, // #include FillBrightEffect.h - the fill bright effect
  #endif
  #ifdef DEF_FX_MODE_FIREWORK
  FX_MODE_FIREWORK, // #include FireworkEffect.h - the firework effect
  #endif
  #ifdef DEF_FX_MODE_FIRE2012
  FX_MODE_FIRE2012, // #include Fire2012Effect.h - the fire 2012 effect
  #endif
  #ifdef DEF_FX_MODE_LARSON_SCANNER
  FX_MODE_LARSON_SCANNER, // #include LarsonScannerEffect.h - the larson scanner effect
  #endif
  #ifdef DEF_FX_MODE_COMET
  FX_MODE_COMET, // #include CometEffect.h - the comet effect
  #endif
  #ifdef DEF_FX_MODE_FIRE_FLICKER_INTENSE
  FX_MODE_FIRE_FLICKER_INTENSE, // #include FireFlickerIntenseEffect.h - the fire flicker intense effect
  #endif
  #ifdef DEF_FX_MODE_BUBBLE_SORT
  FX_MODE_BUBBLE_SORT, // #include BubbleSortEffect.h - the bubble sort effect
  #endif
  #ifdef DEF_FX_MODE_SHOOTING_STAR
  FX_MODE_SHOOTING_STAR, // #include ShootingStarEffect.h - the shooting star effect
  #endif
  #ifdef DEF_FX_MODE_BEATSIN_GLOW
  FX_MODE_BEATSIN_GLOW, // #include BeatSinGlowEffect.h - the beat sine glow effect
  #endif
  #ifdef DEF_FX_MODE_PIXEL_STACK
  FX_MODE_PIXEL_STACK, // #include PixelStackEffect.h - the pixel stack effect
  #endif
  #ifdef DEF_FX_MODE_MOVE_BAR_SIN
  FX_MODE_MOVE_BAR_SIN, // #include MoveBarSinEffect.h - the move bar sine effect
  #endif
  #ifdef DEF_FX_MODE_MOVE_BAR_QUAD
  FX_MODE_MOVE_BAR_QUAD, // #include MoveBarQuadEffect.h - the move bar quad effect
  #endif
  #ifdef DEF_FX_MODE_MOVE_BAR_CUBE
  FX_MODE_MOVE_BAR_CUBE, // #include MoveBarCubeEffect.h - the move bar cube effect
  #endif
  #ifdef DEF_FX_MODE_MOVE_BAR_SAWTOOTH
  FX_MODE_MOVE_BAR_SAWTOOTH, // #include MoveBarSawtoothEffect.h - the move bar sawtooth effect
  #endif
  #ifdef DEF_FX_MODE_POPCORN
  FX_MODE_POPCORN, // #include PopcornEffect.h - the popcorn effect
  #endif
  #ifdef DEF_FX_MODE_FIREWORKROCKETS
  FX_MODE_FIREWORKROCKETS, // #include FireworkRocketsEffect.h - the firework rockets effect
  #endif
  #ifdef DEF_FX_MODE_FIREWORKROCKETSIMPLE
  FX_MODE_FIREWORKROCKETSIMPLE, // #include FireworkRocketEffectSimple.h - the simple firework rocket effect
  #endif
  #ifdef DEF_FX_MODE_HEARTBEAT
  FX_MODE_HEARTBEAT, // #include HeartBeatEffect.h - the heartbeat effect
  #endif
  #ifdef DEF_FX_MODE_RAIN
  FX_MODE_RAIN, // #include RainEffect.h - the rain effect
  #endif
  #ifdef DEF_FX_MODE_EASE_BAR
  FX_MODE_EASE_BAR, // #include EaseBarEffect.h - the ease bar effect
  #endif
  #ifdef DEF_FX_MODE_PACIFICA
  FX_MODE_PACIFICA, // #include PacificaEffect.h - the pacifica effect
  #endif
  #ifdef DEF_FX_MODE_COLOR_WAVES
  FX_MODE_COLOR_WAVES, // #include ColorWavesEffect.h - the color waves effect
  #endif
  #ifdef DEF_FX_MODE_TWINKLE_MAP
  FX_MODE_TWINKLE_MAP, // #include TwinkleMapEffect.h - the twinkle map effect
  #endif
  #ifdef DEF_FX_MODE_VOID
  FX_MODE_VOID, // #include VoidEffect.h - the void effect
  #endif
  #ifdef DEF_FX_MODE_RING_RING
  FX_MODE_RING_RING, // #include PhoneRingEffect.h - the phone ring effect
  #endif
  #ifdef DEF_FX_MODE_SUNRISE
  FX_MODE_SUNRISE, // #include SunriseEffect.h - the sunrise effect
  #endif
  #ifdef DEF_FX_MODE_SUNSET
  FX_MODE_SUNSET, // #include SunsetEffect.h - the sunset effect
  #endif
  
  // has to be the final entry!
  MODE_COUNT
};


#endif // 00__modes.h