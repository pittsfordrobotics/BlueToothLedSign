#include "Arduino.h"
#include "SingleColorStyle.h"

SingleColorStyle::SingleColorStyle(String name, uint32_t color, uint32_t* colors, int numPixels) : LightStyle(name, colors, numPixels) {
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

  shiftColorsRight(newColor);
  m_iterationCount++;
  m_nextUpdate = millis() + getIterationDelay();
}

void SingleColorStyle::reset()
{
  int mod = getModulus();
  for (int i = 0; i < m_numPixels; i++) {
    if (i % mod == 0) {
      m_colors[i] = 0;
    } else {
      m_colors[i] = m_color;
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


