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

// Input-Output pin assignments
#define DATA_OUT 25           // GPIO pin # (NOT Digital pin #) controlling the NeoPixels
#define VOLTAGEINPUTPIN 14    // The pin # for the analog input to detect battery voltage level.

// Initial default values for LED styles
#define DEFAULTSTYLE 0        // The default style to start with. This is an index into the lightStyles vector.
#define DEFAULTBRIGHTNESS 255 // Brightness should be between 0 and 255.
#define DEFAULTSPEED 100       // Speed should be between 1 and 100.
#define DEFAULTSTEP  100       // Step should be between 1 and 100.
#define DEFAULTPATTERN 6      // Default patern (ie, Row/Column/Digit/etc). This is an index into the LightStyle::knownPatterns vector.

// Batter power monitoring
#define LOWPOWERTHRESHOLD 6.0     // The voltage below which the system will go into "low power" mode.
#define NORMALPOWERTHRESHOLD 6.9  // The voltage above which the system will recover from "low power" mode.

// Debugging info
#define INITIALDELAY 500      // Startup delay for debugging.
#define TELEMETRYINTERVAL 2000    // The amount of time (in msec) between timing calculations.

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
byte manualStyleIndex = 0;         // Manual style buttons can have more than one style assigned. This indicates which is active for a button.
byte lastManualStyleSelected = -1; // The index of the last manual style that was selected.
long lastManualStyleUpdate = 0;    // The last time a manual button was pressed - used to debounce the signal.
int debouncePeriodMsec = 500;      // The amount of time to wait until reading the manual buttons again.

// Other internal state
int loopCounter = 0;              // Records the number of times the main loop ran since the last timing calculation.
unsigned long lastTelemetryTimestamp = 0;  // The last time debug information was emitted.
byte inLowPowerMode = false;      // Indicates the system should be in "low power" mode. This should be a boolean, but there are no bool types.

// Main entry point for the program --
// This is run once at startup.
void setup() {
  // Delay for debugging
  delay(INITIALDELAY);
  Serial.begin(9600);
  Serial.println("Starting...");
  lastTelemetryTimestamp = millis();

  // Initialize components
  pixelBuffer.initialize();
  pixelBuffer.setBrightness(DEFAULTBRIGHTNESS);
  initializeIO();
  initializeLightStyles();
  initializeManualStyleDefinitions();
  startBLE();
}

// Main loop --
// This metod is called continously.
void loop()
{  
  emitTelemetry();
  checkForLowPowerState();

  if (inLowPowerMode) {
    // blink LEDs and exit.
    blinkLowPowerIndicator();
    return;    
  }

  // See if any settings have been changed via BLE and apply them if necessary.
  readBleSettings();
  if (manualOverrideEnabled) {
    // If any manual style buttons have been pressed, override the BLE-driven settings.
    readManualStyleButtons();
  }

  // Apply any updates that were received via BLE or manually
  updateBrightness();
  updateLEDs();
}

// Initialize all input/output pins
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

// Set up the list of "known" light styles.
// These are the styles presented in the "Styles" list in the phone app.
void initializeLightStyles() {
  Serial.println("Initializing light styles");
  lightStyles.push_back(new RainbowStyle("Rainbow", &pixelBuffer));
  uint32_t pink =  Adafruit_NeoPixel::Color(230, 22, 161);
  uint32_t red = Adafruit_NeoPixel::Color(255, 0, 0);
  uint32_t blue = Adafruit_NeoPixel::Color(0, 0, 255);
  uint32_t white = Adafruit_NeoPixel::Color(255, 255, 255);
  uint32_t orange = Adafruit_NeoPixel::Color(255, 50, 0);
  lightStyles.push_back(new SingleColorStyle("Pink", pink, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Blue-Pink", blue, pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Blue", blue, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Red-Pink", red, pink, &pixelBuffer));
  lightStyles.push_back(new SingleColorStyle("Red", red, &pixelBuffer));
  lightStyles.push_back(new TwoColorStyle("Orange-Pink", orange, pink, &pixelBuffer));
  //lightStyles.push_back(new SingleColorStyle("White", white, &pixelBuffer));
}

// Create the list of manual styles to be assigned to each of the 4 "manual style" buttons.
void initializeManualStyleDefinitions() {
  // Parameters for the ManualSelection struct are: styleIndex, brightness, patternIndex, step, speed
  manualStyleDefinitions[0].push_back(ManualSelection(1, 255, 1, 50, 50));   // Pink
  manualStyleDefinitions[1].push_back(ManualSelection(2, 255, 1, 50, 50));   // Blue-Pink
  manualStyleDefinitions[1].push_back(ManualSelection(3, 255, 1, 50, 50));   // Blue
  manualStyleDefinitions[2].push_back(ManualSelection(4, 255, 1, 50, 50));   // Red-Pink
  manualStyleDefinitions[2].push_back(ManualSelection(5, 255, 1, 50, 50));   // Red
  manualStyleDefinitions[3].push_back(ManualSelection(0, 255, 1, 100, 100)); // Rainbow
}

// Set the initial BLE characteristic values and start the BLE service.
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

// Read the BLE settings to see if any have been changed.
void readBleSettings() {
  newBrightness = btService.getBrightness();

  // Check the range on the characteristic values.
  // If out of range, ignore the update and reset the BLE characteristic to the old value.
  newStyle = btService.getStyle();
  if (!isInRange(newStyle, 0, lightStyles.size()-1)) {
    btService.setStyle(currentStyle);
    newStyle = currentStyle;
  }

  newSpeed = btService.getSpeed();
  if (!isInRange(newSpeed, 1, 100)) {
    btService.setSpeed(currentSpeed);
    newSpeed = currentSpeed;
  }

  newStep = btService.getStep();
  if (!isInRange(newStep, 1, 100)) {
    btService.setStep(currentStep);
    newStep = currentStep;
  }
  
  newPattern = btService.getPattern();
  if (!isInRange(newPattern, 0, LightStyle::knownPatterns.size()-1)) {
    btService.setPattern(currentPattern);
    newPattern = currentPattern;
  }
  
  // If the style changed, clear any manual style indicators.
  if (newStyle != currentStyle) {
    resetManualStyleIndicators();
    lastManualStyleSelected = -1;
  }
}

// Determine if the give byte value is between (or equal to) the min and max values.
byte isInRange(byte value, byte minValue, byte maxValue) {
  return (value >= minValue && value <= maxValue);
}

// See if any manual input buttons were pressed and set the style accordingly.
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

// Turn all manual style LEDs off.
void resetManualStyleIndicators() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(outputPins[i], LOW);
  }  
}

