#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>

class XiaoC5Board : public ESP32Board {
public:
  const char* getManufacturerName() const override {
    return "Xiao C5";
  }
};
