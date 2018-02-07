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

//#define DEBUG

#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Bounce2.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>


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

/* END Network Definitions */

extern const char index_html[];
extern const char main_js[];

String modes = "";

//flag for saving data
bool shouldSaveConfig = false;
bool shouldSaveRuntime = false;

#define DATAVALID_KEY 0x55aa5a5a

typedef struct {
    uint32_t dataValid = 0x00000000;
    WS2812FX::segment seg;
    uint8_t brightness = DEFAULT_BRIGHTNESS;
    mysunriseParam sParam;
    pah_colorvalues sunriseColors;
    uint8_t currentEffect = FX_NO_FX;
    bool stripIsOn = false;
} EEPROMSaveData;

EEPROMSaveData myEEPROMSaveData;

unsigned long last_wifi_check_time = 0;

// function Definitions
void  saveConfigCallback(void),
      saveEEPROMData(void),
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
      setupWebServer(void),
      clearEEPROM(void);

// write runtime data to EEPROM
void saveEEPROMData(void) {
  if(!shouldSaveRuntime) return;
  shouldSaveRuntime = false;
  #ifdef DEBUG
    Serial.println("\nGoing to store runtime on EEPROM...");
  #endif
  myEEPROMSaveData.dataValid = DATAVALID_KEY;
  myEEPROMSaveData.seg = strip.getSegments()[0];
  myEEPROMSaveData.brightness = strip.getBrightness();
  myEEPROMSaveData.sParam = sunriseParam;
  myEEPROMSaveData.sunriseColors = myColor.getPahColorValues();
  myEEPROMSaveData.currentEffect = currentEffect;
  myEEPROMSaveData.stripIsOn = stripIsOn;

  #ifdef DEBUG
  Serial.print("\tdataValid\t");
  Serial.println(myEEPROMSaveData.dataValid);

  Serial.print("\tBrightness\t");
  Serial.println(myEEPROMSaveData.brightness);

  Serial.print("\tCurrentEffect\t");
  Serial.println(myEEPROMSaveData.currentEffect);

  Serial.print("\twsfxmode\t");
  Serial.println(myEEPROMSaveData.seg.mode);

  Serial.print("\tstripIsOn\t");
  Serial.println(myEEPROMSaveData.stripIsOn);
  #endif

  EEPROM.begin(sizeof(myEEPROMSaveData)+10);
  EEPROM.put(0, myEEPROMSaveData);
  EEPROM.commit();
  EEPROM.end();
  #ifdef DEBUG
    Serial.println("EEPROM write finished...");
  #endif
}

//callback notifying us of the need to save config
void saveConfigCallback(void) {
  #ifdef DEBUG
    Serial.println("\n\tWe are now invited to save the configuration...");
  #endif // DEBUG
  shouldSaveConfig = true;
}


