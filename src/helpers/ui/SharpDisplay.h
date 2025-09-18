#pragma once

#include "DisplayDriver.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SharpMem.h>

#define WHITE 255
#define GREY 64
#define BLACK 0

#ifndef DISPLAY_SCALE
#define DISPLAY_SCALE 1
#endif

const uint8_t bayer4x4[4][4] = {
  {  0, 128,  32, 160 },
  {192,  64, 224,  96 },
  { 48, 176,  16, 144 },
  {240, 112, 208,  80 }
};

class Adafruit_SharpMem_Dither : public Adafruit_SharpMem {
  uint8_t _scale;

  public:
    Adafruit_SharpMem_Dither(
      uint8_t clk,
      uint8_t mosi,
      uint8_t cs,
      uint16_t w,
      uint16_t h,
      uint8_t scale,
      uint32_t freq = 2000000
    ): Adafruit_SharpMem(clk, mosi, cs, w, h, freq), _scale(scale) {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
};

// Display driver for Sharp memory display
class SharpDisplay : public DisplayDriver {
  Adafruit_SharpMem_Dither display;
  bool _isOn;
  uint8_t _color;

public:
  SharpDisplay() : DisplayDriver(400 / DISPLAY_SCALE, 240 / DISPLAY_SCALE), display(PIN_TFT_SCL, PIN_TFT_SDA, PIN_TFT_CS, 400, 240, DISPLAY_SCALE)
  {
    _isOn = false;
  }

  bool begin();
  bool isOn() override { return _isOn; }
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(Color bkg = DARK) override;
  void setTextSize(int sz) override;
  void setColor(Color c) override;
  void setCursor(int x, int y) override;
  void print(const char *str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t *bits, int w, int h) override;
  uint16_t getTextWidth(const char *str) override;
  void endFrame() override;
  void drawPixel(int16_t x, int16_t y, uint16_t color);
};