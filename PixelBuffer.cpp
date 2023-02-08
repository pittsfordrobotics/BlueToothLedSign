#include <Adafruit_NeoPixel.h>
#include <vector>
#include "Arduino.h"
#include "PixelBuffer.h"

PixelBuffer::PixelBuffer(int16_t gpioPin) {
  m_numPixels = 12;
  m_pixelBuffer = new uint32_t[m_numPixels];
  m_neoPixels = new Adafruit_NeoPixel(m_numPixels, gpioPin, NEO_GRB + NEO_KHZ800);
  initializeMatrices();
}

void PixelBuffer::clearBuffer() {
  for (int i = 0; i < m_numPixels; i++) {
    m_pixelBuffer[i] = 0;
  }
}

void PixelBuffer::displayPixels() {
  for (int i = 0; i < m_numPixels; i++)
  {
    m_neoPixels->setPixelColor(i, m_pixelBuffer[i]);
  }

  m_neoPixels->show();
}

unsigned int PixelBuffer::getColumnCount() {
  return m_columns.size();
}

unsigned int PixelBuffer::getRowCount() {
  return m_rows.size();
}

unsigned int PixelBuffer::getDigitCount() {
  return m_digits.size();
}

unsigned int PixelBuffer::getPixelCount() {
  return m_numPixels;
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
  if (pixel >= m_numPixels) {
    return;
  }

  m_pixelBuffer[pixel] = color;
}

void PixelBuffer::shiftLineRight(uint32_t newColor)
{
  for (int i = m_numPixels - 1; i >= 1; i--)
  {
    m_pixelBuffer[i] = m_pixelBuffer[i - 1];
  }

  m_pixelBuffer[0] = newColor;
}

void PixelBuffer::shiftLineLeft(uint32_t newColor)
{
  for (int i = 0; i < m_numPixels - 1; i++)
  {
    m_pixelBuffer[i] = m_pixelBuffer[i + 1];
  }

  m_pixelBuffer[m_numPixels - 1] = newColor;
}

void PixelBuffer::shiftColumnsRight(uint32_t newColor)
{
  shiftPixelBlocksRight(m_columns, newColor);
}

void PixelBuffer::shiftColumnsLeft(uint32_t newColor)
{
  shiftPixelBlocksLeft(m_columns, newColor);
}

void PixelBuffer::shiftDigitsRight(uint32_t newColor)
{
  shiftPixelBlocksRight(m_digits, newColor);
}

void PixelBuffer::shiftDigitsLeft(uint32_t newColor)
{
  shiftPixelBlocksLeft(m_digits, newColor);
}

void PixelBuffer::shiftRowsUp(uint32_t newColor)
{
  shiftPixelBlocksLeft(m_rows, newColor);
}

void PixelBuffer::shiftRowsDown(uint32_t newColor)
{
  shiftPixelBlocksRight(m_rows, newColor);
}

void PixelBuffer::shiftPixelBlocksRight(std::vector<std::vector<int>*> pixelBlocks, uint32_t newColor) {
  for (int i = pixelBlocks.size() - 1; i >= 1; i--) {
    std::vector<int>* source = pixelBlocks.at(i - 1);
    std::vector<int>* destination = pixelBlocks.at(i);
    // Find the color of the first pixel in the source column, and set the destination column to that color.
    uint32_t previousColor = m_pixelBuffer[source->at(0)];
    setColorForMappedPixels(destination, previousColor);
  }

  setColorForMappedPixels(pixelBlocks.at(0), newColor);
}

void PixelBuffer::shiftPixelBlocksLeft(std::vector<std::vector<int>*> pixelBlocks, uint32_t newColor) {
  for (int i = 0; i < pixelBlocks.size() - 1; i++) {
    std::vector<int>* source = pixelBlocks.at(i + 1);
    std::vector<int>* destination = pixelBlocks.at(i);
    // Find the color of the first pixel in the source column, and set the destination column to that color.
    uint32_t previousColor = m_pixelBuffer[source->at(0)];
    setColorForMappedPixels(destination, previousColor);
  }

  setColorForMappedPixels(pixelBlocks.at(pixelBlocks.size() - 1), newColor);
}

void PixelBuffer::setColorForMappedPixels(std::vector<int>* destination, uint32_t newColor) {
  for (int i = 0; i < destination->size(); i++) {
    int pixelIndex = destination->at(i);
    Serial.print("Setting mapped block index ");
    Serial.print(i);
    Serial.print(" (pixel index ");
    Serial.print(pixelIndex);
    Serial.print(") to color ");
    Serial.println(newColor, HEX);
    m_pixelBuffer[pixelIndex] = newColor;
  }
}

void PixelBuffer::initializeMatrices() {
  // Map the pixel indices to rows, columns, and digits.
  // ROW 0 is at the TOP of the display.
  // COLUMN 0 is at the LEFT of the display.
  // DIGIT 0 is at the LEFT of the display.

  // Set up the rows/columns/digits as small pieces of the NeoPixel ring
  m_columns.push_back(new std::vector<int>{0});
  m_columns.push_back(new std::vector<int>{1, 11});
  m_columns.push_back(new std::vector<int>{2, 10});
  m_columns.push_back(new std::vector<int>{3, 9});
  m_columns.push_back(new std::vector<int>{4, 8});
  m_columns.push_back(new std::vector<int>{5, 7});
  m_columns.push_back(new std::vector<int>{6});

  m_rows.push_back(new std::vector<int>{3});
  m_rows.push_back(new std::vector<int>{2, 4});
  m_rows.push_back(new std::vector<int>{1, 5});
  m_rows.push_back(new std::vector<int>{0, 6});
  m_rows.push_back(new std::vector<int>{11, 7});
  m_rows.push_back(new std::vector<int>{10, 8});
  m_rows.push_back(new std::vector<int>{9});

  m_digits.push_back(new std::vector<int>{0, 1, 11});
  m_digits.push_back(new std::vector<int>{2, 3, 4});
  m_digits.push_back(new std::vector<int>{5, 6, 7});
  m_digits.push_back(new std::vector<int>{8, 9, 10});
}
