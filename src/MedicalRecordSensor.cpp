#include "MedicalRecordSensor.h"

// Constructor
MedicalRecordSensor::MedicalRecordSensor() {
    // Constructor kosong, inisialisasi dilakukan di begin()
}

// Callback saat detak jantung terdeteksi
void MedicalRecordSensor::onBeatDetected() {
    // Bisa untuk debug ringan
    // Serial.print("!");
}

// --- TASK MULTITASKING UNTUK MAX30100 ---
void MedicalRecordSensor::max30100Task(void * parameter) {
    MedicalRecordSensor* sensor = (MedicalRecordSensor*)parameter;
    
    Serial.println("MAX30100 Task started on Core 0");
    
    while (true) {
        // Update sensor hanya jika tidak sleeping
        if (!sensor->max30100Sleeping && sensor->max30100Available) {
            sensor->pox.update();
        }
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

// --- FUNGSI INISIALISASI SEMUA SENSOR ---
void MedicalRecordSensor::begin() {
    Serial.println("\n=== INISIALISASI MEDICAL RECORD SENSOR ===");
    
    // 1. Inisialisasi I2C bus untuk semua sensor
    Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
    delay(100);
    
    // 2. Inisialisasi EDA Sensor (Analog)
    Serial.println("1. Inisialisasi EDA Sensor...");
    analogReadResolution(12);
    analogSetPinAttenuation(EDA_PIN, ADC_11db);
    pinMode(EDA_PIN, INPUT);
    Serial.println("✓ EDA Sensor siap (ADC Pin 36)");
    
    // 3. Inisialisasi MAX30100 (Heart Rate & SpO2)
    Serial.println("2. Inisialisasi MAX30100...");
    delay(100);
    
    if (!pox.begin()) {
        Serial.println("✗ MAX30100 GAGAL diinisialisasi!");
        max30100Available = false;
    } else {
        max30100Available = true;
        pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
        pox.setOnBeatDetectedCallback(onBeatDetected);
        
        // Awalnya sensor dalam keadaan sleep
        sleepMAX30100();
        
        // Mulai task di Core 0
        xTaskCreatePinnedToCore(
            max30100Task,
            "MAX30100_Task",
            10000,
            this,
            1,
            &sensorTaskHandle,
            0
        );
        Serial.println("✓ MAX30100 siap (mode sleep)");
    }
    
    // 4. Inisialisasi MLX90614 (Temperature)
    Serial.println("3. Inisialisasi MLX90614...");
    ensureI2CForMLX90614(); // Pastikan I2C 100kHz
    
    int attempts = 0;
    while (attempts < 3) {
        if (mlx.begin()) {
            mlx90614Available = true;
            
            // Test baca suhu
            float objTemp = mlx.readObjectTempC();
            Serial.printf("✓ MLX90614 siap - Test: %.2f°C\n", objTemp);
            
            // Set flag sleeping = false karena sensor aktif
            mlx90614Sleeping = false;
            break;
        }
        delay(100);
        attempts++;
    }
    
    if (!mlx90614Available) {
        mlx90614Sleeping = true;
        Serial.println("✗ MLX90614 GAGAL diinisialisasi!");
    }
    
    Serial.println("=== INISIALISASI SELESAI ===");
}

// --- FUNGSI EDA SENSOR ---
void MedicalRecordSensor::calibrateEDA() {
    Serial.println("Kalibrasi EDA... diam 3 detik.");
    long total = 0;
    const int N = 300;
    delay(500);
    
    for (int i = 0; i < N; i++) {
        total += analogRead(EDA_PIN);
        delay(5);
    }
    
    edaBaseline = (float)total / (float)N;
    edaCalibrated = true;
    Serial.printf("Baseline EDA: %.2f\n", edaBaseline);
}

int MedicalRecordSensor::readEDARaw() {
    return analogRead(EDA_PIN);
}

String MedicalRecordSensor::getStressLevel(int rawValue) {
    if (!edaCalibrated) {
        calibrateEDA(); // Kalibrasi otomatis jika belum
    }
    
    int delta = rawValue - (int)edaBaseline;
    
    if (delta < 50) return "Tenang";
    else if (delta < 150) return "Cemas Ringan";
    else if (delta < 300) return "Cemas Sedang";
    else return "Stres Tinggi";
}

float MedicalRecordSensor::getEDAConductance(int rawValue) {
    // Konversi ADC ke tegangan (3.3V reference, 12-bit ADC)
    float voltage = rawValue * (3.3 / 4095.0);
    
    // Hindari pembagian dengan nol
    if (voltage >= 3.29) return 0.0;
    
    // Rumus konversi ke konduktansi (asumsi sensor EDA dengan resistor fixed 1MΩ)
    float conductance = (voltage * 1000000.0) / (3.3 - voltage);
    
    // Konversi ke microsiemens (µS)
    return conductance;
}

// --- FUNGSI MAX30100 ---
bool MedicalRecordSensor::isMAX30100Available() {
    return max30100Available;
}

float MedicalRecordSensor::getHeartRate() {
    if (!max30100Available || max30100Sleeping || !max30100Measuring) return 0.0;
    
    float hr = pox.getHeartRate();
    
    // Filter nilai yang tidak valid
    if (hr < 30.0 || hr > 200.0) {
        return 0.0;
    }
    return hr;
}

int MedicalRecordSensor::getSpO2() {
    if (!max30100Available || max30100Sleeping || !max30100Measuring) return 0;
    
    int sp = pox.getSpO2();
    
    // Filter nilai yang tidak valid
    if (sp < 70 || sp > 100) {
        return 0;
    }
    return sp;
}

void MedicalRecordSensor::startHRMeasurement() {
    if (max30100Available && !max30100Sleeping) {
        max30100Measuring = true;
    }
}

void MedicalRecordSensor::stopHRMeasurement() {
    max30100Measuring = false;
}

bool MedicalRecordSensor::isHRMeasuring() {
    return max30100Measuring && max30100Available && !max30100Sleeping;
}

void MedicalRecordSensor::sleepMAX30100() {
    if (!max30100Available) return;
    if (max30100Sleeping) return; // Sudah sleep
    
    Serial.println("MAX30100: Going to sleep...");
    
    max30100Measuring = false;
    max30100Sleeping = true;
    
    // Matikan sensor
    pox.shutdown();
    
    Serial.println("MAX30100: Sensor sleeping (low power)");
}

void MedicalRecordSensor::wakeUpMAX30100() {
    if (!max30100Available) return;
    if (!max30100Sleeping) return; // Sudah bangun
    
    Serial.println("MAX30100: Waking up...");
    
    // Bangunkan sensor
    pox.resume();
    delay(200); // Tunggu sensor stabil
    
    // Reinitialize sensor
    if (!pox.begin()) {
        Serial.println("MAX30100: Wake up failed!");
        return;
    }
    
    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
    max30100Sleeping = false;
    max30100Measuring = true; // Siap untuk measurement
    
    Serial.println("MAX30100: Sensor awake and ready");
}

void MedicalRecordSensor::restartI2C() {
    // Restart I2C bus
    Wire.end();
    delay(10);
    Wire.begin(21, 22);
    delay(100);
    
    Serial.println("I2C restarted for sensors");
}

void MedicalRecordSensor::softResetMAX30100() {
    if (!max30100Available) return;
    
    pox.shutdown();
    delay(200);
    pox.resume();
    delay(200);
    pox.begin();
    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    
    Serial.println("MAX30100: Soft reset completed");
}

String MedicalRecordSensor::getMAX30100Status() {
    if (!max30100Available) return "Sensor Error";
    if (max30100Sleeping) return "Sleeping (Low Power)";
    if (!max30100Measuring) return "Ready";
    return "Measuring";
}

// --- FUNGSI MLX90614 ---
bool MedicalRecordSensor::isMLX90614Available() {
    return mlx90614Available;
}

void MedicalRecordSensor::ensureI2CForMLX90614() {
    // Set I2C clock ke 100kHz untuk MLX90614
    Wire.setClock(100000);
    delay(5);
}

float MedicalRecordSensor::readObjectTemp() {
    if (!mlx90614Available) {
        return 0.0;
    }
    
    // Pastikan I2C pada 100kHz setiap kali membaca
    ensureI2CForMLX90614();
    
    // Baca suhu objek (tubuh) dengan error handling
    float temp = mlx.readObjectTempC();
    
    // Filter nilai NaN atau tidak valid
    if (isnan(temp) || temp < 20.0 || temp > 50.0) {
        // Coba baca sekali lagi
        delay(10);
        temp = mlx.readObjectTempC();
        
        // Jika masih tidak valid, return 0
        if (isnan(temp) || temp < 20.0 || temp > 50.0) {
            return 0.0;
        }
    }
    
    return temp;
}

float MedicalRecordSensor::readAmbientTemp() {
    if (!mlx90614Available) {
        return 0.0;
    }
    
    // Pastikan I2C pada 100kHz
    ensureI2CForMLX90614();
    
    // Baca suhu ambient
    float temp = mlx.readAmbientTempC();
    
    // Filter nilai tidak valid
    if (isnan(temp) || temp < -40.0 || temp > 125.0) {
        delay(10);
        temp = mlx.readAmbientTempC();
        
        if (isnan(temp) || temp < -40.0 || temp > 125.0) {
            return 0.0;
        }
    }
    
    return temp;
}

void MedicalRecordSensor::wakeUpMLX90614() {
    if (!mlx90614Available || !mlx90614Sleeping) return;
    
    // MLX90614 tidak punya mode sleep hardware
    // Kita hanya set flag dan pastikan I2C siap
    mlx90614Sleeping = false;
    
    // Reset I2C untuk MLX90614
    ensureI2CForMLX90614();
    delay(100);
    
    Serial.println("MLX90614: Sensor siap untuk pembacaan");
}

void MedicalRecordSensor::sleepMLX90614() {
    if (!mlx90614Available || mlx90614Sleeping) return;
    
    // MLX90614 tidak punya mode sleep hardware
    mlx90614Sleeping = true;
    Serial.println("MLX90614: Sensor dalam mode idle");
}

String MedicalRecordSensor::getMLX90614Status() {
    if (!mlx90614Available) return "Sensor Error";
    if (mlx90614Sleeping) return "Idle";
    return "Ready";
}

// --- FUNGSI KOMBINASI ---
String MedicalRecordSensor::getAllSensorsStatus() {
    String status = "EDA: Ready | ";
    status += "MAX30100: " + getMAX30100Status() + " | ";
    status += "MLX90614: " + getMLX90614Status();
    return status;
}

void MedicalRecordSensor::wakeAllSensors() {
    if (max30100Available) {
        wakeUpMAX30100();
    }
    
    if (mlx90614Available) {
        wakeUpMLX90614();
    }
}

void MedicalRecordSensor::sleepAllSensors() {
    if (max30100Available) {
        sleepMAX30100();
    }
    
    if (mlx90614Available) {
        sleepMLX90614();
    }
}