void blinkLowPowerIndicator() {
  // Turn all LEDs off except for the first one, which will blink red.
  pixelBuffer.clearBuffer();
  pixelBuffer.displayPixels();
  delay(500);

  pixelBuffer.setPixel(0, Adafruit_NeoPixel::Color(255, 0, 0));
  pixelBuffer.displayPixels();
  delay(500);
}

// Set the LEDs to a new brightness if the brightness has changed.
void updateBrightness() {
  if (currentBrightness != newBrightness) {
    Serial.println("Brightness change detected.");
    pixelBuffer.setBrightness(newBrightness);
  }

  currentBrightness = newBrightness;
}

// Set the style properties (speed, step, pattern, etc) if any have changed,
// and call the current style class to update the display.
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

// Check if the current battery voltage is too low to run the sign,
// or if the battery has been charged enough to restart operation.
void checkForLowPowerState()
{
  double currentVoltage = getCalculatedBatteryVoltage();

  // Check if the voltage is too low.
  if (currentVoltage < LOWPOWERTHRESHOLD) {
    if (inLowPowerMode) {
      // Already in low power mode - nothing to do.
    } else {
      Serial.print("Battery voltage is below threshold. Voltage: ");
      Serial.print(currentVoltage);
      Serial.print(", threshold: ");
      Serial.println(LOWPOWERTHRESHOLD);
      Serial.println("Entering low power mode.");
      // Enter low power mode. Disable BLE.
      btService.stop();
      inLowPowerMode = true;
      // Low power mode will clear the pixel buffer to save energy.
      // Set current style to -1 so that if we return from low power mode
      // we'll reset back to the last selected style.
      currentStyle = -1;
    }
  }

  // Check if the voltage is high enough for normal operation.
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
      inLowPowerMode = false;
    }
  }
}

// Read the analog input from the "voltage input" pin and calculate the batter voltage.
float getCalculatedBatteryVoltage()
{
  // The analog input ranges from 0 (0V) to 1024 (3.3V), resulting in 0.00322 Volts per "tick".
  // The battery voltage passes through a voltage divider such that the voltage at the input
  // is 1/3 of the actual battery voltage.
  float rawLevel = getVoltageInputLevel();
  return rawLevel * 3 * 3.3 / 1024;
}

// Read the raw value from the "voltage input" pin.
int getVoltageInputLevel()
{
  return analogRead(VOLTAGEINPUTPIN);
}

// Calculate loop timing information and emit the current battery voltage level.
void emitTelemetry()
{
  loopCounter++;
  unsigned long timestamp = millis();

  if (timestamp > lastTelemetryTimestamp + TELEMETRYINTERVAL)
  {
    // Calculate loop timing data
    unsigned long diff = timestamp - lastTelemetryTimestamp;
    double timePerIteration = (double)diff / loopCounter;
    Serial.print(loopCounter);
    Serial.print(" iterations (msec): ");
    Serial.print(diff);
    Serial.print("; avg per iteration (msec): ");
    Serial.println(timePerIteration);
    lastTelemetryTimestamp = timestamp;
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
