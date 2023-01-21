#include "Arduino.h"

#ifndef LIGHT_STYLE_H
#define LIGHT_STYLE_H

class LightStyle {
  public:
    LightStyle(String name, uint32_t* colors, int numPixels);
    String getName();
    void setSpeed(byte speed);
    void setStep(byte step);

    virtual void reset() = 0;
    virtual void update() = 0;

  protected:
    void shiftColorsRight(uint32_t newColor);

    int m_numPixels;
    uint32_t* m_colors;
    String m_name;
    byte m_speed;
    byte m_step;
};
   
#endif