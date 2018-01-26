/**************************************************************
   This work is based on many others.
   If published then under whatever licence free to be used,
   distibuted, changed and whatsoever.
   Work is based on:
   WS2812BFX library by  - see:
   fhem esp8266 implementation by   - see:
   WiFiManager library by - - see:
   ... many others ( see includes)

 **************************************************************/
#include <FS.h>

#define DEBUG

#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Bounce2.h>
#include <ArduinoOTA.h>


// new approach starts here:
#include <led_strip.h>

#include <pahcolor.h>



/* Flash Button can be used here for toggles.... */
bool hasResetButton = false;
Bounce debouncer = Bounce();




/* Definitions for network usage */
/* maybe move all wifi stuff to separate files.... */
#define WIFI_TIMEOUT 5000
ESP8266WebServer server(80);
WiFiManager wifiManager;

String AP_SSID = "LED_stripe_" + String(ESP.getChipId());

char chrResetButtonPin[3]="X";
char chrLEDCount[5] = "0";
char chrLEDPin[2] = "0";

//default custom static IP
char static_ip[16] = "";
char static_gw[16] = "";
char static_sn[16] = "255.255.255.0";

WiFiManagerParameter ResetButtonPin("RstPin", "Reset Pin", chrResetButtonPin, 3);
WiFiManagerParameter LedCountConf("LEDCount","LED Count", chrLEDCount, 4);
WiFiManagerParameter LedPinConf("LEDPIN", "Strip Data Pin", chrLEDPin, 3);

extern const char index_html[];
extern const char main_js[];

String modes = "";

//flag for saving data
bool shouldSaveConfig = false;

/* END Network Definitions */

unsigned long last_wifi_check_time = 0;

// function Definitions
void  saveConfigCallback(void),
      readConfigurationFS(void),
      initOverTheAirUpdate(void),
      setupResetButton(uint8_t buttonPin),
      updateConfiguration(void),
      setupWiFi(void),
      handleRoot(void),
      srv_handle_main_js(void),
      modes_setup(void),
      srv_handle_modes(void),
      handleSet(void),
      handleNotFound(void),
      handleGetModes(void),
      handleStatus(void),
      factoryReset(void),
      handleResetRequest(void),
      setupWebServer(void);

//callback notifying us of the need to save config
void saveConfigCallback(void) {
  #ifdef DEBUG
    Serial.println("\n\tWe are now invited to save the configuration...");
  #endif // DEBUG
  shouldSaveConfig = true;
}

void readConfigurationFS(void) {
  //read configuration from FS json

  /*  check if we really need to use the filw system or if a somple EEPROM
      write/read would do the trick as well
  */

  #ifdef DEBUG
    Serial.println("mounting FS...");
  #endif
  if (SPIFFS.begin()) {
    #ifdef DEBUG
    Serial.println("mounted file system");
    #endif
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      #ifdef DEBUG
      Serial.println("reading config file");
      #endif
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        #ifdef DEBUG
        Serial.println("opened config file");
        #endif
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          #ifdef DEBUG
          Serial.println("\nparsed json");
          #endif
          strcpy(chrResetButtonPin, json["chrResetButtonPin"]);
          strcpy(chrLEDCount, json["chrLEDCount"]);
          strcpy(chrLEDPin, json["chrLEDPin"]);
          // This checks if a IP is contained in the file
          // currently not used as no IP will be written
          if(json["ip"]) {
            #ifdef DEBUG
            Serial.println("setting custom ip from config");
            #endif
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            #ifdef DEBUG
            Serial.println(static_ip);
            #endif
          } else {
            #ifdef DEBUG
            Serial.println("no custom ip in config");
            #endif
          }
        } else {
          #ifdef DEBUG
          Serial.println("failed to load json config");
          #endif
        }
      }
    }
  } else {
    #ifdef DEBUG
    Serial.println("failed to mount FS");
    #endif
  }
  //end read
  #ifdef DEBUG
  Serial.print("Static IP: \t");
  Serial.println(static_ip);
  Serial.print("LED Count: \t");
  Serial.println(chrLEDCount);
  Serial.print("LED Pin: \t");
  Serial.println(chrLEDPin);
  Serial.print("Rst Btn Pin: \t");
  Serial.println(chrResetButtonPin);
  #endif
}

