#include "Arduino.h"
#include "SingleColorStyle.h"
#include "PixelBuffer.h"

SingleColorStyle::SingleColorStyle(String name, uint32_t color, PixelBuffer* pixelBuffer) : LightStyle(name, pixelBuffer) {
  m_color = color;
}

void SingleColorStyle::update() {
  // Single color style does not blink or anything.
  // Update does nothing.
}

void SingleColorStyle::reset()
{
  // Set all pixels to a single color.
  for (int i = 0; i < m_pixelBuffer->getPixelCount(); i++) {
    m_pixelBuffer->setPixel(i, m_color);
  }
}
