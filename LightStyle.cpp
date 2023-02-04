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
