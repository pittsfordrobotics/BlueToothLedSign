#include <Adafruit_NeoPixel.h>
#include "Arduino.h"
#include "RainbowStyle.h"
#include "PixelBuffer.h"

RainbowStyle::RainbowStyle(String name, PixelBuffer* pixelBuffer) : LightStyle(name, pixelBuffer) {
  m_nextUpdate = 0;
  m_currentHue = 0;
}

void RainbowStyle::update() {
  if (millis() < m_nextUpdate) {
    return;
  }

  uint32_t newColor = Adafruit_NeoPixel::ColorHSV(m_currentHue);
  shiftColorUsingPattern(newColor);
  incrementHue();
  m_nextUpdate = millis() + getIterationDelay();
}

void RainbowStyle::reset()
{
   int numBlocks = getNumberOfBlocksForPattern();
  if (numBlocks > 100) {
    // The only patterns with this many blocks are the line patterns.
    // Instead of shifting tons of times, just set the pixels directly.
    for (int i = numBlocks - 1; i >= 0; i--) {
      m_pixelBuffer->setPixel(i, Adafruit_NeoPixel::ColorHSV(m_currentHue));
      incrementHue();
    }

    return;
  }

  for (int i = 0; i < numBlocks - 1; i++) {
    shiftColorUsingPattern(Adafruit_NeoPixel::ColorHSV(m_currentHue));
    incrementHue();
  }
}

int RainbowStyle::getIterationDelay() {
  // Convert "speed" to a delay.
  // Speed ranges from 1 to 100.
  int minDelay = 5;
  int maxDelay = 500;
  double m = (maxDelay - minDelay)/-99.0;
  double b = maxDelay - m;
  int delay = m_speed*m + b;
  return delay;
}

void RainbowStyle::incrementHue() {
  // Convert "step" to an increment.
  int maxInc = 1000;
  int minInc = 5;
  int maxStep = 100;
  int minStep = 1;
  double m = (double)(maxInc - minInc)/(maxStep - minStep);
  double b = minInc - m;
  int inc = m_step*m + b;
  m_currentHue += inc;
}

