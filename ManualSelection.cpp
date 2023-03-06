#include "ManualSelection.h"
#include "Arduino.h"

ManualSelection::ManualSelection(byte styleIndex, byte brightness, byte patternIndex, byte step, byte speed)
{
  StyleIndex = styleIndex;
  Brightness = brightness;
  PatternIndex = patternIndex;
  Step = step;
  Speed = speed;
}