// reads the stored runtime data from EEPROM
// must be called after everything else is already setup to be working
// otherwise this may terribly fail
void readRuntimeDataEEPROM(void) {
  #ifdef DEBUG
    Serial.println("\n\tReading Config From EEPROM...");
  #endif
  //read the configuration from EEPROM into RAM
  EEPROM.begin(sizeof(myEEPROMSaveData)+10);

  EEPROM.get(0, myEEPROMSaveData);
  EEPROM.end();

  if(myEEPROMSaveData.dataValid == DATAVALID_KEY) {
    strip.setSegment(0, myEEPROMSaveData.seg.start, myEEPROMSaveData.seg.stop,
                        myEEPROMSaveData.seg.mode, myEEPROMSaveData.seg.colors,
                        myEEPROMSaveData.seg.speed, myEEPROMSaveData.seg.reverse);
    strip.setBrightness(myEEPROMSaveData.brightness);
    sunriseParam = myEEPROMSaveData.sParam;
    myColor.setPahColorValues(myEEPROMSaveData.sunriseColors);
    currentEffect = myEEPROMSaveData.currentEffect;
    stripIsOn = myEEPROMSaveData.stripIsOn;
  }
  #ifdef DEBUG
  Serial.print("\tdataValid\n");
  Serial.println(myEEPROMSaveData.dataValid);

  Serial.print("\tBrightness\n");
  Serial.println(myEEPROMSaveData.brightness);
  Serial.println(strip.getBrightness());

  Serial.print("\tCurrentEffect\n");
  Serial.println(myEEPROMSaveData.currentEffect);
  Serial.println(currentEffect);

  Serial.print("\twsfxmode\n");
  Serial.println(myEEPROMSaveData.seg.mode);
  Serial.println(strip.getMode());

  Serial.print("\tstripIsOn\n");
  Serial.println(myEEPROMSaveData.stripIsOn);
  Serial.println(stripIsOn);
  #endif

  // no need to save right now. next save should be after /set?....
  shouldSaveRuntime = false;
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
    uint8_t factor = 85;
    for(uint8_t c = 0; c < 4; c++) {

      for(uint16_t i=0; i<strip.getLength(); i++) {
        uint8_t r = 256 - (c*factor);
        uint8_t g = c > 0 ? (c*factor-1) : (c*factor);
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
    server.stop();
  });
  ArduinoOTA.onEnd([]() {
    #ifdef DEBUG
    Serial.println("\nOTA end");
    #endif
    clearEEPROM();
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
    uint8_t color = 0;

    uint16_t progress_value = progress*100 / (total / strip.getLength());
    uint16_t pixel = (uint16_t) (progress_value / 100);
    uint16_t temp_color = progress_value - (pixel*100);
    if(temp_color > 255) temp_color = 255;

    //uint16_t pixel = (uint16_t)(progress / (total / strip.getLength()));
    strip.setPixelColor(pixel, 0, (uint8_t)temp_color, 0);
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
    for(uint16_t c = 0; c<256; c++)
    {
      for(uint16_t i = 0; i<strip.getLength(); i++)
      {
        strip.setPixelColor(i,(uint8_t)c,0,0);
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
  } else {
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
    modes += "<a href='#' class='mo' id='";
    modes += m;
    modes += "'>";
    modes += strip.getModeName(m);
    modes += "</a>";
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
  // question: is there enough memory to store color and "timing" per pixel?
  // i.e. uint32_t onColor, OffColor, uint16_t ontime, offtime
  // = 12 * 300 = 3600 byte...???
  // this will be a new branch possibly....

  // mo = mode set (eihter +, - or value)
  // br = brightness (eihter +, - or value)
  // co = color (32 bit unsigned color)
  // re = red value of color (eihter +, - or value)
  // gr = green value of color (eihter +, - or value)
  // bl = blue value of color (eihter +, - or value)
  // sp = speed (eihter +, - or value)
  // sec = sunrise / sunset time in seconds....
  // min = sunrise/ sunset time in minutes
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
      reset();
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
    // because it will be zero anyway if not.
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
  } else if (server.hasArg("rnS") && server.hasArg("rnE")) {
    uint16_t start = constrain((uint16_t)strtoul(&server.arg("rnS")[0], NULL, 10), 0, strip.getLength());
    uint16_t end = constrain((uint16_t)strtoul(&server.arg("rnE")[0], NULL, 10), start, strip.getLength());
    set_Range(start, end, color);
  } else if (server.hasArg("rgb")) {
    strip.setColor(color);
    setEffect(FX_WS2812);
    strip.setMode(FX_MODE_STATIC);
  } else {
    strip.setColor(color);
  }
  handleStatus();
  shouldSaveRuntime = true;
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

  if(stripIsOn) {
    currentState["state"] = "on";
  } else {
    currentState["state"] = "off";
  }
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
  currentState["color_red"] = Red(strip.getColor());
  currentState["color_green"] = Green(strip.getColor());
  currentState["color_blue"] = Blue(strip.getColor());


  JsonObject& sunRiseState = root.createNestedObject("sunRiseState");

  if(sunriseParam.isSunrise) {
    sunRiseState["sunRiseMode"] = F("Sunrise");
  } else {
    sunRiseState["sunRiseMode"] = F("Sunset");
  }
  if(sunriseParam.isRunning) {
    sunRiseState["sunRiseActive"] = F("on");
    sunRiseState["sunRiseCurrStep"] = sunriseParam.step;
    sunRiseState["sunRiseTotalSteps"] = sunriseParam.steps;
    if(sunriseParam.isSunrise) {
      sunRiseState["sunRiseTimeToFinish"] =
        ((sunriseParam.steps - sunriseParam.step) * sunriseParam.deltaTime)/1000;
    } else {
      sunRiseState["sunRiseTimeToFinish"] =
        ((sunriseParam.step) * sunriseParam.deltaTime)/1000;
    }

    currentState["rgb"] = myColor.calcColorValue(sunriseParam.step);

  } else {
    sunRiseState["sunRiseActive"] = F("off");
    sunRiseState["sunRiseTimeToFinish"] = 0;
    sunRiseState["sunRiseCurrStep"] = 0;
    sunRiseState["sunRiseTotalSteps"] = sunriseParam.steps;
    currentState["rgb"] = strip.getColor();
  }
  sunRiseState["sunRiseMinStep"] = myColor.getStepStart();
  sunRiseState["sunRiseMidStep"] = myColor.getStepMid();
  sunRiseState["sunRiseEndStep"] = myColor.getStepEnd();
  sunRiseState["sunRiseStartColor"] = myColor.getColorStart();
  sunRiseState["sunRiseMid1Color"] = myColor.getColorMid1();
  sunRiseState["sunRiseMid2Color"] = myColor.getColorMid2();
  sunRiseState["sunRiseMid3Color"] = myColor.getColorMid3();
  sunRiseState["sunRiseEndColor"] = myColor.getColorEnd();

  #ifdef DEBUG
  JsonObject& ESP_Data = root.createNestedObject("ESP_Data");
  ESP_Data["DBG_Debug code"] = "On";
  ESP_Data["DBG_CPU_Freq"] = ESP.getCpuFreqMHz();
  ESP_Data["DBG_Flash Real Size"] = ESP.getFlashChipRealSize();
  ESP_Data["DBG_Free RAM"] = ESP.getFreeHeap();
  ESP_Data["DBG_Free Sketch Space"] = ESP.getFreeSketchSpace();
  ESP_Data["DBG_Sketch Size"] = ESP.getSketchSize();
  root.prettyPrintTo(Serial);
  JsonObject& Server_Args = root.createNestedObject("Server_Args");
  for(uint8_t i = 0; i<server.args(); i++) {
    Server_Args[server.argName(i)] = server.arg(i);
  }
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
  delay(1000);
  clearEEPROM();
  //reset and try again
  #ifdef DEBUG
  Serial.println("Reset ESP and start all over...");
  #endif
  delay(3000);
  ESP.reset();
}

void clearEEPROM(void) {
  //Clearing EEPROM
  #ifdef DEBUG
  Serial.println("Clearing EEPROM");
  #endif
  EEPROM.begin(sizeof(myEEPROMSaveData)+10);
  for(int i = 0; i< EEPROM.length(); i++)
  {
    EEPROM.write(i,0);
  }
  EEPROM.commit();
  EEPROM.end();
}

// Received Factoryreset request.
// To be sure we check the related parameter....
void handleResetRequest(void){
  if(server.arg("rst") == "FactoryReset") {
    server.send(200, "text/plain", "Will now Reset to factory settings. You need to connect to the WLAN AP afterwards....");
    factoryReset();
  } else if(server.arg("rst") == "Defaults") {
    uint32_t colors[3];
    colors[0] = 0xff0000;
    colors[1] = 0x00ff00;
    colors[2] = 0x0000ff;
    strip.setSegment(0, 0, strip.getLength()-1, FX_MODE_STATIC, colors, DEFAULT_SPEED, false);
    setEffect(FX_NO_FX);
    strip.stop();
    strip_On_Off(false);
    server.send(200, "text/plain", "Strip was reset to the default values...");
    shouldSaveRuntime = true;
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
  for(uint8_t a = 0; a < 3; a++) {
    for(uint16_t c = 0; c<256; c++) {
      for(uint16_t i = 0; i<strip.getLength(); i++) {
        strip.setPixelColor(i,0,(uint8_t)c,0);
      }
      strip.show();
      delay(1);
    }
    delay(2);
    for(uint8_t c = 255; c>0; c--) {
      for(uint16_t i = 0; i<strip.getLength(); i++) {
        strip.setPixelColor(i,0,c,0);
      }
      strip.show();
      delay(1);
    }
  }
  //strip.stop();
  delay(100);
  readRuntimeDataEEPROM();
  if(stripIsOn) strip_On_Off(true);
}

// request receive loop
void loop() {
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

  if(shouldSaveRuntime) {
    saveEEPROMData();
    shouldSaveRuntime = false;
  }

}
