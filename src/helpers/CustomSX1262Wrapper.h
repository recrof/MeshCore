#pragma once

#include "CustomSX1262.h"
#include "RadioLibWrappers.h"
#include <math.h>

#ifndef LORA_CR
  #define LORA_CR      5
#endif

#ifndef SX126X_CURRENT_LIMIT
  #define SX126X_CURRENT_LIMIT 140
#endif

#ifndef SX126X_RX_BOOSTED_GAIN
  #define SX126X_RX_BOOSTED_GAIN true
#endif

#ifndef LORA_TX_POWER
  #define LORA_TX_POWER 22
#endif

class CustomSX1262Wrapper : public RadioLibWrapper {
public:
  CustomSX1262Wrapper(CustomSX1262& radio, mesh::MainBoard& board) : RadioLibWrapper(radio, board) { }

  bool init() override {
    #ifdef SX126X_DIO3_TCXO_VOLTAGE
      float tcxo = SX126X_DIO3_TCXO_VOLTAGE;
    #else
      float tcxo = 1.6f;
    #endif

      SPI.setPins(P_LORA_MISO, P_LORA_SCLK, P_LORA_MOSI);
      SPI.begin();
      int status = ((CustomSX1262 *)_radio)->begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, LORA_TX_POWER, 8, tcxo);
      if (status != RADIOLIB_ERR_NONE) {
        Serial.print("ERROR: radio init failed: ");
        Serial.println(status);
        return false;  // fail
      }

      ((CustomSX1262 *)_radio)->setCRC(1);

    #ifdef SX126X_CURRENT_LIMIT
      ((CustomSX1262 *)_radio)->setCurrentLimit(SX126X_CURRENT_LIMIT);
    #endif

    #ifdef SX126X_DIO2_AS_RF_SWITCH
      ((CustomSX1262 *)_radio)->setDio2AsRfSwitch(SX126X_DIO2_AS_RF_SWITCH);
    #endif

    #ifdef SX126X_RX_BOOSTED_GAIN
      int16_t srbgm_result = ((CustomSX1262 *)_radio)->setRxBoostedGainMode(SX126X_RX_BOOSTED_GAIN, true);
      MESH_DEBUG_PRINTLN("BOOSTED_GAIN_RESULT: %d", srbgm_result);
    #endif

      return true;
    }

    bool isReceiving() override {
    if (((CustomSX1262 *)_radio)->isReceiving()) return true;

    idle();  // put sx126x into standby
    // do some basic CAD (blocks for ~12780 micros (on SF 10)!)
    bool activity = (((CustomSX1262 *)_radio)->scanChannel() == RADIOLIB_LORA_DETECTED);
    if (activity) {
      startRecv();
    } else {
      idle();
    }
    return activity;
  }
  float getLastRSSI() const override { return ((CustomSX1262 *)_radio)->getRSSI(); }
  float getLastSNR() const override { return ((CustomSX1262 *)_radio)->getSNR(); }

  float packetScore(float snr, int packet_len) override {
    int sf = ((CustomSX1262 *)_radio)->spreadingFactor;
    return packetScoreInt(snr, sf, packet_len);
  }
};
