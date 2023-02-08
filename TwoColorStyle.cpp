#include "Arduino.h"
#include "TwoColorStyle.h"
#include "PixelBuffer.h"

TwoColorStyle::TwoColorStyle(String name, uint32_t color1, uint32_t color2, PixelBuffer* pixelBuffer) : LightStyle(name, pixelBuffer) {
  m_color1 = color1;
  m_color2 = color2;
  m_iterationCount = 0;
  m_nextUpdate = 0;
}

void TwoColorStyle::update() {
  if (millis() < m_nextUpdate) {
    return;
  }

  uint32_t primaryColor = m_color1;
  uint32_t secondaryColor = m_color2;

  if (m_step > 50) {
    primaryColor = m_color2;
    secondaryColor = m_color1;
  }
  int mod = getModulus();
  uint32_t newColor = primaryColor;
  if (m_iterationCount % mod == 0) {
    newColor = secondaryColor;
  }

  shiftColorUsingPattern(newColor);
  m_iterationCount++;
  m_nextUpdate = millis() + getIterationDelay();
}

void TwoColorStyle::reset()
{
  uint32_t primaryColor = m_color1;
  uint32_t secondaryColor = m_color2;
  int mod = getModulus();

  if (m_step > 50) {
    primaryColor = m_color2;
    secondaryColor = m_color1;
  }

  int numBlocks = getNumberOfBlocksForPattern();
  if (numBlocks > 50) {
    // The only patterns with more than 50 blocks are the line patterns.
    // Instead of shifting tons of times, just set the pixels directly.
    for (int i = 0; i < numBlocks; i++) {
      if (i % mod == 0) {
        m_pixelBuffer->setPixel(i, secondaryColor);
      } else {
        m_pixelBuffer->setPixel(i, primaryColor);
      }
    }
    return;
  }

  for (int i = 0; i < numBlocks; i++) {
    if (i % mod == 0) {
      shiftColorUsingPattern(secondaryColor);
    } else {
      shiftColorUsingPattern(primaryColor);
    }
  }
}

int TwoColorStyle::getIterationDelay() {
  // Convert "speed" to a delay.
  // Speed ranges from 1 to 100.
  int minDelay = 20;
  int maxDelay = 1000;
  double m = (maxDelay - minDelay)/-99.0;
  double b = maxDelay - m;
  int delay = m_speed*m + b;
  return delay;
}

int TwoColorStyle::getModulus() {
  // Convert "step" to a modulus -- every "modulus" pixel will be the secondary color.
  // Step ranges from 1 to 100.
  // The modulus will be the minumum (2) at 50, and increase as you go away from 50.
  int minMod = 2;
  int maxMod = 10;
  int x = abs(m_step - 50);
  double m = (maxMod - minMod)/50.0;
  double b = minMod;
  int mod = x*m + b;
  return mod;
}


