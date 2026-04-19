# Copilot Instructions — LED_Stripe_Dynamic_web_conf

## Project Purpose and Scope

This firmware drives WS2812/NeoPixel LED strips connected to an ESP8266 (target: Wemos D1 Mini / NodeMCU v2) using the **FastLED** library. It exposes:
- A **web UI** (served from LittleFS) for full browser-based control
- An **HTTP GET API** (`/set?<param>=<value>&…`) and a **WebSocket** broadcast channel for home-automation integration (FHEM, openHAB, etc.)
- Optional **local control** via a rotary encoder + SSD1306 OLED display (`HAS_KNOB_CONTROL` build flag)
- OTA (over-the-air) firmware and filesystem upload

Future roadmap items include **MQTT** and **Matter** integration. When those features are eventually added, they must be purely opt-in (guarded by build flags) and require only additive changes (new handler files/classes), without restructuring existing logic.

---

## Repository Layout

```
/src
  httpledstripe_esp.cpp     # Main Arduino entry point (setup / loop)
  LED_strip/
    led_strip.h / .cpp      # Strip lifecycle, HTTP/WS API, field descriptors
  WS2812FX/
    WS2812FX_FastLed.h/.cpp # Core strip driver, segment data, palette management
    Effect.h / .cpp          # Abstract Effect base class + EffectFactory
    EffectHelper.h / .cpp    # Shared math / color helpers used across effects
    effects/                 # One .h/.cpp pair per effect class
/include
  defaults.h                # Compile-time constants and guarded defaults
  debug_help.h              # Debug macros (Serial output only when DEBUG defined)
/lib                        # Vendored / patched third-party libraries (FastLED, FileEditor, RotaryEncoder)
/data                       # LittleFS web-frontend assets (built by favicon_script.py)
/env_data_folders           # Per-environment frontend assets + generic templates
platformio.ini              # All build environments and flags
```

---

## Build System (PlatformIO)

- Framework: **Arduino** on `espressif8266@2.6.3`
- Each deployment target is a named `[env:…]` section in `platformio.ini`
- Key build flags that **must** be set per environment:
  - `-DLED_NAME=\"<name>\"` — unique device name (used in mDNS, web UI title)
  - `-DLED_COUNT=<n>` or `-DLED_COUNT_TOT=<n> -DLED_OFFSET=<n>` — strip size
  - `-DHAS_KNOB_CONTROL` — enables rotary encoder + OLED (optional)
  - `-DDEFAULT_POWER`, `-DDEFAULT_MODE`, `-DDEFAULT_PALETTE` — boot defaults (optional overrides)
  - `-DDEFAULT_PS_MAX_CURRENT=<mA>` — power supply current limit
- Global flags always applied: `-Os` (size optimisation), `-DNO_GLOBAL_EEPROM`, FastLED DMA defines
- Use `!python git_rev_macro.py` in `build_flags` to inject Git revision into firmware
- The `favicon_script.py` `extra_scripts` entry copies and customises frontend assets before LittleFS image creation

---

## Coding Standards

Follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) unless explicitly overridden below.

### Naming
- **camelCase** for all identifiers (variables, functions, parameters)
- Private member variables prefixed with `_` (e.g. `_isInitialized`, `_segment`)
- Constants and macros in `UPPER_SNAKE_CASE`
- Effect class names use `PascalCase` with the `Effect` suffix (e.g. `Fire2012Effect`)

### Documentation
- Every public class, method, and non-trivial constant must have a Doxygen `/** … */` comment block
- Use `@brief`, `@param`, `@return` tags consistently
- Inline comments explain *why*, not *what*
- Store effect-specific algorithm notes in the class header, not the `.cpp` — headers are the first file a contributor opens when exploring a class, so putting the algorithm description there means the intent is visible without navigating to the implementation

### File Organisation
- One effect per `.h`/`.cpp` pair inside `src/WS2812FX/effects/`
- Register each effect with `REGISTER_EFFECT(FX_MODE_<NAME>, <ClassName>)` at the bottom of its `.cpp`
- Include guards use `#ifndef <FILENAME_H>` / `#define <FILENAME_H>` / `#endif // <FILENAME_H>`
- Never include project headers from inside a library file in `/lib`

---

## Performance — Embedded Constraints

This firmware targets ESP8266 with ~80 KB usable heap. Treat every byte as expensive.

