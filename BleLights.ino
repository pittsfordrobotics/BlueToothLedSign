#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>
#include <vector>
#include "LightStyle.h"
#include "SingleColorStyle.h"

#define PIN       18 
#define NUMPIXELS 600 
#define DEFAULTSTYLE 0
#define DEFAULTBRIGHTNESS 70  // Brightness should be between 0 and 255.
#define DEFAULTSPEED 50       // Speed should be between 1 and 100.
#define INITIALDELAY 2000     // Startup delay for debugging

// Main BLE service and characteristics
BLEService LEDService("99be4fac-c708-41e5-a149-74047f554cc1");
BLEByteCharacteristic BrightnessCharacteristic("5eccb54e-465f-47f4-ac50-6735bfc0e730", BLERead | BLENotify | BLEWrite);
BLEByteCharacteristic LightStyleCharacteristic("c99db9f7-1719-43db-ad86-d02d36b191b3", BLERead | BLENotify | BLEWrite);
BLEStringCharacteristic StyleNamesCharacteristic("9022a1e0-3a1f-428a-bad6-3181a4d010a5", BLERead, 100);
// Need to add SpeedCharacteristic and StepCharacteristic

// Pixel and color data
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
uint32_t colors[NUMPIXELS];
std::vector<LightStyle*> lightStyles;

// Settings that are updated via bluetooth
uint8_t currentBrightness = DEFAULTBRIGHTNESS;
uint8_t newBrightness = DEFAULTBRIGHTNESS;
uint8_t currentStyle = -1; // Force the style to "change" on the first iteration.
uint8_t newStyle = DEFAULTSTYLE;

// For testing
int loopCount = 1;
unsigned long timestamp = 0;

void setup() {
  // Delay for debugging
  delay(INITIALDELAY);
  Serial.begin(9600);
  Serial.println("Starting...");

  // Initialize the NEOPixels
  pixels.begin();
  pixels.setBrightness(DEFAULTBRIGHTNESS);
  Serial.println("Resetting pixels");
  initializePixels();

  // Define the light known light styles
  Serial.println("Initializing light styles");
  initializeLightStyles();

  // Start up BLE
  Serial.println("Starting BLE");
  startBLE();
}

void loop()
{
  // Loop timing...  remove eventually
  if (loopCount++ % 100 == 0)
  {
    unsigned long newtime = millis();
    unsigned long diff = newtime - timestamp;
    Serial.print("100 iterations: ");
    Serial.println(diff);
    timestamp = newtime;
  }

  readBleSettings();
  updateBrightness();
  updateLEDs();
}

void initializeLightStyles() {
  Serial.println("Initializing light styles");
  lightStyles.push_back(new SingleColorStyle("Red", Adafruit_NeoPixel::Color(255, 0, 0), colors, NUMPIXELS));  
  lightStyles.push_back(new SingleColorStyle("Green", Adafruit_NeoPixel::Color(0, 255, 0), colors, NUMPIXELS));
  lightStyles.push_back(new SingleColorStyle("Blue", Adafruit_NeoPixel::Color(0, 0, 255), colors, NUMPIXELS));
}

void startBLE() {
  Serial.println("Starting BLE...");
  
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");
  }

  String allStyles = "";
  for (int i = 0; i < lightStyles.size(); i++) {
    allStyles += lightStyles[i]->getName();
    if (i < lightStyles.size()-1) {
      allStyles += ";";
    }
  }

  Serial.print("All style names: ");
  Serial.println(allStyles);

  BLE.setLocalName("3181 LED Controller");
  BLE.setAdvertisedService(LEDService);
  BrightnessCharacteristic.setValue(DEFAULTBRIGHTNESS);
  LightStyleCharacteristic.setValue(DEFAULTSTYLE);
  StyleNamesCharacteristic.setValue(allStyles);
  LEDService.addCharacteristic(BrightnessCharacteristic);
  LEDService.addCharacteristic(LightStyleCharacteristic);
  LEDService.addCharacteristic(StyleNamesCharacteristic);
  BLE.addService(LEDService);
  BLE.advertise();
}

void readBleSettings() {
  BLEDevice central = BLE.central();
  if (central) {
    if (BrightnessCharacteristic.written()) {
      Serial.println("Reading new value for brightness.");
      byte valByte = BrightnessCharacteristic.value();
      Serial.print("byte received: ");
      Serial.println(valByte, HEX);
      newBrightness = valByte;
    }

    if (LightStyleCharacteristic.written()) {
      Serial.println("Reading new value for style.");
      byte valByte = LightStyleCharacteristic.value();
      Serial.print("byte received: ");
      Serial.println(valByte, HEX);
      newStyle = valByte;
      // Sanity check
      if (newStyle >= lightStyles.size()) {
        newStyle = lightStyles.size() - 1;        
      }
    }
  }
}

void updateBrightness() {
  if (currentBrightness != newBrightness) {
    Serial.println("Brightness change detected.");
    pixels.setBrightness(newBrightness);
  }

  currentBrightness = newBrightness;
}

void updateLEDs() {
  LightStyle *style = lightStyles[newStyle];
  if (currentStyle != newStyle)  
  {
    Serial.print("Changing style to ");
    Serial.println(newStyle);
    // Changing styles - reset the lights
    style->reset();
    style->setSpeed(DEFAULTSPEED);
    //style->setSpeed(currentSpeed);
    currentStyle = newStyle;
  }

  // if speed changed, reset speed.
  // yeah, this gets done 2x for a style change.
//  if (currentSpeed != newSpeed) {
//    style->setSpeed(newSpeed);
//    currentSpeed = newSpeed;
//  }

  style->update();
  displayColors();  
}

void initializePixels() {
  for (int i = 0; i < NUMPIXELS; i++)
  {
    colors[i] = 0;
  }
  pixels.clear();
}

void displayColors()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, colors[i]);
  }

  pixels.show();
}

// ************************************
// Older routines past here.
// They need to be migrated.
// ************************************

uint16_t currentHue = 0;

void rainbowSwipe()
{
  uint32_t newColor = pixels.ColorHSV(currentHue);
  shiftColorsRight(newColor);
  displayColors();
  currentHue = currentHue + 500;
}

void rainbow()
{
  uint32_t newColor = pixels.ColorHSV(currentHue);
  setAllColors(newColor);
  displayColors();
  currentHue = currentHue + 70;
}

void shiftColorsRight(uint32_t newColor)
{
  for (int i = NUMPIXELS - 1; i >= 1; i--)
  {
    colors[i] = colors[i - 1];
  }
  colors[0] = newColor;
}

void setAllColors(uint32_t newColor)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    colors[i] = newColor;
  }
}

