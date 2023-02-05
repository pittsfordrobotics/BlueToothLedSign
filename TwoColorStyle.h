#include "LightStyle.h"
#include "Arduino.h"
#include "PixelBuffer.h"

#ifndef TWO_COLOR_STYLE_H
#define TWO_COLOR_STYLE_H

class TwoColorStyle : public LightStyle {
  public:
    TwoColorStyle(String name, uint32_t color1, uint32_t color2, PixelBuffer* pixelBuffer);
    
    void reset();
    void update();

  private:
    int getIterationDelay();
    int getModulus();

    uint32_t m_color1;
    uint32_t m_color2;
    int m_iterationCount;
    unsigned long m_nextUpdate;
};

#endif