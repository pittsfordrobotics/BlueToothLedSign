#include "Arduino.h"
#include "PixelBuffer.h"

#ifndef LIGHT_STYLE_H
#define LIGHT_STYLE_H

class LightStyle {
  public:
    LightStyle(String name, PixelBuffer* pixelBuffer);
    String getName();
    void setSpeed(byte speed);
    void setStep(byte step);

    virtual void reset() = 0;
    virtual void update() = 0;

  protected:
    PixelBuffer* m_pixelBuffer;
    String m_name;
    byte m_speed;
    byte m_step;
};
   
#endif