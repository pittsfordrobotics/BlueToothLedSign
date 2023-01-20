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

  uint32_t newColor = m_color;
  if (m_iterationCount % 3 == 0) {
    newColor = 0;
  }

  shiftColorsRight(newColor);
  m_iterationCount++;
  m_nextUpdate = millis() + getIterationDelay();
}

void SingleColorStyle::reset()
{
  // Set every 3rd pixel off
  for (int i = 0; i < m_numPixels; i++) {
    if (i % 3 == 0) {
      m_colors[i] = 0;
    } else {
      m_colors[i] = m_color;
    }
  }
}

int SingleColorStyle::getIterationDelay() {
  // Convert "speed" to a delay.
  // Speed ranges from 1 to 100.
  // Convert that to a delay of about 100ms to 1000ms.
  int delay = -9.0*m_speed + 1009;
  return delay;
}


