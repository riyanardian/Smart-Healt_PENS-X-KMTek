// BodyPositionSnoreSensor.h
#ifndef BODY_POSITION_SNORE_SENSOR_H
#define BODY_POSITION_SNORE_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

class BodyPositionSnoreSensor {
public:
    void begin();
    void update();
    
    // Getter untuk posisi dan dengkur
    int getPosition() { return lastPosisi; }
    int getSnoreAmplitude() { return lastAmplitude; }
    String getPositionName();
    
    // Status sensor
    bool isMPU6050Available() { return mpuAvailable; }
    // Hapus isCalibrated() karena tidak ada kalibrasi
    
private:
    // MPU6050
    uint8_t mpuAddr = 0x68;
    static constexpr float ACC_SCALE = 16384.0f;
    
    // Kalibrasi MPU6050 (opsional)
    float accOffsetX = 0, accOffsetY = 0, accOffsetZ = 0;
    float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;
    
    // Variabel pembacaan MPU6050
    int16_t rax, ray, raz, rgx, rgy, rgz;
    
    // Sound sensor
    static constexpr int SOUND_PIN = 34; // GPIO34 (ADC1_CH6)
    
    // Variabel untuk posisi tubuh
    static constexpr int POS_TRANSISI = 0;
    static constexpr int POS_TEGAK = 1;
    static constexpr int POS_DUDUK = 2;
    static constexpr int POS_TELENTANG = 3;
    static constexpr int POS_TENGKURAP = 4;
    static constexpr int POS_MIRING_KANAN = 5;
    static constexpr int POS_MIRING_KIRI = 6;
    
    int lastPosisi = POS_TRANSISI;
    int lastAmplitude = 0; // Amplitudo mentah
    
    // Status sensor
    bool mpuAvailable = false;
    
    // Fungsi internal
    bool initMPU6050();
    void resetMPU6050();
    void calibrateMPU6050(int samples);
    bool readMPU6050();
    
    // Fungsi untuk sound sensor - SAMA PERSIS dengan kode Anda
    int readAmplitude();
};

#endif