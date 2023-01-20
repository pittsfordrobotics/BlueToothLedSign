#import "Arduino.h"
#import "LightStyle.h"

LightStyle::LightStyle(String name, uint32_t* colors, int numPixels) {
  m_colors = colors;
  m_numPixels = numPixels;
  m_name = name;
}

void LightStyle::setSpeed(uint8_t speed) {
  m_speed = speed;
}

String LightStyle::getName() {
  return m_name;
}

void LightStyle::shiftColorsRight(uint32_t newColor)
{
  for (int i = m_numPixels - 1; i >= 1; i--)
  {
    m_colors[i] = m_colors[i - 1];
  }
  m_colors[0] = newColor;
}
