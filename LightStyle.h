#include "Arduino.h"
#include "PixelBuffer.h"

#ifndef LIGHT_STYLE_H
#define LIGHT_STYLE_H

#define LIGHT_PATTERN_LINEAR 0;
#define LIGHT_PATTERN_COLUMN_RIGHT 1;
#define LIGHT_PATTERN_COLUMN_LEFT 2;
#define LIGHT_PATTERN_ROW_DOWN 3;
#define LIGHT_PATTERN_ROW_UP 4;

class LightStyle {
  public:
    LightStyle(String name, PixelBuffer* pixelBuffer);

    // Gets the name of the style.
    String getName();

    // Sets the frequency at which the light style updates.
    void setSpeed(byte speed);

    // Sets the "step", which is defined by the style.
    // Examples would be the ratio of light-to-dark colors for 1- or 2-color styles,
    // or the amount of hue change for a rainbow style.
    void setStep(byte step);

    // Sets the display pattern, if supported by the style.
    // Use LIGHT_PATTERN_* macros to supply values.
    void setPattern(byte pattern);

    // Populates the buffer with a pattern of colors to show when the
    // light style has been selected.
    virtual void reset() = 0;

    // Updates the pixel buffer.
    virtual void update() = 0;

  protected:
    PixelBuffer* m_pixelBuffer;
    String m_name;
    byte m_speed;
    byte m_step;
    byte m_pattern;
};
   
#endif