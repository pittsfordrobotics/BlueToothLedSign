#include "LightStyle.h"
#include "Arduino.h"

#ifndef SINGLE_COLOR_STYLE_H
#define SINGLE_COLOR_STYLE_H

class SingleColorStyle : public LightStyle {
  public:
    SingleColorStyle(String name, uint32_t color, uint32_t* colors, int numPixels);
    
    void reset();
    void update();

  private:
    int getIterationDelay();

    uint32_t m_color;
    int m_iterationCount;
    unsigned long m_nextUpdate;
};

#endif