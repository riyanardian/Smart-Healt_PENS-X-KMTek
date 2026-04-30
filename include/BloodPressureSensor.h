#ifndef BLOOD_PRESSURE_SENSOR_H
#define BLOOD_PRESSURE_SENSOR_H

#include <Arduino.h>
#include "Config.h"

class BloodPressureSensor {
public:
    void begin();
    void update(); // Membaca data dari Serial2 dan memprosesnya

    int getSystolic();   // Sistolik (mmHg)
    int getDiastolic();  // Diastolik (mmHg)
    int getBPM();        // Denyut nadi (bpm)
    
    bool isDataAvailable(); // Data baru tersedia
    void resetDataFlag();   // Reset flag data baru
    bool isMeasuring();     // Cek apakah sedang mengukur (selalu true di mode ini)
    bool hasValidData();    // Cek apakah ada data valid yang terbaca
    
    void startMeasurement(); // Mulai pengukuran (otomatis saat mode dipilih)
    void stopMeasurement();  // Stop pengukuran (saat keluar mode)

private:
    void processBuffer();
    int hexToDec(char c);

    int systolic = 0;
    int diastolic = 0;
    int bpm = 0;

    char rawBuffer[100];
    int bufferIndex = 0;
    char last5Chars[5];

    enum State {
        SEARCHING_HEADER,
        COLLECTING_DATA
    };
    State currentState = SEARCHING_HEADER;

    bool newDataAvailable = false;
    bool measuring = false;
    bool dataValid = false; // Flag untuk data valid
};

#endif