void initOverTheAirUpdate(void) {
  #ifdef DEBUG
  Serial.println("\nInitializing OTA capabilities....");
  #endif
  /* init OTA */
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // ToDo: Implement Hostname in config and WIFI Settings?

  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname("esp8266Toby01");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    #ifdef DEBUG
    Serial.println("OTA start");
    #endif
    setEffect(FX_NO_FX);
    reset();
    for(uint8_t c = 0; c<5; c++){
      for(uint16_t i=0; i<strip.getLength(); i++) {
        uint8_t r = 256 - (c*64);
        uint8_t g = c > 0 ? (c*64-1) : (c*64);
        strip.setPixelColor(i, r, g, 0);
      }
      strip.show();
      delay(400);
      for(uint16_t i=0; i<strip.getLength(); i++) {
        strip.setPixelColor(i, 0x000000);
      }
      strip.show();
      delay(400);
    }
  });
  ArduinoOTA.onEnd([]() {
    #ifdef DEBUG
    Serial.println("\nOTA end");
    #endif
    // OTA finished.
    // Green Leds fade out.
    for(uint8_t i = Green(strip.getPixelColor(0)); i>0; i--)
    {
      for(uint16_t p=0; p<strip.getLength(); p++)
      {
        strip.setPixelColor(p, 0, i-1 ,0);
      }
      strip.show();
      delay(2);
    }
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    #ifdef DEBUG
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    #endif
    // OTA Update will show increasing green LEDs during progress:
    uint16_t pixel = (uint16_t)(progress / (total / strip.getLength()));
    strip.setPixelColor(pixel, 0x00ff00);
    strip.show();
    delay(1);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    #ifdef DEBUG
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    #endif
    // something went wrong during OTA.
    // We will fade in to red...
    for(uint8_t c = 0; c<256; c++)
    {
      for(uint16_t i = 0; i<strip.getLength(); i++)
      {
        strip.setPixelColor(i,c,0,0);
      }
      strip.show();
      delay(2);
    }
  });
  ArduinoOTA.begin();
  #ifdef DEBUG
  Serial.println("OTA capabilities initialized....");
  #endif
  delay(500);
}

void setupResetButton(uint8_t buttonPin){
    pinMode(buttonPin,INPUT_PULLUP);
    // After setting up the button, setup the Bounce instance :
    debouncer.attach(buttonPin);
    debouncer.interval(50); // interval in ms
}

void updateConfiguration(void){
  #ifdef DEBUG
  Serial.println("Updating configuration just received");
  #endif
  // only copy the values in case the Parameter wa sset in config!
  if(shouldSaveConfig) {
    strcpy(chrResetButtonPin, ResetButtonPin.getValue());
    strcpy(chrLEDCount, LedCountConf.getValue());
    strcpy(chrLEDPin, LedPinConf.getValue());
  }
  /*
  String sLedCount = chrLEDCount;
  String sLedPin   = chrLEDPin;

  uint16_t ledCount = sLedCount.toInt();
  uint8_t ledPin = sLedPin.toInt();
  */
  uint16_t ledCount = (uint16_t) strtoul(chrLEDCount, NULL, 10);
  uint8_t ledPin = (uint8_t) strtoul(chrLEDPin, NULL, 10);
  // if something went wrong here (GPIO = 0 or LEDs = 0)
  // we reset and start allover again
  if(ledCount == 0 || ledPin == 0) {
    #ifdef DEBUG
    Serial.println("\n\tSomething went wrong! Config will be deleted and ESP Reset!");
    #endif
    SPIFFS.format();
    wifiManager.resetSettings();
    #ifdef DEBUG
    Serial.println("\nCountdown to Reset:");
    #endif
    for(uint8_t i = 0; i<10; i++) {
      #ifdef DEBUG
      Serial.println(10-i);
      #endif
    }
    ESP.reset();
  }
  #ifdef DEBUG
  Serial.print("LEDCount: ");
  Serial.println(ledCount);

  Serial.print("LED datapin: ");
  Serial.println(ledPin);
  #endif
  if(chrResetButtonPin[0] == 'x' || chrResetButtonPin[0] == 'X') {
      hasResetButton = false;
      #ifdef DEBUG
      Serial.println("No Reset Button specified.");
      #endif
  }
  else {
      String srstPin = chrResetButtonPin;
      uint8_t rstPin =  srstPin.toInt();
      #ifdef DEBUG
      Serial.print("Reset Button Pin: ");
      Serial.println(rstPin);
      #endif
      hasResetButton = true;
      setupResetButton(rstPin);
  }

  if (shouldSaveConfig) {
    #ifdef DEBUG
    Serial.println("saving config");
    #endif
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["chrResetButtonPin"] = chrResetButtonPin;
    json["chrLEDCount"] = chrLEDCount;
    json["chrLEDPin"] = chrLEDPin;

    /*  Maybe we deal with static IP later.
        For now we just don't save it....
        json["ip"] = WiFi.localIP().toString();
        json["gateway"] = WiFi.gatewayIP().toString();
        json["subnet"] = WiFi.subnetMask().toString();
    */
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      #ifdef DEBUG
      Serial.println("failed to open config file for writing");
      #endif
    }
    #ifdef DEBUG
    json.prettyPrintTo(Serial);
    #endif
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  #ifdef DEBUG
  Serial.println("\nEverything in place... setting up stripe.");
  #endif
  stripe_setup(ledCount, ledPin, DEFAULT_PIXEL_TYPE);
}

