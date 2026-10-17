#include <Arduino.h>
#include <Wire.h>

#include "T1000eBoard.h"

void T1000eBoard::begin() {
  NRF52BoardDCDC::begin();
  btn_prev_state = HIGH;

#ifdef BUTTON_PIN
  pinMode(BATTERY_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
#endif

#if defined(PIN_BOARD_SDA) && defined(PIN_BOARD_SCL)
  Wire.setPins(PIN_BOARD_SDA, PIN_BOARD_SCL);
#endif

  Wire.begin();

  delay(10);   // give sx1262 some time to power up
}

void T1000eBoard::attachDynamicPrefs(KeyValueStore* prefs) {
  _prefs = prefs;
#ifdef PIN_BUZZER_EN
  char val[8];
  val[0] = 0;
  _prefs->getByKey("buzzer", val, sizeof(val) - 1);
  _buzzer_muted = strcmp(val, "0") == 0;
#endif
}

void T1000eBoard::setBuzzerEnabled(bool enable) {
#ifdef PIN_BUZZER_EN
  _buzzer_muted = !enable;
  digitalWrite(PIN_BUZZER_EN, enable ? HIGH : LOW);
#endif
}

void T1000eBoard::loop() {
  NRF52BoardDCDC::loop();
#ifdef PIN_BUZZER_EN
  if (_buzzer_muted) digitalWrite(PIN_BUZZER_EN, LOW);  // keep it off even if the UI re-enables it
#endif
}

bool T1000eBoard::handleCommand(const char* command, uint32_t sender_timestamp, char* reply) {
#ifdef PIN_BUZZER_EN
  if (strcmp(command, "get buzzer") == 0) {
    sprintf(reply, "> %s", _buzzer_muted ? "off" : "on");
    return true;
  }
  if (memcmp(command, "set buzzer ", 11) == 0) {
    if (strcmp(&command[11], "on") == 0) {
      if (_prefs && !_prefs->setByKey("buzzer", "1")) {
        strcpy(reply, "Error: failed to save buzzer setting");
      } else {
        setBuzzerEnabled(true);
        strcpy(reply, "OK - buzzer on");
      }
    } else if (strcmp(&command[11], "off") == 0) {
      if (_prefs && !_prefs->setByKey("buzzer", "0")) {
        strcpy(reply, "Error: failed to save buzzer setting");
      } else {
        setBuzzerEnabled(false);
        strcpy(reply, "OK - buzzer off");
      }
    } else {
      strcpy(reply, "Error: state must be on or off");
    }
    return true;
  }
#endif
  return false; // not handled
}
