#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>
#include <vector>
#include "PixelBuffer.h"
#include "LightStyle.h"
#include "SingleColorStyle.h"
#include "TwoColorStyle.h"
#include "RainbowStyle.h"
#include "Bluetooth.h"

#define DATA_OUT 25           // GPIO pin # (NOT Digital pin #) controlling the NeoPixels
#define DEFAULTSTYLE 1        // The default style to start with. This is an index into the lightStyles vector.
#define DEFAULTBRIGHTNESS 255 // Brightness should be between 0 and 255.
#define DEFAULTSPEED 50       // Speed should be between 1 and 100.
#define DEFAULTSTEP  50       // Step should be between 1 and 100.
#define DEFAULTPATTERN 0      // Default patern (ie, Row/Column/Digit/etc). This is an index into the LightStyle::knownPatterns vector.
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

// Main BLE service wrapper
Bluetooth btService;

// Pixel and color data
PixelBuffer pixelBuffer(DATA_OUT);
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
byte currentPattern = DEFAULTPATTERN;
byte newPattern = DEFAULTPATTERN;

// Other internal state
// For timing and debug information
#define DEBUGINTERVAL 2000        // The amount of time (in msec) between timing calculations.
int loopCounter = 0;              // Records the number of times the main loop ran since the last timing calculation.
unsigned long lastTimestamp = 0;  // The last time debug information was emitted.
byte inLowPowerMode = false;      // Indicates the system should be in "low power" mode. This should be a boolean, but there are no bool types.

void setup() {
  // Delay for debugging
  delay(INITIALDELAY);
  Serial.begin(9600);
  Serial.println("Starting...");
  lastTimestamp = millis();

  pixelBuffer.initialize();
  pixelBuffer.setBrightness(DEFAULTBRIGHTNESS);
  initializeIO();
  initializeLightStyles();
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

void initializeIO() {
  Serial.println("Initializing manual override I/O pins.");
  for (int i = 0; i < 4; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  Serial.println("Initializing the analog input to monitor battery voltage.");
  pinMode(VOLTAGEINPUTPIN, INPUT);
}

void initializeLightStyles() {
  Serial.println("Initializing light styles");
  lightStyles.push_back(new RainbowStyle("Rainbow", &pixelBuffer));
  uint32_t pink =  Adafruit_NeoPixel::Color(255, 0, 212);
  lightStyles.push_back(new SingleColorStyle("Pink", pink, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Blue-Pink", Adafruit_NeoPixel::Color(0, 0, 255), pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Blue", Adafruit_NeoPixel::Color(0, 0, 255), &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Red-Pink", Adafruit_NeoPixel::Color(255, 0, 0), pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Red", Adafruit_NeoPixel::Color(255, 0, 0), &pixelBuffer));  
}

void startBLE() {
  btService.initialize();

  std::vector<String> styleNames;
  for (int i = 0; i < lightStyles.size(); i++) {
    styleNames.push_back(lightStyles[i]->getName());    
  }  

  btService.setStyleNames(styleNames);
  btService.setPatternNames(LightStyle::knownPatterns);
  btService.setBrightness(DEFAULTBRIGHTNESS);
  btService.setStyle(DEFAULTSTYLE);
  btService.setSpeed(DEFAULTSPEED);
  btService.setPattern(DEFAULTPATTERN);
  btService.setStep(DEFAULTSTEP);
}

void readBleSettings() {
  newBrightness = applyRange(btService.getBrightness(), 0, 255);
  newStyle = applyRange(btService.getStyle(), 0, lightStyles.size() - 1);
  newSpeed = applyRange(btService.getSpeed(), 1, 100);
  newStep = applyRange(btService.getStep(), 1, 100);
  newPattern = applyRange(btService.getPattern(), 0, LightStyle::knownPatterns.size() - 1);

  // If the style changed, clear any manual style indicators.
  if (newStyle != currentStyle) {
    resetManualStyleIndicators();
  }
}

byte applyRange(byte value, byte minValue, byte maxValue) {
  if (value < minValue) {
    return minValue;
  }

  if (value > maxValue) {
    return maxValue;
  }

  return value;
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
      btService.setStyle(newStyle);
      return;
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
    pixelBuffer.setBrightness(newBrightness);
  }

  currentBrightness = newBrightness;
}

void updateLEDs() {
  int shouldResetStyle = false;
  LightStyle *style = lightStyles[newStyle];
  if (currentStyle != newStyle)  
  {
    Serial.print("Changing style to ");
    Serial.println(newStyle);
    // Changing styles - reset the lights
    style->setSpeed(currentSpeed);
    style->setStep(currentStep);
    style->setPattern(currentPattern);
    shouldResetStyle = true;
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

  if (currentPattern != newPattern) {
    style->setPattern(newPattern);
    shouldResetStyle = true;
    currentPattern = newPattern;
  }

  if (shouldResetStyle) {
    style->reset();
  }

  style->update();
  pixelBuffer.displayPixels();
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
      // For the LEDs, set brightness to 0 and call display to turn them off.
      // (This keeps the pixel buffer intact so we can resume where we left off when power returns.)
      pixelBuffer.setBrightness(0);
      pixelBuffer.displayPixels();
      btService.stop();
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
      btService.resume();
      pixelBuffer.setBrightness(currentBrightness);
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
