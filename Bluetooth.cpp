#include <ArduinoBLE.h>
#include <vector>
#include "Arduino.h"
#include "Bluetooth.h"

void Bluetooth::initialize() {
}

byte Bluetooth::getBrightness() {
  m_currentBrightness = readByteFromCharacteristic(m_brightnessCharacteristic, m_currentBrightness, "brightness");
  return m_currentBrightness;
}

void Bluetooth::setBrightness(byte brightness) {
  m_currentBrightness = brightness;
  m_brightnessCharacteristic.setValue(brightness);
}

byte Bluetooth::getStyle() {
  m_currentStyle = readByteFromCharacteristic(m_styleCharacteristic, m_currentStyle, "style");
  return m_currentStyle;
}

void Bluetooth::setStyle(byte style) {
  m_currentStyle = style;
  m_styleCharacteristic.setValue(style);
}

byte Bluetooth::getSpeed() {
  m_currentSpeed = readByteFromCharacteristic(m_speedCharacteristic, m_currentSpeed, "speed");
  return m_currentSpeed;
}

void Bluetooth::setSpeed(byte speed) {
  m_currentSpeed = speed;
  m_speedCharacteristic.setValue(speed);
}

byte Bluetooth::getPattern() {
  m_currentPattern = readByteFromCharacteristic(m_patternCharacteristic, m_currentPattern, "pattern");
  return m_currentPattern;
}

void Bluetooth::setPattern(byte pattern) {
  m_currentPattern = pattern;
  m_patternCharacteristic.setValue(pattern);
}

byte Bluetooth::getStep() {
  m_currentStep = readByteFromCharacteristic(m_stepCharacteristic, m_currentStep, "step");
  return m_currentStep;
}

void Bluetooth::setStep(byte step) {
  m_currentStep = step;
  m_stepCharacteristic.setValue(step);
}

byte Bluetooth::readByteFromCharacteristic(BLEByteCharacteristic characteristic, byte defaultValue, String name) {
  BLEDevice central = BLE.central();
  if (central) {
    if (characteristic.written()) {
      Serial.print("Reading new value for ");
      Serial.print(name);
      byte valByte = characteristic.value();
      Serial.print(". Byte received: ");
      Serial.println(valByte, HEX);
      return valByte;
    }
  }

  return defaultValue;
}

String* Bluetooth::joinStrings(std::vector<String> strings) {
  String* joinedStrings = new String();
  for (int i = 0; i < strings.size(); i++) {
    joinedStrings->concat(strings.at(i));
    if (i < strings.size()-1) {
      joinedStrings->concat(";");
    }
  }

  return joinedStrings;
}
