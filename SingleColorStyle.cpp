#include "Arduino.h"
#include "SingleColorStyle.h"
#include "PixelBuffer.h"

SingleColorStyle::SingleColorStyle(String name, uint32_t color, PixelBuffer* pixelBuffer) : LightStyle(name, pixelBuffer) {
  m_color = color;
  m_iterationCount = 0;
  m_nextUpdate = 0;
}

void SingleColorStyle::update() {
  if (millis() < m_nextUpdate) {
    return;
  }

  int mod = getModulus();
  uint32_t newColor = m_color;
  if (m_iterationCount % mod == 0) {
    newColor = 0;
  }

  shiftColorUsingPattern(newColor);
  m_iterationCount++;
  m_nextUpdate = millis() + getIterationDelay();
}

void SingleColorStyle::reset()
{
  int mod = getModulus();
  int numBlocks = getNumberOfBlocksForPattern();
  if (numBlocks > 200) {
    // The only patterns with this many blocks are the line patterns.
    // Instead of shifting tons of times, just set the pixels directly.
    for (int i = 0; i < numBlocks; i++) {
      if (i % mod == 0) {
        m_pixelBuffer->setPixel(i, 0);
      } else {
        m_pixelBuffer->setPixel(i, m_color);
      }
    }
  }

  for (int i = 0; i < numBlocks; i++) {
    if (i % mod == 0) {
      shiftColorUsingPattern(0);
    } else {
      shiftColorUsingPattern(m_color);
    }
  }
}

int SingleColorStyle::getIterationDelay() {
  // Convert "speed" to a delay.
  // Speed ranges from 1 to 100.
  int minDelay = 20;
  int maxDelay = 1000;
  double m = (maxDelay - minDelay)/-99.0;
  double b = maxDelay - m;
  int delay = m_speed*m + b;
  return delay;
}

int SingleColorStyle::getModulus() {
  // Convert "step" to a modulus -- every "modulus" pixel will be off.
  // Step ranges from 1 to 100.
  int minMod = 2;
  int maxMod = 10;
  double m = (maxMod - minMod)/99.0;
  double b = minMod - m;
  int mod = m_step*m + b;
  return mod;
}


