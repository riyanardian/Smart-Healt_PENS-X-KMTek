#ifndef EMGSENSOR_H
#define EMGSENSOR_H

#include <Arduino.h>
#include "Config.h"

class EMGSensor {
public:
    EMGSensor();
    void begin(int pin = EMG_PIN, int windowSize = EMG_WINDOW_SIZE);
    void update();                
    int getRawValue();            
    int getWindowedRange();       
    float getAmplitudemV();       // Mendapatkan amplitudo dalam mV (BARU: fungsi utama)
    int getActivationPercent();   // Mendapatkan persentase aktivasi (0-100%)
    float getEMGVoltage();        
    bool isSensorAvailable();     
    void startCalibration();      
    bool isCalibrated();          
    void resetCalibration();      
    int getBaseline();            
    String getEMGLevel();         
    
private:
    int _pin;
    int _windowSize;
    int* _buffer;                 
    int _readIndex;
    long _total;
    int _range;
    bool _calibrated;
    int _baseline;
    int _calibrationSamples;
    long _calibrationTotal;
    
    // Konstanta untuk konversi
    const float MV_PER_COUNT = EMG_MV_PER_COUNT;  // 0.8 mV per count
    const float MAX_MV = EMG_MAX_MV;              // 3000 mV maksimal
    
    void updateBuffer(int value); 
};

#endif