### Memory
- **Never use `String` (Arduino class)** — use `const char*`, `__FlashStringHelper*` (`F(…)`), `PSTR`, or fixed-size `char[]` buffers
- **Never use `std::string`** in embedded code paths
- Store string literals and lookup tables in Flash with `PROGMEM` / `F()` / `PSTR()`
- Avoid `new` / `delete` in the main loop and effect `update()` methods; prefer stack allocation or statically sized members
- When dynamic allocation is unavoidable (e.g. `Fire2012Effect` heat array), allocate once in `init()`, free in `cleanup()`, and guard with null checks
- Use `uint8_t`, `uint16_t`, `uint32_t` (from `<stdint.h>`) — never `int` where a smaller type is sufficient
- Avoid `double`; prefer fixed-point integer arithmetic where possible — use `float` only as a fallback when fixed-point would measurably hurt performance or visual quality

### CPU
- `update()` must return as fast as possible — avoid blocking waits (`delay()`) inside effects
- Use FastLED's `beat88()`, `sin8()`, `cos8()`, `lerp8by8()`, `scale8()`, `qadd8()`, `qsub8()` — they operate on `uint8_t` using integer math optimised for AVR/Xtensa
- Prefer bit-shifts over division/multiplication by powers of two
- Avoid repeated `millis()` calls inside a single `update()` — capture once to a local variable
- Do not call `FastLED.show()` directly from an effect; the main service loop controls when the frame is committed
- Respect `FRAME_CALC_WAIT_MICROINTERVAL` and `MIN_LED_WRITE_CYCLE` timing constants in `defaults.h`

### FastLED Specifics
- ESP8266 DMA mode is enabled globally (`FASTLED_ESP8266_DMA`, `FASTLED_ESP8266_RAW_PIN_ORDER`); LED data pin must be GPIO3 (raw pin 3 = RX)
- Use `FASTLED_USE_PROGMEM 1` — palette data lives in Flash automatically
- Apply `FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLTAGE, STRIP_MILLIAMPS)` for current limiting; this is already wired to the `milliamps` segment field
- Prefer `CRGBPalette16` over `CRGBPalette256` to save RAM; interpolate with `ColorFromPalette(…, index, brightness, blendType)`
- Use `nblendPaletteTowardPalette()` for smooth live palette transitions — do not hard-switch palettes mid-animation
- `fill_solid()`, `fill_rainbow()`, `fill_gradient_RGB()` are faster than pixel-by-pixel loops
- Effect colour output goes into the `CRGB* leds` array provided by `WS2812FX`; never cache a separate shadow copy unless the effect algorithm requires it
- When blending/dimming use `fadeToBlackBy()` / `nscale8()` — do not multiply by a float

### Timing model
- The strip runs at a configurable FPS (`SEG.fps`, bounded by `STRIP_MIN_FPS` … `STRIP_MAX_FPS`)
- `update()` returns a delay in milliseconds; return `0` only when the next frame must be rendered immediately
- Use `SEG_RT.next_time` (segment runtime) and the `beat88`-based timebase for tempo-synchronised effects; do not create independent time references unless necessary

---

## Connectivity

### Current stack
| Protocol | Library | Notes |
|---|---|---|
| WiFi STA | ESP8266WiFi (Arduino core) | Managed by ESPAsyncWiFiManager; captive-portal on first boot |
| HTTP server | ESPAsyncWebServer | Non-blocking; all handlers run in async callbacks |
| WebSocket | ESPAsyncWebServer built-in | Broadcasts state changes to all connected clients |
| OTA | ArduinoOTA | Firmware + LittleFS image upload |
| mDNS | ESP8266mDNS | Device reachable as `<LED_NAME>.local` |
| EEPROM | EEPROM_Rotate | Wear-levelled settings persistence with CRC |

### Async programming rules
- **Never block** inside an `AsyncWebServer` request handler, WebSocket event, or OTA callback
- Do not call `delay()`, `yield()`, or any blocking wait inside a handler
- Keep handler bodies short; defer heavy work by setting a flag and processing it in `loop()`
- JSON responses must use `AsyncJsonResponse` (from `AsyncJson.h`) — do not build JSON into a `String`
- The WebSocket broadcast (`ws.textAll(…)`) is safe to call from `loop()` context; avoid calling it from ISR context

### API contract (HTTP GET)
- Endpoint: `/set?<field>=<value>[&<field>=<value>…]`
- Field names and value ranges are defined by the `Field` array in `led_strip.cpp`
- Any change applied via `/set` **must** also be broadcast over WebSocket so all clients stay in sync
- Status query: `/status` returns the full JSON field-value map

### Future: MQTT *(not yet implemented — opt-in via `HAS_MQTT`)*
> These are design guidelines for when MQTT support is added. No MQTT code exists yet; do **not** add any unless the feature is explicitly activated.

