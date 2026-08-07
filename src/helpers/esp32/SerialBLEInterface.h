#pragma once

#include "../BaseSerialInterface.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// arduino-esp32 3.3 (pioarduino platform 55.x, ESP-IDF 5.5) merged NimBLE into the BLE library,
// and dropped some of the old Bluedroid-only helpers like BLESecurity::setStaticPIN().
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)
  #define BLE_UNIFIED_API  1
#endif

// On chips with no Bluedroid stack (C6/H2/C5) that library binds to NimBLE, which uses
// different callback signatures and has no per-characteristic access permissions.
#if defined(BLE_UNIFIED_API) && defined(CONFIG_NIMBLE_ENABLED)
  #define BLE_NIMBLE_API  1
#endif

class SerialBLEInterface : public BaseSerialInterface, BLESecurityCallbacks, BLEServerCallbacks, BLECharacteristicCallbacks {
  BLEServer *pServer;
  BLEService *pService;
  BLECharacteristic * pTxCharacteristic;
  bool deviceConnected;
  bool oldDeviceConnected;
  bool _isEnabled;
#ifdef BLE_NIMBLE_API
  bool _peer_authenticated;
#endif
  uint16_t last_conn_id;
  uint32_t _pin_code;
  unsigned long _last_write;
  unsigned long adv_restart_time;

  struct Frame {
    uint8_t len;
    uint8_t buf[MAX_FRAME_SIZE];
  };

  #define FRAME_QUEUE_SIZE  4
  StaticQueue_t recv_queue_state;
  uint8_t recv_queue_storage[FRAME_QUEUE_SIZE * sizeof(Frame)];
  QueueHandle_t recv_queue;
  int send_queue_len;
  Frame send_queue[FRAME_QUEUE_SIZE];

  void clearBuffers();

protected:
  // BLESecurityCallbacks methods
  uint32_t onPassKeyRequest() override;
  void onPassKeyNotify(uint32_t pass_key) override;
  bool onConfirmPIN(uint32_t pass_key) override;
  bool onSecurityRequest() override;
#ifdef BLE_NIMBLE_API
  void onAuthenticationComplete(ble_gap_conn_desc* desc) override;
#else
  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) override;
#endif

  // BLEServerCallbacks methods
  void onConnect(BLEServer* pServer) override;
  void onDisconnect(BLEServer* pServer) override;
#ifdef BLE_NIMBLE_API
  void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) override;
  void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* desc) override;
  void onMtuChanged(BLEServer* pServer, ble_gap_conn_desc* desc, uint16_t mtu) override;
#else
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;
  void onMtuChanged(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) override;
#endif

  // BLECharacteristicCallbacks methods
#ifdef BLE_NIMBLE_API
  void onWrite(BLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override;
#else
  void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) override;
#endif

public:
  SerialBLEInterface() {
    pServer = NULL;
    pService = NULL;
    deviceConnected = false;
    oldDeviceConnected = false;
#ifdef BLE_NIMBLE_API
    _peer_authenticated = false;
#endif
    adv_restart_time = 0;
    _isEnabled = false;
    _last_write = 0;
    last_conn_id = 0;
    recv_queue = xQueueCreateStatic(
      FRAME_QUEUE_SIZE, sizeof(Frame), recv_queue_storage, &recv_queue_state
    );
    send_queue_len = 0;
  }

  /**
   * init the BLE interface.
   * @param prefix   a prefix for the device name
   * @param name  IN/OUT - a name for the device (combined with prefix). If "@@MAC", is modified and returned
   * @param pin_code   the BLE security pin
   */
  void begin(const char* prefix, char* name, uint32_t pin_code);

  // BaseSerialInterface methods
  void enable() override;
  void disable() override;
  bool isEnabled() const override { return _isEnabled; }

  bool isConnected() const override;

  bool isWriteBusy() const override;
  size_t writeFrame(const uint8_t src[], size_t len) override;
  size_t checkRecvFrame(uint8_t dest[]) override;
};

#if BLE_DEBUG_LOGGING && ARDUINO
  #include <Arduino.h>
  #define BLE_DEBUG_PRINT(F, ...) Serial.printf("BLE: " F, ##__VA_ARGS__)
  #define BLE_DEBUG_PRINTLN(F, ...) Serial.printf("BLE: " F "\n", ##__VA_ARGS__)
#else
  #define BLE_DEBUG_PRINT(...) {}
  #define BLE_DEBUG_PRINTLN(...) {}
#endif
