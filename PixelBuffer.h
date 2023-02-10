#include <Adafruit_NeoPixel.h>
#include <vector>
#include "Arduino.h"

#ifndef PIXEL_BUFFER_H
#define PIXEL_BUFFER_H

class PixelBuffer {
  public:
    PixelBuffer(int16_t gpioPin);
    void setBrightness(uint8_t brightess);
    void initialize();

    // Sets the first pixel in the buffer to the new color,
    // shifting all the pixels in the buffer to the right by one.
    void shiftLineRight(uint32_t newColor);

    // Sets the last pixel in the buffer to the new color,
    // shifting all the pixels in the buffer to the left by one.
    void shiftLineLeft(uint32_t newColor);

    // Sets the pixels in the first column to the new color,
    // shifting all the columns to the right by one.
    void shiftColumnsRight(uint32_t newColor);

    // Sets the pixels in the last column to the new color,
    // shifting all the columns to the left by one.
    void shiftColumnsLeft(uint32_t newColor);

    // Sets the pixels in the first digit to the new color,
    // shifting all the digits to the right by one.
    void shiftDigitsRight(uint32_t newColor);

    // Sets the pixels in the last digit to the new color,
    // shifting all the digits to the left by one.
    void shiftDigitsLeft(uint32_t newColor);

    // Sets the pixels in the bottom row to the new color,
    // shifting all the rows up by one.
    void shiftRowsUp(uint32_t newColor);

    // Sets the pixels in the top row to the new color,
    // shifting all the rows down by one.
    void shiftRowsDown(uint32_t newColor);

    // Gets the number of pixels in the buffer.
    unsigned int getPixelCount();

    // Gets the number of columns in the buffer.
    // Might not be needed?
    unsigned int getColumnCount();

    // Gets the number of Rows in the buffer.
    // Might not be needed?
    unsigned int getRowCount();

    // Gets the number of Digits in the buffer.
    // Might not be needed? Even if needed, it should always be 4 anyways.
    unsigned int getDigitCount();

    // Set an individual pixel in the buffer to a color.
    void setPixel(unsigned int pixel, uint32_t color);

    // Output the interal pixel buffer to the NeoPixel LEDs.
    void displayPixels();

    // Clears the internal pixel buffer, but does not reset the NeoPixel LEDs.
    void clearBuffer();

  private:
    Adafruit_NeoPixel* m_neoPixels;
    unsigned int m_numPixels;
    uint32_t* m_pixelColors;
    std::vector<std::vector<int>*> m_columns;
    std::vector<std::vector<int>*> m_rows;
    std::vector<std::vector<int>*> m_digits;

    void initializeSignBuffer(int16_t gpioPin);
    void initializeTestRingBuffer(int16_t gpioPin);
    void setColorForMappedPixels(std::vector<int>* destination, uint32_t newColor);
    void shiftPixelBlocksRight(std::vector<std::vector<int>*> pixelBlocks, uint32_t newColor);
    void shiftPixelBlocksLeft(std::vector<std::vector<int>*> pixelBlocks, uint32_t newColor);
};

#endif