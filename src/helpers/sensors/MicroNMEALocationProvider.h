#pragma once

#include "LocationProvider.h"
#include <MicroNMEA.h>
#include <RTClib.h>

#ifndef GPS_EN
#define GPS_EN (-1)
#endif

#ifndef GPS_RESET
#define GPS_RESET (-1)
#endif

#ifndef GPS_RESET_FORCE
#define GPS_RESET_FORCE LOW
#endif

class MicroNMEALocationProvider : public LocationProvider {
    char _nmeaBuffer[100];
    MicroNMEA nmea;
    mesh::RTCClock* _clock;
    Stream* _gps_serial;
    int _pin_reset;
    int _pin_en;
    long next_check = 0;
    long time_valid = 0;

public :
    MicroNMEALocationProvider(Stream& ser, mesh::RTCClock* clock = NULL, int pin_reset = GPS_RESET, int pin_en = GPS_EN) :
    _gps_serial(&ser), nmea(_nmeaBuffer, sizeof(_nmeaBuffer)), _pin_reset(pin_reset), _pin_en(pin_en), _clock(clock) {
        if (_pin_reset != -1) {
            pinMode(_pin_reset, OUTPUT);
            digitalWrite(_pin_reset, GPS_RESET_FORCE);
        }
        if (_pin_en != -1) {
            pinMode(_pin_en, OUTPUT);
            digitalWrite(_pin_en, LOW);
        }
    }

    void begin() override {
        if (_pin_reset != -1) {
            digitalWrite(_pin_reset, !GPS_RESET_FORCE);
        }
        if (_pin_en != -1) {
            digitalWrite(_pin_en, HIGH);
        }
    }

    void reset() override {
        if (_pin_reset != -1) {
            digitalWrite(_pin_reset, GPS_RESET_FORCE);
            delay(100);
            digitalWrite(_pin_reset, !GPS_RESET_FORCE);
        }
    }

    void stop() override {
        if (_pin_en != -1) {
            digitalWrite(_pin_en, LOW);
        }        
    }

    void syncTime() override { nmea.clear(); LocationProvider::syncTime(); }
    long getLatitude() override { return nmea.getLatitude(); }
    long getLongitude() override { return nmea.getLongitude(); }
    long getAltitude() override { 
        long alt = 0;
        nmea.getAltitude(alt);
        return alt;
    }
    long satellitesCount() override { return nmea.getNumSatellites(); }
    bool isValid() override { return nmea.isValid(); }

    long getTimestamp() override { 
        DateTime dt(nmea.getYear(), nmea.getMonth(),nmea.getDay(),nmea.getHour(),nmea.getMinute(),nmea.getSecond());
        return dt.unixtime();
    } 

    void sendSentence(const char *sentence) override {
        nmea.sendSentence(*_gps_serial, sentence);
    }

    void loop() override {

        while (_gps_serial->available()) {
            char c = _gps_serial->read();
            #ifdef GPS_NMEA_DEBUG
            Serial.print(c);
            #endif
            nmea.process(c);
        }

        if (!isValid()) time_valid = 0;

        if (millis() > next_check) {
            next_check = millis() + 1000;
            if (_time_sync_needed && time_valid > 2) {
                if (_clock != NULL) {
                    _clock->setCurrentTime(getTimestamp());
                    _time_sync_needed = false;
                }
            }
            if (isValid()) {
                time_valid ++;
            }
        }
    }
};