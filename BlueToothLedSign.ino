#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>
#include <vector>
#include "PixelBuffer.h"
#include "LightStyle.h"
#include "SingleColorStyle.h"
#include "TwoColorStyle.h"
#include "RainbowStyle.h"
#include "Bluetooth.h"
#include "ManualSelection.h"

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
int inputPins[] = {3, 4, 5, 6};   // The pins attached to the buttons
int outputPins[] = {7, 8, 10, 9}; // The pins attached to the LED indicators
std::vector<ManualSelection>* manualStyleDefinitions = new std::vector<ManualSelection>[4];

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

// State-keeping for the manual style buttons
byte manualStyleIndex = 0; // Manual style buttons can have more than one style assigned. This indicates which is active for a button.
byte lastManualStyleSelected = -1; // The index of the last manual style that was selected.
long lastManualStyleUpdate = 0; // The last time a manual button was pressed - used to debounce the signal.
int debouncePeriodMsec = 500; // The amount of time to wait until reading the manual buttons again.

// Other internal state
// For timing and debug information
#define TELEMETRYINTERVAL 2000    // The amount of time (in msec) between timing calculations.
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
  initializeManualStyleDefinitions();
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
  uint32_t red = Adafruit_NeoPixel::Color(255, 0, 0);
  uint32_t blue = Adafruit_NeoPixel::Color(0, 0, 255);
  lightStyles.push_back(new SingleColorStyle("Pink", pink, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Blue-Pink", blue, pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Blue", blue, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Red-Pink", red, pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Red", red, &pixelBuffer));  
}

void initializeManualStyleDefinitions() {
  // Create the list of manual styles to be assigned to each of the 4 "manual style" buttons.
  // Parameters for the ManualSelection struct are: styleIndex, brightness, patternIndex, step, speed
  manualStyleDefinitions[0].push_back(ManualSelection(1, 255, 1, 50, 50));   // Pink
  manualStyleDefinitions[1].push_back(ManualSelection(2, 255, 1, 50, 50));   // Blue-Pink
  manualStyleDefinitions[1].push_back(ManualSelection(3, 255, 1, 50, 50));   // Blue
  manualStyleDefinitions[2].push_back(ManualSelection(4, 255, 1, 50, 50));   // Red-Pink
  manualStyleDefinitions[2].push_back(ManualSelection(5, 255, 1, 50, 50));   // Red
  manualStyleDefinitions[3].push_back(ManualSelection(0, 255, 1, 250, 250)); // Rainbow
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
    lastManualStyleSelected = -1;
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
  long now = millis();
  if (now < lastManualStyleUpdate + debouncePeriodMsec) {
    // We're in the debounce window.  Just exit.
    return;
  }

  // Check the status of the buttons and set LEDs.
  // input = low means the button has been pressed
  for (int i = 0; i < 4; i++) {
    if (digitalRead(inputPins[i]) == LOW) {
      if (lastManualStyleSelected == i) {
        // Pressed the same button again - update the style index.
        manualStyleIndex = ++manualStyleIndex % manualStyleDefinitions[i].size();
      } else {
        // Selected a different button. Reset the style index.
        manualStyleIndex = 0;
        lastManualStyleSelected = i;
      }
      ManualSelection selection = manualStyleDefinitions[i].at(manualStyleIndex);
      newStyle = selection.StyleIndex;
      newBrightness = selection.Brightness;
      newStep = selection.Step;
      newSpeed = selection.Speed;
      newPattern = selection.PatternIndex;
      resetManualStyleIndicators();
      // Turn on the corresponding status LED to indicate the manual style was selected.
      digitalWrite(outputPins[i], HIGH);
      btService.setStyle(newStyle);
      btService.setSpeed(newSpeed);
      btService.setStep(newStep);
      btService.setBrightness(newBrightness);
      btService.setPattern(newPattern);
      lastManualStyleUpdate = now;
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

float getCalculatedBatteryVoltage()
{
  // The analog input ranges from 0 (0V) to 1024 (3.3V), resulting in 0.00322 Volts per "tick".
  // The battery voltage passes through a voltage divider such that the voltage at the input
  // is 1/3 of the actual battery voltage.
  float rawLevel = getVoltageInputLevel();
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

  if (timestamp > lastTimestamp + TELEMETRYINTERVAL)
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
    float voltage = getCalculatedBatteryVoltage();

    // Emit battery voltage information on Bluetooth as well as Serial.
    btService.emitBatteryVoltage(voltage);
    Serial.print("Analog input: ");
    Serial.print(rawLevel);
    Serial.print("; calculated voltage: ");
    Serial.println(voltage);
  }
}