void setupWiFi(void){

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&ResetButtonPin);
  wifiManager.addParameter(&LedCountConf);
  wifiManager.addParameter(&LedPinConf);

  // 4 Minutes should be sufficient.
  // Especially in case of WiFi loss...
  wifiManager.setConfigPortalTimeout(240);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  #ifdef DEBUG
  Serial.println("Going to autoconnect and/or Start AP");
  #endif
  if (!wifiManager.autoConnect(AP_SSID.c_str())) {
    #ifdef DEBUG
    Serial.println("failed to connect, we should reset as see if it connects");
    #endif
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  #ifdef DEBUG
  Serial.println("Print LED config again to be sure: ");
  Serial.print("LED Count: \t");
  Serial.println(chrLEDCount);
  Serial.print("LED Pin: \t");
  Serial.println(chrLEDPin);
  Serial.print("Rst Btn Pin: \t");
  Serial.println(chrResetButtonPin);
  //if you get here you have connected to the WiFi
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());
  #endif
}

void handleRoot(void){
    server.send_P(200,"text/html", index_html);
    #ifdef DEBUG
    Serial.println("\t/ called from Webserver...\n");
    #endif
}

void srv_handle_main_js(void) {
  server.send_P(200,"application/javascript", main_js);
}

void modes_setup(void) {
  modes = "";
  uint8_t num_modes = strip.getModeCount();
  for(uint8_t i=0; i < num_modes; i++) {
    uint8_t m = i;
    modes += "<li><a href='#' class='mo' id='";
    modes += m;
    modes += "'>";
    modes += strip.getModeName(m);
    modes += "</a></li>";
  }
}

void srv_handle_modes(void) {
  server.send(200,"text/plain", modes);
}


uint8_t changebypercentage (uint8_t value, uint8_t percentage) {
  uint16_t ret = max((value*percentage)/100, 10);
  if (ret > 255) ret = 255;
  return (uint8_t) ret;
}

