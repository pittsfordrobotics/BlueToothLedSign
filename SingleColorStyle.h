#include "LightStyle.h"
#include "Arduino.h"
#include "PixelBuffer.h"

#ifndef SINGLE_COLOR_STYLE_H
#define SINGLE_COLOR_STYLE_H

class SingleColorStyle : public LightStyle {
  public:
    SingleColorStyle(String name, uint32_t color, PixelBuffer* pixelBuffer);
    
    void reset();
    void update();

  private:
    uint32_t m_color;
};

#endif