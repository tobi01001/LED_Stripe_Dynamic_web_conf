#include "network_manager.h"
#include "defaults.h"
#include <LittleFS.h>
#include <FastLED.h>
#include <DNSServer.h>
#include <EEPROM_Rotate.h>

#ifdef HAS_KNOB_CONTROL
  #include "SSD1306Brzo.h"
  extern SSD1306Brzo display;
#endif

// External dependencies from main file
extern void showInitColor(CRGB Color);
extern void writeLastResetReason(const String reason);
extern void deleteConfigFile(void);
extern String readLastResetReason(void);
extern void clearCRC(void);

// External EEPROM object
extern EEPROM_Rotate EEPROM;

// Network-related constants
extern const char * AP_SSID;

// LED strip external reference  
extern class LEDStripe * strip;

/// @brief setup WiFi connection with Wifi Manager...
/// @param timeout in seconds how long the WiFi captive portal should be active (default 240)
void setupWiFi(uint16_t timeout)
{
// setup the Wifi connection with Wifi Manager...

// when wifi is disabled via parameter, then there is nothing to do
// only works with "knob control"
#ifdef HAS_KNOB_CONTROL
  if (strip->getWiFiDisabled())
    return;
#endif

  showInitColor(CRGB::Blue);

  delay(INITDELAY);
  // set the hostname of the ESP8266
  WiFi.hostname(LED_NAME);
  // set the mode to station
  WiFi.mode(WIFI_STA);
  // set the sleep mode to none
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // set the persistent mode to true to not loose the last configuration
  WiFi.persistent(true);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&server, &dns);

// ToDo: Do not see the difference in handling....
// should simplyfy those Knob-Control #ifdefs
#ifndef HAS_KNOB_CONTROL
  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(timeout);

  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

  // tries to connect to last known settings
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration

  if (!wifiManager.autoConnect(AP_SSID))
  {
    showInitColor(CRGB::Yellow);
    delay(6 * INITDELAY);
    showInitColor(CRGB::Red);
    writeLastResetReason(F("WifiManager Timeout"));
    ESP.restart();
  }
  // reset the disconnect Counter
  wifi_disconnect_counter = 0;
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
  }

  WiFi.setAutoReconnect(true);
  // if we get here we have connected to the WiFi

#else // We have a control knob / button

  // If we are in button control mode
  // we only need WiFi for "convenience"
  wifiManager.setConfigPortalTimeout(timeout);

  // we reset after configuration to not have AP and STA in parallel...
  wifiManager.setBreakAfterConfig(true);

  // tries to connect to last known settings
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(AP_SSID))
  {
    WiFiConnected = false;
  }
  else
  {
    WiFiConnected = true;
    wifi_disconnect_counter = 0; // number of times we (re-)connected // = 0; // reset only in case we actually reconnected via setup routine
    if (WiFi.getMode() != WIFI_STA)
    {
      WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(true);
    // if we get here we have connected to the WiFi
  }
  wifi_err_counter = 0; // need to reset this regardless the connected state. Otherwise we try to reconntect every loop....

#endif
  gateway_ip = WiFi.gatewayIP();
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
}

