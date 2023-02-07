#import "Arduino.h"
#import "LightStyle.h"
#import "PixelBuffer.h"

std::vector<String> LightStyle::knownPatterns;

LightStyle::LightStyle(String name, PixelBuffer* pixelBuffer) {
  m_pixelBuffer = pixelBuffer;
  m_name = name;
  knownPatterns.push_back("Solid");
  knownPatterns.push_back("ColumnRight");
  knownPatterns.push_back("ColumnLeft");
  knownPatterns.push_back("RowUp");
  knownPatterns.push_back("RowDown");
  knownPatterns.push_back("DigitRight");
  knownPatterns.push_back("DigitLeft");
  knownPatterns.push_back("LineRight");
  knownPatterns.push_back("LineLeft");
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

int LightStyle::getNumberOfBlocksForPattern() {
  switch (m_pattern) {
    case 1:
    case 2:
      return m_pixelBuffer->getColumnCount();
    case 3:
    case 4:
      return m_pixelBuffer->getRowCount();
    case 5:
    case 6:
      return m_pixelBuffer->getDigitCount();
    case 7:
    case 8:
      return m_pixelBuffer->getPixelCount();
    default:
      // Default to Solid (ie, all lights the same color)
      return 1;
  }
}

void LightStyle::shiftColorUsingPattern(uint32_t newColor) {
  // Stick with integer values here instead of doing a bunch
  // of "ifs" to compare strings.
  switch (m_pattern) {
    case 1:
      m_pixelBuffer->shiftColumnsRight(newColor);
      break;
    case 2:
      m_pixelBuffer->shiftColumnsLeft(newColor);
      break;
    case 3:
      m_pixelBuffer->shiftRowsUp(newColor);
      break;
    case 4:
      m_pixelBuffer->shiftRowsDown(newColor);
      break;
    case 5:
      m_pixelBuffer->shiftDigitsRight(newColor);
      break;
    case 6:
      m_pixelBuffer->shiftDigitsLeft(newColor);
      break;
    case 7:
      m_pixelBuffer->shiftLineRight(newColor);
      break;
    case 8:
      m_pixelBuffer->shiftLineLeft(newColor);
      break;
    default:
      // Default to Solid (ie, all lights the same color)
      for (int i = 0; i < m_pixelBuffer->getPixelCount(); i++) {
        m_pixelBuffer->setPixel(i, newColor);
      }
  }
}

