#include "EMGSensor.h"

EMGSensor::EMGSensor() {
    _buffer = nullptr;
    _calibrated = false;
    _baseline = 2048;  // Default 2048 (VCC/2 untuk 5-pin EMG)
    _calibrationSamples = 0;
    _calibrationTotal = 0;
}

void EMGSensor::begin(int pin, int windowSize) {
    _pin = pin;
    _windowSize = windowSize;
    
    // Alokasi buffer
    _buffer = new int[windowSize];
    for (int i = 0; i < windowSize; i++) {
        _buffer[i] = 0;
    }
    
    _readIndex = 0;
    _total = 0;
    _range = 0;
    
    pinMode(_pin, INPUT);
    
    Serial.printf("EMG Sensor initialized on pin %d with window size %d\n", _pin, _windowSize);
}

void EMGSensor::update() {
    // 1. Baca nilai mentah dari ADC
    int rawValue = analogRead(_pin);
    
    // 2. Jika belum dikalibrasi, lakukan kalibrasi
    if (!_calibrated) {
        startCalibration();
        return;
    }
    
    // 3. Rectification (nilai absolut dari selisih baseline)
    int rectified = abs(rawValue - _baseline);
    
    // 4. Update buffer dan hitung range
    updateBuffer(rectified);
}

void EMGSensor::updateBuffer(int value) {
    // Kurangi nilai lama dari total
    _total = _total - _buffer[_readIndex];
    
    // Tambahkan nilai baru
    _buffer[_readIndex] = value;
    _total = _total + value;
    
    // Update index
    _readIndex = (_readIndex + 1) % _windowSize;
    
    // Hitung range (maks - min) dari buffer
    int minVal = _buffer[0];
    int maxVal = _buffer[0];
    
    for (int i = 1; i < _windowSize; i++) {
        if (_buffer[i] < minVal) minVal = _buffer[i];
        if (_buffer[i] > maxVal) maxVal = _buffer[i];
    }
    
    _range = maxVal - minVal;
}

// Fungsi BARU: Mendapatkan amplitudo dalam mV
float EMGSensor::getAmplitudemV() {
    // Konversi range ke mV
    // Range adalah selisih antara max dan min dalam window
    // Kalibrasi: 0.8 mV per count ADC
    float amplitude_mV = _range * MV_PER_COUNT;
    
    // Batasi nilai maksimal
    if (amplitude_mV > MAX_MV) {
        amplitude_mV = MAX_MV;
    }
    
    return amplitude_mV;
}

// Fungsi BARU: Mendapatkan persentase aktivasi
int EMGSensor::getActivationPercent() {
    float amplitude_mV = getAmplitudemV();
    
    // Konversi ke persentase (0-100%)
    // Asumsi: 100% = 1000 mV (1V)
    int percent = (int)((amplitude_mV / 1000.0f) * 100.0f);
    
    // Batasi antara 0-100%
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    return percent;
}

int EMGSensor::getRawValue() {
    // Return nilai terbaru dari buffer
    if (_readIndex == 0) {
        return _buffer[_windowSize - 1];
    } else {
        return _buffer[_readIndex - 1];
    }
}

int EMGSensor::getWindowedRange() {
    return _range;
}

float EMGSensor::getEMGVoltage() {
    // Konversi range ke voltage (0-3.3V)
    int raw = getRawValue();
    return (raw * 3.3) / 4095.0;
}

bool EMGSensor::isSensorAvailable() {
    // Selama pin analog terhubung, sensor dianggap tersedia
    return true;
}

void EMGSensor::startCalibration() {
    // Baca 100 sampel untuk menentukan baseline
    if (_calibrationSamples < 100) {
        int rawValue = analogRead(_pin);
        _calibrationTotal += rawValue;
        _calibrationSamples++;
        
        if (_calibrationSamples >= 100) {
            _baseline = _calibrationTotal / _calibrationSamples;
            _calibrated = true;
            Serial.printf("EMG Calibration complete! Baseline: %d\n", _baseline);
        }
    }
}

bool EMGSensor::isCalibrated() {
    return _calibrated;
}

void EMGSensor::resetCalibration() {
    _calibrated = false;
    _calibrationSamples = 0;
    _calibrationTotal = 0;
}

int EMGSensor::getBaseline() {
    return _baseline;
}

String EMGSensor::getEMGLevel() {
    int range = getWindowedRange();
    
    if (range < 50) return "Relaxed";
    else if (range < 200) return "Light";
    else if (range < 500) return "Moderate";
    else if (range < 1000) return "Strong";
    else return "Maximum";
}