void initOverTheAirUpdate(void)
{
  // we distinguish between KNOB_CONTROL and no knob control
  // hte difference is that we either show the progress 
  // - on the display or 
  // - on the leds itself
  #ifdef HAS_KNOB_CONTROL
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  // TODO: Implement Hostname in config and WIFI Settings?

  ArduinoOTA.setHostname(LED_NAME); // platformio currently works with IPs but who knows...

  // callback when the OTA starts
  ArduinoOTA.onStart([]() {
    FastLED.clear(true);
    display.clear();
    display.drawString(0, 0, F("Starting OTA..."));
    display.displayOn();
    display.display();
    // delete the configFile - gets recreated on request
    deleteConfigFile();
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();

    EEPROM.rotate(false);
    EEPROM.commit();

    // and we stop the websockets
    if(webSocketsServer)
    {
      webSocketsServer->textAll("OTA started!");
      webSocketsServer->closeAll();
      webSocketsServer->enable(false);
    }
    // we indicate our skEtch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });

  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    // OTA finished.
    display.drawString(0, 53, F("OTA finished!"));
    display.displayOn();
    display.display();

    // we delete the first uint16 in the EEPROM (which is the CRC)
    // as this will (hopefully) reset to defaults on SW updates
    if(RESET_DEFAULTS)
    {
      clearCRC();
    }

    // indicate that OTA is no longer running.
    OTAisRunning = false;
    delay(100);
    display.displayOff();
        // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(F("OTA success"));
    // and we stop (unmount) the Filesystem again
    LittleFS.end();
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress  on the display
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int prog = (progress / (total / 100));
    display.clear();
    display.drawString(0, 0, F("Starte OTA..."));
    display.drawStringMaxWidth(0, 12, 128, "Prog: " + String(prog) + " % done");
    display.drawProgressBar(1,33, 126, 7, prog);
    display.displayOn();
    display.display();
  });

  // something went wrong, we gonna show an error "message".
  ArduinoOTA.onError([](ota_error_t error) {
    String err = F("OTA Error: ");

    if (error == OTA_AUTH_ERROR) {
      err = err + F("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      err = err + F("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      err = err + F("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      err = err + F("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      err = err + F("End Failed");
    }

    display.clear();
    display.drawStringMaxWidth(0, 0,  128, F("Update failed!"));
    display.drawStringMaxWidth(0, 22, 128, err);
    display.drawStringMaxWidth(0, 43, 128, F("Reset in 5 Secs"));
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(err);
    // and we stop (unmount) the Filesystem again
    LittleFS.end();
    delay(5000);
    ESP.restart();
  });
  
  #else // HAS_KNOB_CONTROL
  showInitColor(CRGB::Blue);
  delay(INITDELAY);
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setRebootOnSuccess(true);

  ArduinoOTA.setHostname(LED_NAME);

  ArduinoOTA.onStart([]() {

    strip->setIsRunning(false);
    strip->setPower(false);
    // the following is just to visually indicate the OTA start
    // which is done by blinking the complete stripe in different colors from yellow to green
    uint8_t factor = 85;
    for (uint8_t c = 0; c < 4; c++)
    {
      uint8_t r = 256 - (c * factor);
      uint8_t g = c > 0 ? (c * factor - 1) : (c * factor);
      fill_solid(strip->leds, strip->getStripLength(), CRGB(r, g, 0));
      strip->show();
      delay(250);
      fill_solid(strip->leds, strip->getStripLength(), CRGB::Black);
      strip->show();
      delay(500);
    }
    // delete the configFile - gets recreated on request
    deleteConfigFile();
    // we stop the webserver to not get interrupted....
    server.end();
    // and we stop (unmount) the Filesystem
    LittleFS.end();

    EEPROM.rotate(false);
    EEPROM.commit();

    // and we stop the websockets
    if(webSocketsServer)
    {
      webSocketsServer->textAll(F("OTA started!"));
      webSocketsServer->closeAll();
      webSocketsServer->enable(false);
    }
    // we indicate our skEtch that OTA is currently running (should actually not be required)
    OTAisRunning = true;
  });
  // what to do if OTA is finished...
  ArduinoOTA.onEnd([]() {
    // OTA finished.
    // We fade out the green Leds being activated during OTA.
    bool ledsActive = true;
    
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(F("OTA success"));
    // and we stop (unmount) the Filesystem again
    LittleFS.end();

    while(ledsActive)
    {
      ledsActive = false;
      for(uint16_t i=0; i<strip->getStripLength(); i++)
      {
        if(strip->leds[i]) 
        {
          ledsActive = true;
          break;
        }
      }
      fadeToBlackBy(strip->leds, strip->getStripLength(), 8);
      strip->show();
      delay(2);
    }

    // we delete the first uint16 in the EEPROM (which is the CRC)
    // as this will (hopefully) reset to defaults on SW updates
    if(RESET_DEFAULTS)
    {
      clearCRC();
    }
    

    // indicate that OTA is no longer running. (rather useless)
    OTAisRunning = false;
    // no need to reset ESP as this is done by the OTA handler by default
  });
  // show the progress on the strips as well to be informed if anything gets stuck...
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // OTA Update will show increasing green LEDs during progress:
    uint16_t progress_value = progress * 100 / (total / strip->getStripLength());
    uint16_t pixel = (uint16_t)(progress_value / 100);
    uint16_t temp_color = progress_value - (pixel * 100);
    if (temp_color > 255)
      temp_color = 255;

    strip->leds[pixel] = CRGB(0, (uint8_t)temp_color, 0);
    strip->show();
  });

  // something went wrong, we gonna show an error "message" via LEDs.
  ArduinoOTA.onError([](ota_error_t error) {
    String err = F("OTA Error: ");

    if (error == OTA_AUTH_ERROR) {
      err = err + F("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      err = err + F("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      err = err + F("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      err = err + F("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      err = err + F("End Failed");
    }
    // something went wrong during OTA.
    // We will fade in to red...
    for (uint16_t c = 0; c < 256; c++)
    {
      for (uint16_t i = 0; i < strip->getStripLength(); i++)
      {
        strip->leds[i] = CRGB((uint8_t)c, 0, 0); 
      }
      strip->show();
      delay(2);
    }
    // need to mount again since we did unmount during OTA
    LittleFS.begin();
    writeLastResetReason(err);
    // and we stop (unmount) the Filesystem again
    LittleFS.end();

    // We wait 5 seconds and then restart the ESP...
    delay(5000);
    ESP.restart();
  });
  #endif // !HAS_KNOB_CONTROL

  // start the service
  ArduinoOTA.begin();
  showInitColor(CRGB::Green);
  delay(INITDELAY);
  showInitColor(CRGB::Black);
  delay(INITDELAY);
}

void handleOTA(void)
{
  ArduinoOTA.handle();
}

void handleMDNS(void)
{
  MDNS.update();
}