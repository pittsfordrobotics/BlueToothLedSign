#import "Arduino.h"
#import "LightStyle.h"
#import "PixelBuffer.h"

LightStyle::LightStyle(String name, PixelBuffer* pixelBuffer) {
  m_pixelBuffer = pixelBuffer;
  m_name = name;
}

void LightStyle::setSpeed(uint8_t speed) {
  m_speed = speed;
}

void LightStyle::setStep(byte step) {
  m_step = step;
}

String LightStyle::getName() {
  return m_name;
}

void LightStyle::setPattern(byte pattern) {
  m_pattern = pattern;
}

void LightStyle::shiftColorUsingPattern(uint32_t newColor) {
  switch (m_pattern) {
    case LIGHT_PATTERN_COLUMN_RIGHT:
      m_pixelBuffer->shiftColumnsRight(newColor);
      break;
    case LIGHT_PATTERN_COLUMN_LEFT:
      m_pixelBuffer->shiftColumnsLeft(newColor);
      break;
    case LIGHT_PATTERN_ROW_UP:
      m_pixelBuffer->shiftRowsUp(newColor);
      break;
    case LIGHT_PATTERN_ROW_DOWN:
      m_pixelBuffer->shiftRowsDown(newColor);
      break;
    default:
      m_pixelBuffer->shiftLineRight(newColor);
  }
}

