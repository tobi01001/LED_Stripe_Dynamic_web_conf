// Simple test to verify LED strip compilation and palette distribution functionality compiles
#include <Arduino.h>
#include "LED_strip/led_strip.h"

// Initialize LED arrays for testing
CRGB leds[250];
CRGB bleds[250];

void setup() {
  Serial.begin(115200);
  Serial.println("Testing palette distribution functionality compilation");
  
  // Initialize the LED strip properly using the provided function
  stripe_setup(leds, bleds);
  
  Serial.println("LED strip initialized");
  
  // Test the new paletteDistribution functionality using the field interface
  Serial.println("Testing paletteDistribution parameter:");
  
  // Test using the field interface
  Serial.print("Default paletteDistribution: ");
  Serial.println(getFieldValue("paletteDistribution"));
  
  // Test setting different values using the field interface
  setFieldValue("paletteDistribution", 50);
  Serial.print("After setting to 50%: ");
  Serial.println(getFieldValue("paletteDistribution"));
  
  setFieldValue("paletteDistribution", 200);
  Serial.print("After setting to 200%: ");
  Serial.println(getFieldValue("paletteDistribution"));
  
  setFieldValue("paletteDistribution", 100);
  Serial.print("Reset to 100%: ");
  Serial.println(getFieldValue("paletteDistribution"));
  
  Serial.println("Palette distribution test completed successfully!");
}

void loop() {
  // Nothing to do in loop
  delay(1000);
}