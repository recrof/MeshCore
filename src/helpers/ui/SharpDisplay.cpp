#include "SharpDisplay.h"

#include "../../MeshCore.h"

bool SharpDisplay::begin() {
  display.begin();
  display.setRotation(2);
  _isOn = true;

  clear();

  return true;
}

void SharpDisplay::turnOn() {
  begin();
  _isOn = true;
}

void SharpDisplay::turnOff() {
  _isOn = false;
}

void SharpDisplay::clear() {
  display.clearDisplay();
}

void SharpDisplay::startFrame(Color bkg) {
  _color = WHITE;

  display.fillRect(0, 0, width(), height(), _color);
  display.setTextColor(_color);
  display.setTextSize(1);
  display.cp437(true);
}

void SharpDisplay::setTextSize(int sz) {
  display.setTextSize(sz);
}

void SharpDisplay::setColor(Color c) {
  if (c == LIGHT)
    _color = WHITE;
  else if (c == YELLOW)
    _color = GREY;
  else
    _color = BLACK;

  display.setTextColor(_color);
}

void SharpDisplay::setCursor(int x, int y) {
  display.setCursor(x, y);
}

void SharpDisplay::print(const char *str) {
  display.print(str);
}

void SharpDisplay::fillRect(int x, int y, int w, int h) {
  display.fillRect(x, y, w, h, _color);
}

void SharpDisplay::drawRect(int x, int y, int w, int h) {
  display.drawRect(x, y, w, h, _color);
}

void SharpDisplay::drawXbm(int x, int y, const uint8_t *bits, int w, int h) {
  display.drawBitmap(x, y, bits, w, h, _color);
}

uint16_t SharpDisplay::getTextWidth(const char *str) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return w;
}

void SharpDisplay::endFrame() {
  display.refresh();
}

void Adafruit_SharpMem_Dither::drawPixel(int16_t x, int16_t y, uint16_t color) {
  for (uint8_t i = 0; i < _scale; i++)
  {
    for (uint8_t j = 0; j < _scale; j++)
    {
      int16_t scaled_x = x * _scale + i;
      int16_t scaled_y = y * _scale + j;

      uint8_t threshold = bayer4x4[scaled_y % 4][scaled_x % 4];
      Adafruit_SharpMem::drawPixel(scaled_x, scaled_y, color > threshold);
    }
  }
}
