#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>

#define PIN       18 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 600 
#define DELAYVAL 1 // Time (in milliseconds) to pause between iterations

// Main BLE service
BLEService LEDService("99be4fac-c708-41e5-a149-74047f554cc1");

// Characteristic to control LED brightness
BLEByteCharacteristic BrightnessCharacteristic("5eccb54e-465f-47f4-ac50-6735bfc0e730", BLERead | BLENotify | BLEWrite);
BLEByteCharacteristic LightStyleCharacteristic("c99db9f7-1719-43db-ad86-d02d36b191b3", BLERead | BLENotify | BLEWrite);
BLEStringCharacteristic StyleNamesCharacteristic("9022a1e0-3a1f-428a-bad6-3181a4d010a5", BLERead, 100);

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
uint32_t colors[NUMPIXELS];
uint16_t currentHue = 0;
uint8_t currentBrightness = 50;
uint8_t newBrightness = currentBrightness;
uint8_t currentStyle = 3;
uint8_t newStyle = currentStyle;

int count = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(currentBrightness);
  Serial.println("Resetting pixels");
  resetLights();
  Serial.println("Starting BLE");
  startBLE();
}

int loopCount = 0;
unsigned long timestamp = 0;

void loop()
{
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
  delay(DELAYVAL);
}

void startBLE() {
  Serial.println("Starting BLE...");
  
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");
  }

  // dynamically construct the name list here...
  String styles = "Red1;Blue1;Green1;Rainbow";

  BLE.setLocalName("Nano Button Device");
  BLE.setAdvertisedService(LEDService);
  BrightnessCharacteristic.setValue(currentBrightness);
  LightStyleCharacteristic.setValue(currentStyle);
  StyleNamesCharacteristic.setValue(styles);
  LEDService.addCharacteristic(BrightnessCharacteristic);
  LEDService.addCharacteristic(LightStyleCharacteristic);
  LEDService.addCharacteristic(StyleNamesCharacteristic);
  BLE.addService(LEDService);
  BLE.advertise();
}

void resetLights() {
  currentHue = 0;
  for (int i = 0; i < NUMPIXELS; i++)
  {
    colors[i] = 0;
  }
  pixels.clear();
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
  if (currentStyle != newStyle)  
  {
    resetLights();
  } 

  currentStyle = newStyle;
  switch (currentStyle) {
    case 0:
      showRed();
      break;
    case 1:
      showBlue();
      break;
    case 2:
      showGreen();
      break;
    case 3:
      rainbow();
      break;
    default:
      break;
  }
}

void showRed() {
  setAllColors(pixels.Color(255,0,0));
  displayColors();  
}

void showBlue() {
  setAllColors(pixels.Color(0,0,255));
  displayColors();  
}

void showGreen() {
  setAllColors(pixels.Color(0,255,0));
  displayColors();  
}

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

void displayColors()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, colors[i]);
  }

  pixels.show();
}

void fadeIn(int red, int blue, int green)
{
  fade(red, blue, green, false);
}

void fadeOut(int red, int blue, int green)
{
  fade(red, blue, green, true);
}

void fade(int red, int blue, int green, bool shouldFadeOut)
{
  int smallDelay = 20;
  int fadeStep = 1;
  int currentFade = 0;
  if (shouldFadeOut)
  {
    fadeStep = -1;
    currentFade = 100;
  }
  while (currentFade <= 100 && currentFade >=0)
  {
    double multiplier = pow(currentFade / 100.0, 2);
    Serial.print("Multiplier: ");
    Serial.println(multiplier);
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(red * multiplier, green * multiplier, blue * multiplier));
    }
    pixels.show();
    delay(smallDelay);
    currentFade += fadeStep;
  }
}
