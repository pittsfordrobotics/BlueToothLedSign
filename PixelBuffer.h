#ifndef PIXEL_BUFFER_H
#define PIXEL_BUFFER_H
#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

#define PIXELBUFFER_PIXELCOUNT 600

class PixelBuffer {
  public:
    PixelBuffer(int16_t gpioPin);
    void setBrightness(uint8_t brightess);

    // Sets the first pixel in the buffer to the new color,
    // shifting all the pixels in the buffer to the right by one.
    void shiftLineRight(uint32_t newColor);

    // Sets the pixels in the first column to the new color,
    // shifting all the columns to the right by one.
    void shiftColumnsRight(uint32_t newColor);

    // Sets the pixels in the last column to the new color,
    // shifting all the columns to the left by one.
    void shiftColumnsLeft(uint32_t newColor);

    // Sets the pixels in the bottom row to the new color,
    // shifting all the rows up by one.
    void shiftRowsUp(uint32_t newColor);

    // Sets the pixels in the top row to the new color,
    // shifting all the rows down b one.
    void shiftRowsDown(uint32_t newColor);

    uint32_t getPixel(unsigned int pixel);
    unsigned int getPixelCount();
    unsigned int getColumnCount();
    unsigned int getRowCount();
    void displayPixels();
    void clearBuffer();

  private:
    Adafruit_NeoPixel* m_neoPixels;
    unsigned int m_numPixels;
    uint32_t* m_pixelBuffer;
    unsigned int m_colCount;
    unsigned int m_rowCount;
};

#endif