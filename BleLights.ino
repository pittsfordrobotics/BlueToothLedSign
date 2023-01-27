#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>
#include <vector>
#include "LightStyle.h"
#include "SingleColorStyle.h"
#include "TwoColorStyle.h"
#include "RainbowStyle.h"

#define DATA_OUT 26           // GPIO pin # (NOT Digital pin #) controlling the NeoPixels
#define NUMPIXELS 600         // The total number of pixels to control.
#define DEFAULTSTYLE 6        // The default style to start with. This is an index into the lightStyles vector.
#define DEFAULTBRIGHTNESS 50  // Brightness should be between 0 and 255.
#define DEFAULTSPEED 50       // Speed should be between 1 and 100.
#define DEFAULTSTEP  50       // Step should be between 1 and 100.
#define INITIALDELAY 1000     // Startup delay for debugging
#define TIMINGITERATIONS 500  // The number of iterations between timing messages

// Manual style button configuration.
// The input/output pin numbers are the Digital pin numbers.
int inputPins[] = {15, 16, 17, 18};
int outputPins[] = {19, 4, 3, 2};
// The manual values styles are indexes into the lightStyles vector.
int manualStyles[] = {1, 3, 0, 6};

// Main BLE service and characteristics
BLEService LEDService("99be4fac-c708-41e5-a149-74047f554cc1");
BLEByteCharacteristic BrightnessCharacteristic("5eccb54e-465f-47f4-ac50-6735bfc0e730", BLERead | BLENotify | BLEWrite);
BLEByteCharacteristic LightStyleCharacteristic("c99db9f7-1719-43db-ad86-d02d36b191b3", BLERead | BLENotify | BLEWrite);
BLEStringCharacteristic StyleNamesCharacteristic("9022a1e0-3a1f-428a-bad6-3181a4d010a5", BLERead, 100);
BLEByteCharacteristic SpeedCharacteristic("b975e425-62e4-4b08-a652-d64ad5097815", BLERead | BLENotify | BLEWrite);
BLEByteCharacteristic StepCharacteristic("70e51723-0771-4946-a5b3-49693e9646b5", BLERead | BLENotify | BLEWrite);

// Pixel and color data
Adafruit_NeoPixel pixels(NUMPIXELS, DATA_OUT, NEO_GRB + NEO_KHZ800);
uint32_t colors[NUMPIXELS];
std::vector<LightStyle*> lightStyles;

// Settings that are updated via bluetooth
byte currentBrightness = DEFAULTBRIGHTNESS;
byte newBrightness = DEFAULTBRIGHTNESS;
byte currentStyle = -1; // Force the style to "change" on the first iteration.
byte newStyle = DEFAULTSTYLE;
byte currentSpeed = DEFAULTSPEED;
byte newSpeed = DEFAULTSPEED;
byte currentStep = DEFAULTSTEP;
byte newStep = DEFAULTSTEP;

// For timing information
int loopCount = 1;
unsigned long timestamp = 0;

void setup() {
  // Delay for debugging
  delay(INITIALDELAY);
  Serial.begin(9600);
  Serial.println("Starting...");

  // Initialize digital input/output for manual style selection
  initializeIO();

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
  if (loopCount++ % TIMINGITERATIONS == 0)
  {
    unsigned long newtime = millis();
    unsigned long diff = newtime - timestamp;
    double timePerIteration = (double)diff / TIMINGITERATIONS;
    Serial.print(TIMINGITERATIONS);
    Serial.print(" iterations (msec): ");
    Serial.print(diff);
    Serial.print("; avg per iteration (msec): ");
    Serial.println(timePerIteration);
    timestamp = newtime;
  }

  // TEST CODE
  // To be done if manual selection was detected:
  // Write style choice back to the BLE characteristic
  // Reset brightness and speed/step to defaults? Or leave as-was?
  // When updating via BLE, turn off any manually set LEDs.
  readManualStyleButtons();
  
  readBleSettings();
  updateBrightness();
  updateLEDs();
}

void initializeIO() {
  for (int i = 0; i < 4; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }
}

void initializeLightStyles() {
  uint32_t pink =  Adafruit_NeoPixel::Color(255, 0, 212);
  lightStyles.push_back(new SingleColorStyle("Pink", pink, colors, NUMPIXELS));
  lightStyles.push_back(new SingleColorStyle("Red", Adafruit_NeoPixel::Color(255, 0, 0), colors, NUMPIXELS));  
  lightStyles.push_back(new SingleColorStyle("Green", Adafruit_NeoPixel::Color(0, 255, 0), colors, NUMPIXELS));
  lightStyles.push_back(new SingleColorStyle("Blue", Adafruit_NeoPixel::Color(0, 0, 255), colors, NUMPIXELS));
  lightStyles.push_back(new TwoColorStyle("Red-Pink", Adafruit_NeoPixel::Color(255, 0, 0), pink, colors, NUMPIXELS));
  lightStyles.push_back(new TwoColorStyle("Blue-Pink", Adafruit_NeoPixel::Color(0, 0, 255), pink, colors, NUMPIXELS));
  lightStyles.push_back(new RainbowStyle("Rainbow", colors, NUMPIXELS));
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
  SpeedCharacteristic.setValue(DEFAULTSPEED);
  StepCharacteristic.setValue(DEFAULTSTEP);
  LEDService.addCharacteristic(BrightnessCharacteristic);
  LEDService.addCharacteristic(LightStyleCharacteristic);
  LEDService.addCharacteristic(StyleNamesCharacteristic);
  LEDService.addCharacteristic(SpeedCharacteristic);
  LEDService.addCharacteristic(StepCharacteristic);
  BLE.addService(LEDService);
  BLE.advertise();
}

void readBleSettings() {
  BLEDevice central = BLE.central();
  if (central) {
    if (BrightnessCharacteristic.written()) {
      Serial.println("Reading new value for brightness.");
      newBrightness = readByteFromCharacteristic(BrightnessCharacteristic, 0, 255);
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

    if (SpeedCharacteristic.written()) {
      Serial.println("Reading new value for speed.");
      newSpeed = readByteFromCharacteristic(SpeedCharacteristic, 1, 100);
    }

    if (StepCharacteristic.written()) {
      Serial.println("Reading new value for step.");
      newStep = readByteFromCharacteristic(StepCharacteristic, 1, 100);
    }
  }
}

byte readByteFromCharacteristic(BLEByteCharacteristic characteristic, byte minValue, byte maxValue) {
      byte valByte = characteristic.value();
      Serial.print("byte received: ");
      Serial.println(valByte, HEX);
      if (valByte < minValue) {
        valByte = minValue;
      }
      if (valByte > maxValue) {
        valByte = maxValue;
      }
      return valByte;
}

void readManualStyleButtons() {
  // check the status of the buttons and set leds
  // input = low -> activate led
  for (int i = 0; i < 4; i++) {
    if (digitalRead(inputPins[i]) == LOW) {
      newStyle = manualStyles[i];
      // Set speed/step to default?
      for (int j = 0; j < 4; j++) {
        digitalWrite(outputPins[j], LOW);
      }
      digitalWrite(outputPins[i], HIGH);
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
    style->setSpeed(currentSpeed);
    style->setStep(currentStep);
    style->reset();
    currentStyle = newStyle;
  }

  if (currentSpeed != newSpeed) {
    style->setSpeed(newSpeed);
    currentSpeed = newSpeed;
  }

  if (currentStep != newStep) {
    style->setStep(newStep);
    currentStep = newStep;
  }

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
