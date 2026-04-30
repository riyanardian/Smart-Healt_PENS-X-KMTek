#include "ECGSensor.h"

// Constructor - HANYA SATU KALI
ECGSensor::ECGSensor(int outputPin) {
    this->outputPin = outputPin;
    // Inisialisasi buffer dengan 0
    for (int i = 0; i < ECG_BUFFER_SIZE; i++) {
        ecgBuffer[i] = 0;
    }
}

void ECGSensor::begin() {
    Serial.println("\n=== INISIALISASI ECG SENSOR (AD8232 SIMPLE) ===");
    
    // Setup pin
    pinMode(outputPin, INPUT);
    
    // Setup ADC untuk ECG
    analogReadResolution(12);
    analogSetPinAttenuation(outputPin, ADC_11db);
    
    // Inisialisasi nilai filter dengan pembacaan pertama
    filteredValue = analogRead(outputPin);
    
    // Check sensor connection
    sensorAvailable = checkSensorConnection();
    
    if (sensorAvailable) {
        Serial.println("✓ ECG Sensor (AD8232) siap digunakan");
        Serial.printf("  Pin: GPIO%d\n", outputPin);
        Serial.printf("  Threshold: %d, Refractory Period: %d ms\n", 
                     UpperThreshold, RefractoryPeriod);
        Serial.printf("  Filter Alpha: %.2f\n", alpha);
        
        // Tampilkan petunjuk untuk tuning
        Serial.println("  Tips: Gunakan Serial Plotter untuk tuning threshold");
        Serial.println("  Format: Raw:[value],Filter:[value],Threshold:[value]");
    } else {
        Serial.println("✗ ECG Sensor (AD8232) TIDAK terdeteksi!");
        Serial.println("  Periksa koneksi sensor dan pastikan:");
        Serial.println("  1. Sensor mendapat daya 3.3V");
        Serial.println("  2. Pin OUTPUT terhubung dengan benar");
    }
}

bool ECGSensor::checkSensorConnection() {
    // Baca beberapa sample untuk test
    int samples[10];
    int total = 0;
    
    for (int i = 0; i < 10; i++) {
        samples[i] = analogRead(outputPin);
        total += samples[i];
        delay(5);
    }
    
    int avg = total / 10;
    Serial.printf("  Test read ECG (10 samples): Avg=%d\n", avg);
    
    // Cek apakah pembacaan valid (bukan stuck di 0 atau 4095)
    bool stuck = true;
    for (int i = 1; i < 10; i++) {
        if (abs(samples[i] - samples[i-1]) > 10) {
            stuck = false;
            break;
        }
    }
    
    if (stuck || avg == 0 || avg == 4095) {
        Serial.println("  PERINGATAN: Pembacaan ECG tidak valid atau konstan!");
        return false;
    }
    
    // Tampilkan range untuk membantu tuning
    int minVal = 4095, maxVal = 0;
    for (int i = 0; i < 10; i++) {
        if (samples[i] < minVal) minVal = samples[i];
        if (samples[i] > maxVal) maxVal = samples[i];
    }
    Serial.printf("  Range: %d - %d\n", minVal, maxVal);
    Serial.printf("  Rekomendasi Threshold: %d - %d\n", 
                  avg + 100, avg + 500);
    
    return true;
}

// readRawECG - HANYA SATU KALI
int ECGSensor::readRawECG() {
    if (!sensorAvailable) return 0;
    int rawValue = analogRead(outputPin);
    updateECGBuffer(rawValue);  // Tambahkan ke buffer
    return rawValue;
}

void ECGSensor::updateECGBuffer(int value) {
    ecgBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % ECG_BUFFER_SIZE;
}

int* ECGSensor::getECGBuffer() {
    return ecgBuffer;
}

int ECGSensor::getECGBufferSize() {
    return ECG_BUFFER_SIZE;
}

float ECGSensor::getECGVoltage() {
    if (!sensorAvailable) return 0.0;
    int raw = readRawECG();
    return raw * (3.3 / 4095.0);
}

void ECGSensor::updateFilter() {
    if (!sensorAvailable) return;
    
    int rawValue = readRawECG();
    
    // Exponential Moving Average (EMA) filter
    // Rumus: (alpha * data_baru) + ((1 - alpha) * data_lama)
    filteredValue = (alpha * rawValue) + ((1.0 - alpha) * filteredValue);
}

bool ECGSensor::detectHeartbeat() {
    if (!sensorAvailable) return false;
    
    // Update filter terlebih dahulu
    updateFilter();
    
    unsigned long currentTime = millis();
    
    // Deteksi jika sinyal melebihi threshold DAN melewati refractory period
    if (filteredValue > UpperThreshold && 
        (currentTime - lastBeatTime > RefractoryPeriod)) {
        
        // Catat waktu detak ini
        lastBeatTime = currentTime;
        
        // Hitung BPM berdasarkan interval antara dua detak
        if (previousBeatTime > 0) {
            unsigned long interval = currentTime - previousBeatTime;
            
            // Pastikan interval valid (30-200 BPM)
            if (interval >= 300 && interval <= 2000) { // 30-200 BPM
                BPM = 60000 / interval;
                
                // Debug ke Serial (opsional)
                static unsigned long lastDebug = 0;
                if (currentTime - lastDebug > 1000) {
                    Serial.printf("ECG: Detak! Interval=%d ms, BPM=%d\n", interval, BPM);
                    lastDebug = currentTime;
                }
            } else {
                // Interval tidak valid, reset BPM
                BPM = 0;
            }
        }
        
        // Simpan waktu detak sebelumnya untuk perhitungan berikutnya
        previousBeatTime = currentTime;
        return true;
    }
    
    return false;
}

int ECGSensor::getHeartRateBPM() {
    if (!sensorAvailable) return 0;
    
    // Update deteksi detak
    detectHeartbeat();
    
    // Return BPM yang valid (30-200 BPM)
    if (BPM >= 30 && BPM <= 200) {
        return BPM;
    }
    return 0;
}