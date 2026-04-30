#ifndef MEDICAL_RECORD_SENSOR_H
#define MEDICAL_RECORD_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "MAX30100_PulseOximeter.h"
#include "Config.h"

class MedicalRecordSensor {
public:
    // Constructor
    MedicalRecordSensor();
    
    // Inisialisasi semua sensor
    void begin();
    
    // --- FUNGSI EDA SENSOR ---
    void calibrateEDA();
    int readEDARaw();
    String getStressLevel(int rawValue);
    float getEDAConductance(int rawValue);
    
    // --- FUNGSI MAX30100 (Heart Rate & SpO2) ---
    bool isMAX30100Available();
    float getHeartRate();
    int getSpO2();
    void startHRMeasurement();
    void stopHRMeasurement();
    bool isHRMeasuring();
    void sleepMAX30100();
    void wakeUpMAX30100();
    bool isMAX30100Sleeping() { return max30100Sleeping; }
    void restartI2C();
    void softResetMAX30100();
    String getMAX30100Status();
    
    // --- FUNGSI MLX90614 (Temperature) ---
    bool isMLX90614Available();
    float readObjectTemp();    // Suhu tubuh
    float readAmbientTemp();   // Suhu ruangan
    void wakeUpMLX90614();
    void sleepMLX90614();
    bool isMLX90614Sleeping() { return mlx90614Sleeping; }
    String getMLX90614Status();
    
    // --- FUNGSI KOMBINASI ---
    String getAllSensorsStatus();
    void wakeAllSensors();
    void sleepAllSensors();
    
    // Callback untuk detak jantung
    static void onBeatDetected();

private:
    // Sensor objects
    PulseOximeter pox;           // MAX30100
    Adafruit_MLX90614 mlx;       // MLX90614
    
    // Status flags
    bool edaCalibrated = false;
    float edaBaseline = 0.0;
    
    bool max30100Available = false;
    bool max30100Measuring = false;
    bool max30100Sleeping = true;
    
    bool mlx90614Available = false;
    bool mlx90614Sleeping = false;
    
    uint32_t lastMAX30100Report = 0;
    unsigned long lastMLXReadTime = 0;
    
    // Multitasking untuk MAX30100
    TaskHandle_t sensorTaskHandle = NULL;
    static void max30100Task(void * parameter);
    
    // Helper functions
    void ensureI2CForMLX90614();
};

#endif