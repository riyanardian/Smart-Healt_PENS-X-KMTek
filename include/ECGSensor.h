#ifndef ECG_SENSOR_H
#define ECG_SENSOR_H

#include <Arduino.h>
#include "Config.h"

class ECGSensor {
public:
    // Constructor hanya dengan output pin
    ECGSensor(int outputPin = ECG_OUTPUT);
    
    void begin();
    
    // Baca data ECG
    int readRawECG();
    float getECGVoltage();
    float getFilteredValue() { return filteredValue; }
    
    // Deteksi detak jantung
    bool detectHeartbeat();
    int getHeartRateBPM();
    int getCurrentBPM() { return BPM; }
    
    // Status (sederhana)
    bool isSensorAvailable() { return sensorAvailable; }
    String getLeadOffStatus() { return "Leads OK"; }  // Selalu OK
    
    // Filtering
    void updateFilter();
    
    // Konfigurasi threshold
    void setUpperThreshold(int threshold) { UpperThreshold = threshold; }
    int getUpperThreshold() { return UpperThreshold; }
    
    // BARU: Fungsi untuk grafik
    int* getECGBuffer();
    int getECGBufferSize();
    
private:
    int outputPin;
    bool sensorAvailable = false;
    
    // Parameter deteksi detak
    int UpperThreshold = 2500;   // Ambang batas, bisa disesuaikan
    const int RefractoryPeriod = 300;  // Periode refractory (ms)
    
    // Variabel untuk deteksi detak
    unsigned long lastBeatTime = 0;
    int BPM = 0;
    unsigned long previousBeatTime = 0; // Untuk menghitung interval
    
    // Filtering - Exponential Moving Average
    float filteredValue = 0.0;
    const float alpha = 0.1;  // Faktor smoothing
    
    // BARU: Buffer untuk grafik
    static const int ECG_BUFFER_SIZE = 200;
    int ecgBuffer[ECG_BUFFER_SIZE];
    int bufferIndex = 0;
    
    // Helper functions
    bool checkSensorConnection();
    void updateECGBuffer(int value);  // Fungsi baru
};

#endif