- Planned topics follow `<LED_NAME>/set/<field>` (subscribe) and `<LED_NAME>/state` (publish on change)
- Use an async MQTT client (e.g. `AsyncMqttClient`) — no synchronous blocking calls
- Decouple the MQTT handler from `led_strip.cpp` by using the existing `setFieldValue()` / `getAllValuesJSON()` API
- Add a `#ifdef HAS_MQTT` compile-time guard so the feature is opt-in
- TLS support should be optional (`#ifdef MQTT_TLS`) to keep RAM usage manageable on ESP8266

### Future: Matter / Thread *(not yet implemented — opt-in via `HAS_MATTER`)*
> Matter support targets ESP32-S3 or newer hardware and does not exist yet. Do **not** add any Matter code to the current ESP8266 codebase.

- Protect all Matter code with `#ifdef ESP32` and a `HAS_MATTER` flag
- Expose the strip state as a **ColorTemperatureLight** or **ExtendedColorLight** cluster
- Re-use `setFieldValue()` / `getFieldValue()` as the single source-of-truth; Matter callbacks must not bypass this layer
- Keep the Matter stack isolated in its own source file (e.g. `src/matter_bridge.cpp`)

---

## Future: ESP32 Portability *(not yet implemented — forward-looking guidelines only)*

> The current firmware targets **ESP8266 exclusively**. ESP32 support does not exist yet. Do **not** add ESP32-specific code until it is explicitly planned. The notes below are design guidelines to keep the codebase ready for a future port.

- All ESP8266-specific includes (`ESP8266WiFi.h`, `ESP8266mDNS.h`, `EEPROM_Rotate.h`, `ESPAsyncTCP.h`) are currently unconditional; guard them with `#ifdef ESP8266` when adding ESP32 support
- Replace with the ESP32 equivalents (`WiFi.h`, `ESPmDNS.h`, `AsyncTCP.h`, `Preferences.h`) inside `#elif defined(ESP32)` blocks
- FastLED DMA defines (`FASTLED_ESP8266_DMA`, `FASTLED_ESP8266_RAW_PIN_ORDER`) are ESP8266-only; wrap them
- On ESP32 the LED data pin is not restricted to GPIO3; expose it via a build flag (`-DLED_PIN=<n>`)
- Use `IRAM_ATTR` on any function called from an interrupt on both platforms

---

## Effect System

### Adding a new effect
1. Create `src/WS2812FX/effects/<EffectName>Effect.h` and `<EffectName>Effect.cpp`
2. Inherit from `Effect`; implement `init()`, `update()`, `getName()`, `getModeId()`, and optionally `cleanup()`
3. Add the new mode ID to the `MODES` enum in `WS2812FX_FastLed.h` **before** `MODE_COUNT`
4. `#include` the new header in `WS2812FX_FastLED.cpp` and call `REGISTER_EFFECT(FX_MODE_<NAME>, <ClassName>)` at the bottom of the `.cpp`
5. Return the display name via `getName()` using `F("…")` (Flash string — saves RAM)

### Effect `update()` contract
- Must be pure (no side-effects on global state other than the `leds` array and effect-local `_segment_runtime` fields)
- Must return a non-zero delay in milliseconds unless frame-rate limiting is handled by the caller
- Must not allocate heap memory
- Access strip parameters through the `WS2812FX*` pointer using `SEG.*` and `SEG_RT.*` macros
- Use `SEG_RT_MV.*` for per-effect mode variables stored in the shared runtime union

### Effect `init()` contract
- Called once when the effect becomes active (or when `_isInitialized == false`)
- Call `standardInit(strip)` as the first statement unless the effect has special reset requirements
- Allocate any private heap buffers here and record size; free them in `cleanup()`
- Always return `true` on success, `false` on allocation failure

---

## Debugging

- Wrap all Serial output in `#ifdef DEBUG … #endif` using the macros in `debug_help.h`
- `INITDELAY` is automatically longer in DEBUG builds to allow Serial monitor to connect
- Use `FASTLED_INTERNAL` define to suppress FastLED's own compile-time warnings in non-debug builds (already done in `httpledstripe_esp.cpp`)
- The `monitor_filters = esp8266_exception_decoder` PlatformIO option decodes stack traces automatically

---

## Security and Robustness

- Validate all incoming HTTP/WebSocket field values against `min`/`max` bounds in the `Field` descriptor before applying them
- Guard all EEPROM reads with CRC verification (`RESET_DEFAULTS` mechanism in `defaults.h`)
- Watchdog: do not starve the ESP8266 watchdog — `yield()` or `ESP.wdtFeed()` if a computation takes more than ~100 ms
- Never expose raw filesystem write endpoints without authentication (when authentication is added)
- Sanitise any user-supplied string that is reflected back in HTTP responses to prevent injection

