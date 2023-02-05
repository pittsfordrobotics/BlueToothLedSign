#include <Adafruit_NeoPixel.h>
#include "LightStyle.h"
#include "Arduino.h"
#include "PixelBuffer.h"

#ifndef RAINBOW_STYLE_H
#define RAINBOW_STYLE_H

class RainbowStyle : public LightStyle {
  public:
    RainbowStyle(String name, PixelBuffer* pixelBuffer);
    
    void reset();
    void update();

  private:
    int getIterationDelay();
    void incrementHue();

    uint16_t m_currentHue;
    unsigned long m_nextUpdate;
};

#endif