// if /set was called
void handleSet(void){

  // Debug only
  #ifdef DEBUG
  Serial.println("<Begin>Server Args:");
  for(uint8_t i = 0; i<server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print("\t");
    Serial.println(server.arg(i));
    Serial.print(server.argName(i));
    Serial.print("\t char[0]: ");
    Serial.println(server.arg(i)[0]);
  }
  Serial.println("<End> Server Args");
  #endif
  // to be completed in general
  // question: do we include the "effects in the library?"
  // question: is there enough memory to store color and "timing" per pixel?
  // i.e. uint32_t onColor, OffColor, uint16_t ontime, offtime
  // = 12 * 300 = 3600 byte...???
  // this will be a new branch possibly....

  // mo = mode set (eihter +, - or value)
  // br = brightness (eihter +, - or value)
  // co = color (32 bit unsigned color) (eihter +, - or value)
  // re = red value of color (eihter +, - or value)
  // gr = green value of color (eihter +, - or value)
  // bl = blue value of color (eihter +, - or value)
  // sp = speed (eihter +, - or value)
  // ti = time in seconds....
  // pi = pixel to be set (clears others?)
  // rnS = Range start Pixel;
  // rnE = Range end Pixel;

  // here we set a new mode if we have the argument mode
  if(server.hasArg("mo")) {
    bool isWS2812FX = false;
    uint8_t effect = strip.getMode();

    // just switch to the next
    if (server.arg("mo")[0] == 'u') {
      effect = effect + 1;
      isWS2812FX = true;
    }
    // switch to the previous one
    else if (server.arg("mo")[0] == 'd') {
      effect = effect - 1;
      isWS2812FX = true;
    }
    else if (server.arg("mo")[0] == 'o') {
      strip_On_Off(false);
      strip.clear();
      strip.stop();
    }
    else if (server.arg("mo")[0] == 'f') {
      effect = FX_MODE_FIRE_FLICKER;
      isWS2812FX = true;
    }
    else if (server.arg("mo")[0] == 'r') {
      effect = FX_MODE_RAINBOW_CYCLE;
      isWS2812FX = true;
    }
    else if (server.arg("mo")[0] == 'k') {
      effect = FX_MODE_LARSON_SCANNER;
      isWS2812FX = true;
    }
    else if (server.arg("mo")[0] == 's') {
      effect = FX_MODE_TWINKLE_FADE_RANDOM;
      isWS2812FX = true;
    }
    else if (server.arg("mo")[0] == 'w') {
      strip.setColor(0xffffff);
      effect = FX_MODE_TWINKLE_FADE;
      isWS2812FX = true;
    }
    // sunrise effect
    // + delta value
    // ToDo Implement

    else if (server.arg("mo") == "Sunrise") {
      // milliseconds time to full sunrise
      uint32_t mytime = 0;
      const uint16_t mysteps = 512; // defaults to 512;
      // sunrise time in seconds
      if(server.hasArg("sec")) {
        mytime = 1000 * (uint32_t)strtoul(&server.arg("sec")[0], NULL, 10);
      }
      // sunrise time in minutes
      else if(server.hasArg("min")) {
        mytime = (1000 * 60) * (uint8_t)strtoul(&server.arg("min")[0], NULL, 10);
      }
      // use default if time = 0;
      // ToDo: Maybe use "stored", i.e. last value?
      if(mytime == 0) {
        // default will be 10 minutes
        // = (1000 ms * 60) = 1 minute *10 = 10 minutes
        mytime = 1000 * 60 * 10; // for readability
      }
      mySunriseStart(mytime, mysteps, true);
      setEffect(FX_SUNRISE);
    }
    // sunrise effect
    // + delta value
    // ToDo Implement
    else if (server.arg("mo") == "Sunset") {
      // milliseconds time to full sunrise
      uint32_t mytime = 0;
      const uint16_t mysteps = 512; // defaults to 1000;
      // sunrise time in seconds
      if(server.hasArg("sec")) {
        mytime = 1000 * (uint32_t)strtoul(&server.arg("sec")[0], NULL, 10);
      }
      // sunrise time in minutes
      else if(server.hasArg("min")) {
        mytime = (1000 * 60) * (uint8_t)strtoul(&server.arg("min")[0], NULL, 10);
      }
      // use default if time = 0;
      // ToDo: Maybe use "stored", i.e. last value?
      if(mytime == 0) {
        // default will be 10 minutes
        // = (1000 ms * 60) = 1 minute *10 = 10 minutes
        mytime = 1000 * 60 * 10; // for readability
      }
      mySunriseStart(mytime, mysteps, false);
      setEffect(FX_SUNSET);
    }
    // finally switch to the one being provided.
    // we don't care if its actually an int or not
    // because it wil be zero anyway if not.
    else {
      effect = (uint8_t)strtoul(&server.arg("mo")[0], NULL, 10);
      isWS2812FX = true;
    }
    // make sure we roll over at the max number
    if(effect >= strip.getModeCount()) {
      effect = 0;
    }
    // activate the effect and trigger it once...
    if(isWS2812FX) {
      setEffect(FX_WS2812);
      strip.setMode(effect);
      strip.start();
      strip.trigger();
    }
  }
  // if we got a new brightness value
  if(server.hasArg("br")) {
    uint8_t brightness = strip.getBrightness();
    if (server.arg("br")[0] == 'u') {
    brightness = changebypercentage(brightness, 110);
    } else if (server.arg("br")[0] == 'd') {
      brightness = changebypercentage(brightness, 90);
    } else {
      brightness = constrain((uint8_t)strtoul(&server.arg("br")[0], NULL, 10), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    }
    strip.setBrightness(brightness);
    strip.show();
  }
  // if we got a speed value
  if(server.hasArg("sp")) {
    uint16_t speed = strip.getSpeed();
    if (server.arg("sp")[0] == 'u') {
      uint16_t ret = max((speed*110)/100, 10);
      if (ret > SPEED_MAX) ret = SPEED_MAX;
      speed = ret;
      //speed = changebypercentage(speed, 110);
    } else if (server.arg("sp")[0] == 'd') {
      uint16_t ret = max((speed*90)/100, 10);
      if (ret > SPEED_MAX) ret = SPEED_MAX;
      speed = ret;
      //speed = changebypercentage(speed, 90);
    } else {
      speed = constrain((uint16_t)strtoul(&server.arg("sp")[0], NULL, 10), SPEED_MIN, SPEED_MAX);
    }
    strip.setSpeed(speed);
    // delay_interval = (uint8_t)(speed / 256); // obsolete???
    strip.show();
  }
  // color handling
  uint32_t color = strip.getColor();
  if(server.hasArg("re")) {
    uint8_t re = Red(color);
    if(server.arg("re")[0] == 'u') {
        re = changebypercentage(re, 110);
    } else if (server.arg("re")[0] == 'd') {
      re = changebypercentage(re, 90);
    } else {
      re = constrain((uint8_t)strtoul(&server.arg("re")[0], NULL, 10), 0, 255);
    }
    color = (color & 0x00ffff) | (re << 16);
  }
  if(server.hasArg("gr")) {
    uint8_t gr = Green(color);
    if(server.arg("gr")[0] == 'u') {
        gr = changebypercentage(gr, 110);
    } else if (server.arg("gr")[0] == 'd') {
      gr = changebypercentage(gr, 90);
    } else {
      gr = constrain((uint8_t)strtoul(&server.arg("gr")[0], NULL, 10), 0, 255);
    }
    color = (color & 0xff00ff) | (gr << 8);
  }
  if(server.hasArg("bl")) {
    uint8_t bl = Blue(color);
    if(server.arg("bl")[0] == 'u') {
        bl = changebypercentage(bl, 110);
    } else if (server.arg("bl")[0] == 'd') {
      bl = changebypercentage(bl, 90);
    } else {
      bl = constrain((uint8_t)strtoul(&server.arg("bl")[0], NULL, 10), 0, 255);
    }
    color = (color & 0xffff00) | (bl << 0);
  }
  if(server.hasArg("co")) {
    color = constrain((uint32_t)strtoul(&server.arg("co")[0], NULL, 16), 0, 0xffffff);
  }
  if(server.hasArg("pi")) {
    //setEffect(FX_NO_FX);
    uint16_t pixel = constrain((uint16_t)strtoul(&server.arg("pi")[0], NULL, 10), 0, strip.getLength()-1);
    strip_setpixelcolor(pixel, color);
/*
    strip.setPixelColor(pixel, color);
    strip.show();
*/
  } else if (server.hasArg("rnS") && server.hasArg("rnE")) {
    uint16_t start = constrain((uint16_t)strtoul(&server.arg("rnS")[0], NULL, 10), 0, strip.getLength());
    uint16_t end = constrain((uint16_t)strtoul(&server.arg("rnE")[0], NULL, 10), start, strip.getLength());
    set_Range(start, end, color);
/*
    if(start > end && end > 0) {
      start = end-1;
    }
    setEffect(FX_NO_FX);
    for(uint16_t i = start; i<(end+1)%strip.getLength(); i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
*/
  } else if (server.hasArg("rgb")) {
    strip.setColor(color);
    setEffect(FX_WS2812);
    strip.setMode(FX_MODE_STATIC);
  } else {
    strip.setColor(color);
  }
  handleStatus();
}

// if something unknown was called...
void handleNotFound(void){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleGetModes(void){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(56) + 1070;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();

  JsonObject& modeinfo = root.createNestedObject("modeinfo");
  modeinfo["count"] = strip.getModeCount();

  JsonObject& modeinfo_modes = modeinfo.createNestedObject("modes");
  for(uint8_t i=0; i<strip.getModeCount(); i++) {
      modeinfo_modes[strip.getModeName(i)] = i;
  }

  #ifdef DEBUG
  root.printTo(Serial);
  #endif

  String message = "";
  root.prettyPrintTo(message);
  server.send(200, "application/json", message);
}

void handleStatus(void){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(10) + 180;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  String message = "";
  uint16_t num_leds_on = 0;
  // if brightness = 0, no LED can be lid.
  if(strip.getBrightness()) {
    // count the number of active LEDs
    // in rare occassions, this can still be 0, depending on the effect.
    for(uint16_t i=0; i<strip.getLength(); i++) {
      if(strip.getPixelColor(i)) num_leds_on++;
    }
  }

  JsonObject& root = jsonBuffer.createObject();

  JsonObject& currentState = root.createNestedObject("currentState");

  currentState["state"] = stripIsOn;
  currentState["LedsOn"] = num_leds_on;
  currentState["mode"] = currentEffect;
  switch (currentEffect) {
    case FX_NO_FX :
      currentState["modename"] = F("No FX");
      break;
    case FX_SUNRISE :
      currentState["modename"] = F("Sunrise Effect");
      break;
    case FX_SUNSET :
      currentState["modename"] = F("Sunset Effect");
      break;
    case FX_WS2812 :
      currentState["modename"] = (String)"WS2812fx " + (String)strip.getModeName(strip.getMode());
      break;
    default :
      currentState["modename"] = F("UNKNOWN");
      break;
  }
  currentState["wsfxmode"] = strip.getMode();
  currentState["speed"] = strip.getSpeed();
  currentState["brightness"] = strip.getBrightness();
  currentState["red"] = Red(strip.getColor());
  currentState["green"] = Green(strip.getColor());
  currentState["blue"] = Blue(strip.getColor());

  #ifdef DEBUG
  JsonObject& ESP_Data = root.createNestedObject("ESP_Data");
  ESP_Data["Debug code"] = "On";
  ESP_Data["CPU_Freq"] = ESP.getCpuFreqMHz();
  ESP_Data["Flash Real Size"] = ESP.getFlashChipRealSize();
  ESP_Data["Free RAM"] = ESP.getFreeHeap();
  ESP_Data["Free Sketch Space"] = ESP.getFreeSketchSpace();
  ESP_Data["Sketch Size"] = ESP.getSketchSize();
  ESP_Data["Vcc"] = ESP.getVcc();
  root.prettyPrintTo(Serial);
  #endif

  root.prettyPrintTo(message);
  server.send(200, "application/json", message);
}

void factoryReset(void){
  #ifdef DEBUG
  Serial.println("Someone requested Factory Reset");
  #endif
  // on factory reset, each led will be red
  // increasing from led 0 to max.
  for(uint16_t i = 0; i<strip.getLength(); i++) {
    strip.setPixelColor(i, 0xa00000);
    strip.show();
    delay(2);
  }
  strip.show();
  // formatting File system
  #ifdef DEBUG
  Serial.println("Format File System");
  #endif
  delay(1000);
  SPIFFS.format();
  delay(1000);
  #ifdef DEBUG
  Serial.println("Reset WiFi Settings");
  #endif
  wifiManager.resetSettings();
  delay(3000);
  //reset and try again
  #ifdef DEBUG
  Serial.println("Reset ESP and start all over...");
  #endif
  ESP.reset();
}

// Received Factoryreset request.
// To be sure we check the related parameter....
void handleResetRequest(void){
  if(server.arg("rst") == "FactoryReset")
  {
    factoryReset();
  }
}

void setupWebServer(void){
  modes.reserve(5000);
  modes_setup();

  server.on("/main.js", srv_handle_main_js);
  server.on("/modes", srv_handle_modes);
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/getmodes", handleGetModes);
  server.on("/status", handleStatus);
  server.on("/reset", handleResetRequest);
  server.onNotFound(handleNotFound);

  server.begin();
  #ifdef DEBUG
  Serial.println("HTTP server started.\n");
  #endif
}

// setup network and output pins
void setup() {
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("\n\n\n");
  Serial.println(F("Booting"));
  #endif
  readConfigurationFS();

  setupWiFi();

  setupWebServer();

  updateConfiguration();

  initOverTheAirUpdate();

  // if we got that far, we show by a nice little animation
  // as setup finished signal....
  for(uint8_t num=0; num<4; num++)
  {
    for(uint16_t i=0; i<strip.getLength(); i+=1)
    {
      if(i%2)
        strip.setPixelColor(i,0x00a000);
      else
        strip.setPixelColor(i,0xa00000);
    }
    strip.show();
    delay(250);
    for(uint16_t i=0; i<strip.getLength(); i+=1)
    {
      if(i%2)
        strip.setPixelColor(i,0xa00000);
      else
        strip.setPixelColor(i,0x00a000);
    }
    strip.show();
    delay(150);
  }
  strip.stop();
}

// request receive loop
void loop(){
  unsigned long now = millis();
  #ifdef DEBUG
  static uint8_t life_sign = 0;
  static unsigned long last_status_msg = 0;
  #endif
  // if someone requests a call to the factory reset...
  static bool ResetRequested = false;

  #ifdef DEBUG
    // Debug Watchdog. to be removed for "production".
  if(now - last_status_msg > 20000) {
    last_status_msg = now;
    Serial.print("\ncurrentEffect\t");
    Serial.println(currentEffect);
  }
  #endif
  // Checking WiFi state every WIFI_TIMEOUT
  // Reset on disconnection
  if(now - last_wifi_check_time > WIFI_TIMEOUT) {
    //Serial.print("\nChecking WiFi... ");
    if(WiFi.status() != WL_CONNECTED) {
      #ifdef DEBUG
      Serial.println("WiFi connection lost. Reconnecting...");
      Serial.println("Lost Wifi Connection....");
      #endif
      // Show the WiFi loss with yellow LEDs.
      // Whole strip lid finally.
      for(uint16_t i = 0; i<strip.getLength(); i++)
      {
        strip.setPixelColor(i, 0xa0a000);
        strip.show();
      }
      // Reset after 6 seconds....
      delay(3000);
      #ifdef DEBUG
      Serial.println("Resetting ESP....");
      #endif
      delay(3000);
      ESP.reset();
    }
    last_wifi_check_time = now;
  }

  ArduinoOTA.handle(); // check and handle OTA updates of the code....

  //Button Handling
  // ToDo: Move to function?
  if(hasResetButton)  {
    debouncer.update();
    if(debouncer.read() == LOW)
    {
      factoryReset();
    }
  }
  server.handleClient();
  effectHandler();
  /*
  if (client)
  {
    while(client.connected())
    {
      if(client.available())
      {
        inputLine = client.readStringUntil('\n');



        // POST PIXEL DATA
        if (inputLine.length() > 3 && inputLine.substring(0,10) == F("POST /leds")) {
          isPost = true;
          //Serial.println("Received POST Data...");
        }

        if (inputLine.length() > 3 && inputLine.substring(0,16) == F("Content-Length: ")) {
          //postDataLength = inputLine.substring(16).toInt();
          //Serial.printf("\t\tGot Postdata with Length %u\n", postDataLength);
          client.readStringUntil('\n');
          inputLine = client.readStringUntil('\n');
          //Serial.print("Inputline with content:\n\t-->Start of Line\n\t\t");
          //Serial.println(inputLine);
          //Serial.println("\t--> End of Line");
          uint8_t r,g,b = 0;
          uint16_t pixel = 0;
          for(uint16_t i=0; i<inputLine.length()-1; i+=3)
          {
            r = colorVal(inputLine[i]);
            g = colorVal(inputLine[i+1]);
            b = colorVal(inputLine[i+2]);
            if(pixel < strip.getLength()) strip.setPixelColor(pixel++,r,g,b);
          }
          strip.show();
          isGet = true;
        }

        // let the given range blink
        // rework for encapsulation and such....
        if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /blink/")) {
          int slash = inputLine.indexOf('/', 11 );
          int komma1 = inputLine.indexOf(',');
          fx_blinker_start_pixel = inputLine.substring(11, komma1).toInt();
          fx_blinker_end_pixel = inputLine.substring(komma1+1, slash).toInt();
          int urlend = inputLine.indexOf(' ', 11 );
          String getParam = inputLine.substring(slash+1,urlend+1);
          komma1 = getParam.indexOf(',');
          int komma2 = getParam.indexOf(',',komma1+1);
          int komma3 = getParam.indexOf(',',komma2+1);
          int komma4 = getParam.indexOf(',',komma3+1);
          fx_blinker_red = getParam.substring(0,komma1).toInt();
          fx_blinker_green = getParam.substring(komma1+1, komma2).toInt();
          fx_blinker_blue = getParam.substring(komma2+1, komma3).toInt();

          fx_blinker_time_on = getParam.substring(komma3+1, komma4).toInt();
          fx_blinker_time_off = getParam.substring(komma4+1).toInt();

          setEffect(FX_BLINKER);

          isGet = true;
        }

  */

}
