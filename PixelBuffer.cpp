#include <Adafruit_NeoPixel.h>
#include "Arduino.h"
#include "PixelBuffer.h"

PixelBuffer::PixelBuffer(int16_t gpioPin) {
  m_pixelBuffer = new uint32_t[PIXELBUFFER_PIXELCOUNT];
  m_neoPixels = new Adafruit_NeoPixel(PIXELBUFFER_PIXELCOUNT, gpioPin, NEO_GRB + NEO_KHZ800);
}

void PixelBuffer::clearBuffer() {
  for (int i = 0; i < PIXELBUFFER_PIXELCOUNT; i++) {
    m_pixelBuffer[i] = 0;
  }
}

void PixelBuffer::displayPixels() {
  for (int i = 0; i < PIXELBUFFER_PIXELCOUNT; i++)
  {
    m_neoPixels->setPixelColor(i, m_pixelBuffer[i]);
  }

  m_neoPixels->show();
}

unsigned int PixelBuffer::getPixelCount() {
  return PIXELBUFFER_PIXELCOUNT;
}

void PixelBuffer::initialize() {
  clearBuffer();
  m_neoPixels->begin();
  m_neoPixels->clear();
}

void PixelBuffer::setBrightness(uint8_t brightness) {
  m_neoPixels->setBrightness(brightness);
}

void PixelBuffer::setPixel(unsigned int pixel, uint32_t color) {
  if (pixel >= PIXELBUFFER_PIXELCOUNT) {
    return;
  }

  m_pixelBuffer[pixel] = color;
}

void PixelBuffer::shiftLineRight(uint32_t newColor)
{
  for (int i = PIXELBUFFER_PIXELCOUNT - 1; i >= 1; i--)
  {
    m_pixelBuffer[i] = m_pixelBuffer[i - 1];
  }

  m_pixelBuffer[0] = newColor;
}

void PixelBuffer::shiftLineLeft(uint32_t newColor)
{
  for (int i = 0; i < PIXELBUFFER_PIXELCOUNT - 1; i++)
  {
    m_pixelBuffer[i] = m_pixelBuffer[i + 1];
  }

  m_pixelBuffer[PIXELBUFFER_PIXELCOUNT - 1] = newColor;
}

void PixelBuffer::shiftColumnsRight(uint32_t newColor)
{
  // Testing... just call shiftLine for now.
  shiftLineRight(newColor);
}

void PixelBuffer::shiftColumnsLeft(uint32_t newColor)
{
  // Testing... just call shiftLine for now.
  shiftLineLeft(newColor);
}

void PixelBuffer::shiftRowsUp(uint32_t newColor)
{
  // Testing... just call shiftLine for now.
  shiftLineRight(newColor);
}

void PixelBuffer::shiftRowsDown(uint32_t newColor)
{
  // Testing... just call shiftLine for now.
  shiftLineLeft(newColor);
}

/*
       3
     2   4
   1       5
 0           6
  11       7
    10   8
       9
*/
