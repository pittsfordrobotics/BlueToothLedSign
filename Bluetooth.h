#include <ArduinoBLE.h>
#include <vector>
#include "Arduino.h"

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

# define BLUETOOTH_H_MAXSTRINGLENGTH 250

class Bluetooth {
  public:
    void initialize();
    void stop();
    void resume();

    void setStyleNames(std::vector<String> styleNames);
    void setPatternNames(std::vector<String> patternNames);
    void setBrightness(byte brightness);
    byte getBrightness();
    void setStyle(byte style);
    byte getStyle();
    void setPattern(byte pattern);
    byte getPattern();
    void setStep(byte step);
    byte getStep();
    void setSpeed(byte speed);
    byte getSpeed();

  private:
    BLEService m_ledService{ "99be4fac-c708-41e5-a149-74047f554cc1" };
    BLEByteCharacteristic m_brightnessCharacteristic{ "5eccb54e-465f-47f4-ac50-6735bfc0e730", BLERead | BLENotify | BLEWrite };
    BLEByteCharacteristic m_styleCharacteristic{ "c99db9f7-1719-43db-ad86-d02d36b191b3", BLERead | BLENotify | BLEWrite };
    BLEStringCharacteristic m_styleNamesCharacteristic{ "9022a1e0-3a1f-428a-bad6-3181a4d010a5", BLERead, BLUETOOTH_H_MAXSTRINGLENGTH };
    BLEByteCharacteristic m_speedCharacteristic{ "b975e425-62e4-4b08-a652-d64ad5097815", BLERead | BLENotify | BLEWrite };
    BLEByteCharacteristic m_stepCharacteristic{ "70e51723-0771-4946-a5b3-49693e9646b5", BLERead | BLENotify | BLEWrite };
    BLEByteCharacteristic m_patternCharacteristic{ "6b503d25-f643-4823-a8a6-da51109e713f", BLERead | BLENotify | BLEWrite };
    BLEStringCharacteristic m_patternNamesCharacteristic{ "348195d1-e237-4b0b-aea4-c818c3eb5e2a", BLERead, BLUETOOTH_H_MAXSTRINGLENGTH };

    byte m_currentBrightness{0};
    byte m_currentStyle{0};
    byte m_currentPattern{0};
    byte m_currentSpeed{0};
    byte m_currentStep{0};

    String* joinStrings(std::vector<String> strings);
    byte readByteFromCharacteristic(BLEByteCharacteristic characteristic, byte defaultValue, String name);
};

#endif