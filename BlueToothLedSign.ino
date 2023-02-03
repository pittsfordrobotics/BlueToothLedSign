#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>
#include <vector>
#include "LightStyle.h"
#include "SingleColorStyle.h"
#include "TwoColorStyle.h"
#include "RainbowStyle.h"

#define DATA_OUT 25           // GPIO pin # (NOT Digital pin #) controlling the NeoPixels
#define NUMPIXELS 600         // The total number of pixels to control.
#define DEFAULTSTYLE 6        // The default style to start with. This is an index into the lightStyles vector.
#define DEFAULTBRIGHTNESS 50  // Brightness should be between 0 and 255.
#define DEFAULTSPEED 50       // Speed should be between 1 and 100.
#define DEFAULTSTEP  50       // Step should be between 1 and 100.
#define INITIALDELAY 1000     // Startup delay for debugging.
#define TIMINGITERATIONS 100  // The number of iterations between timing messages.
#define VOLTAGEINPUTPIN 14    // The pin # for the analog input to detect battery voltage level.
#define LOWPOWERTHRESHOLD 5.9     // The voltage below which the system will go into "low power" mode.
#define NORMALPOWERTHRESHOLD 6.9  // The voltage above which the system will recover from "low power" mode.

// Manual style button configuration.
// The input/output pin numbers are the Digital pin numbers.
bool manualOverrideEnabled = true;
int inputPins[] = {3, 4, 5, 6};
int outputPins[] = {9, 10, 8, 7};
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

byte inLowPowerMode = false;      // Indicates the system should be in "low power" mode. This should be a boolean, but there are no bool types.

// For timing and debug information
#define DEBUGINTERVAL 2000        // The amount of time (in msec) between timing calculations.
int loopCounter = 0;
unsigned long lastTimestamp = 0;

void setup() {
  // Delay for debugging
  delay(INITIALDELAY);
  Serial.begin(9600);
  Serial.println("Starting...");
  lastTimestamp = millis();

  initializeManualIO();

  // Configure the analog input pin to be able to read the battery voltage
  pinMode(VOLTAGEINPUTPIN, INPUT);

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
  printTimingAndDebugInfo();
  checkForLowPowerState();

  if (inLowPowerMode) {
    // blink LEDs and exit.
    blinkManualStyleIndicators();
    return;    
  }

  readBleSettings();

  if (manualOverrideEnabled) {
    readManualStyleButtons();
  }

  // Apply any updates that were received via BLE or manually
  updateBrightness();
  updateLEDs();
}

void initializeManualIO() {
  Serial.println("Initializing manual override I/O pins.");
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
  Serial.print("Style name string length: ");
  Serial.println(allStyles.length());

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
      newStyle = readByteFromCharacteristic(LightStyleCharacteristic, 0, lightStyles.size() - 1);
      resetManualStyleIndicators();
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
  // input = low means the button has been pressed
  for (int i = 0; i < 4; i++) {
    if (digitalRead(inputPins[i]) == LOW) {
      newStyle = manualStyles[i];
      resetManualStyleIndicators();
      // Turn on the corresponding status LED to indicate the manual style was selected.
      digitalWrite(outputPins[i], HIGH);
    }
  }
}

void resetManualStyleIndicators() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(outputPins[i], LOW);
  }  
}

void blinkManualStyleIndicators() {
  // Provides a visual indication that the system is in a Low Power state
  // by blinking all of the "manual style selection" LEDs.
  // Turn all indicators on
  for (int i = 0; i < 4; i++) {
    digitalWrite(outputPins[i], HIGH);
  }
  delay(500);

  // Turn all indicators back off
  for (int i = 0; i < 4; i++) {
    digitalWrite(outputPins[i], LOW);
  }
  delay(500);
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

// Initialize the pixel buffer and reset all LEDs to an off state.
void initializePixels() {
  for (int i = 0; i < NUMPIXELS; i++)
  {
    colors[i] = 0;
  }
  pixels.clear();
  pixels.show();
}

// Copy the pixel values from the buffer to the LEDs.
void displayColors()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, colors[i]);
  }

  pixels.show();
}

void checkForLowPowerState()
{
  double currentVoltage = getCalculatedBatteryVoltage();
  if (currentVoltage < LOWPOWERTHRESHOLD) {
    if (inLowPowerMode) {
      // Already in low power mode - nothing to do.
    } else {
      Serial.print("Battery voltage is below threshold. Voltage: ");
      Serial.print(currentVoltage);
      Serial.print(", threshold: ");
      Serial.println(LOWPOWERTHRESHOLD);
      Serial.println("Entering low power mode.");
      // Enter low power mode. Disable LEDs and BLE.
      pixels.clear();
      pixels.show();
      BLE.disconnect();
      BLE.stopAdvertise();
      inLowPowerMode = true;
    }
  }
  if (currentVoltage > NORMALPOWERTHRESHOLD) {
    if (!inLowPowerMode) {
      // Not in low power mode - nothing to do.
    } else {
      Serial.print("Battery voltage is above normal threshold. Voltage: ");
      Serial.print(currentVoltage);
      Serial.print(", threshold: ");
      Serial.println(NORMALPOWERTHRESHOLD);
      Serial.println("Exiting low power mode.");
      // Exit low power mode.  Re-enable BLE.
      BLE.advertise();
      inLowPowerMode = false;
    }
  }
}

double getCalculatedBatteryVoltage()
{
  // The analog input ranges from 0 (0V) to 1024 (3.3V), resulting in 0.00322 Volts per "tick".
  // The battery voltage passes through a voltage divider such that the voltage at the input
  // is 1/3 of the actual battery voltage.
  double rawLevel = getVoltageInputLevel();
  return rawLevel * 3 * 3.3 / 1024;
}

int getVoltageInputLevel()
{
  return analogRead(VOLTAGEINPUTPIN);
}

void printTimingAndDebugInfo()
{
  loopCounter++;
  unsigned long timestamp = millis();

  if (timestamp > lastTimestamp + DEBUGINTERVAL)
  {
    // Calculate loop timing data
    unsigned long diff = timestamp - lastTimestamp;
    double timePerIteration = (double)diff / loopCounter;
    Serial.print(loopCounter);
    Serial.print(" iterations (msec): ");
    Serial.print(diff);
    Serial.print("; avg per iteration (msec): ");
    Serial.println(timePerIteration);
    lastTimestamp = timestamp;
    loopCounter = 0;
    
    // Output voltage info periodically
    int rawLevel = getVoltageInputLevel();
    double voltage = getCalculatedBatteryVoltage();
    Serial.print("Analog input: ");
    Serial.print(rawLevel);
    Serial.print("; calculated voltage: ");
    Serial.println(voltage);
  }
}
