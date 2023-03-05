#include "Arduino.h"

#ifndef MANUAL_SELECTION_H
#define MANUAL_SELECTION_H

struct ManualSelection {
  public:
    ManualSelection(byte styleIndex, byte brightness, byte patternIndex, byte step, byte speed);

    byte StyleIndex;
    byte Brightness;
    byte PatternIndex;
    byte Step;
    byte Speed;
};

#endif