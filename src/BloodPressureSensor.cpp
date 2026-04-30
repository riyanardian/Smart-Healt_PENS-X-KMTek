#include "BloodPressureSensor.h"

void BloodPressureSensor::begin() {
    Serial2.begin(115200, SERIAL_8N1, BP_RX_PIN, BP_TX_PIN);
    memset(last5Chars, 0, sizeof(last5Chars));
    currentState = SEARCHING_HEADER;
    bufferIndex = 0;
    newDataAvailable = false;
    measuring = false;
    dataValid = false;
    
    Serial.println("Blood Pressure Sensor (Serial2) initialized");
    Serial.printf("RX Pin: %d, TX Pin: %d\n", BP_RX_PIN, BP_TX_PIN);
}

void BloodPressureSensor::update() {
    while (Serial2.available()) {
        char c = (char)Serial2.read();

        switch (currentState) {
            case SEARCHING_HEADER:
                for (int i = 0; i < 4; i++) last5Chars[i] = last5Chars[i + 1];
                last5Chars[4] = c;

                if (last5Chars[0] == 'e' && last5Chars[1] == 'r' && last5Chars[2] == 'r' &&
                    last5Chars[3] == ':' && last5Chars[4] == '0') {
                    Serial.println("BP Header ditemukan! Mulai koleksi data...");
                    currentState = COLLECTING_DATA;
                    bufferIndex = 0;
                    memset(rawBuffer, 0, sizeof(rawBuffer));
                }
                break;

            case COLLECTING_DATA:
                if (bufferIndex < 50) {
                    rawBuffer[bufferIndex] = c;
                    bufferIndex++;
                } else {
                    processBuffer();
                    currentState = SEARCHING_HEADER;
                }
                break;
        }
    }
}

void BloodPressureSensor::processBuffer() {
    int offset = 30;
    if (offset + 10 < 50) {
        systolic = hexToDec(rawBuffer[offset + 0]) * 16 + hexToDec(rawBuffer[offset + 1]);
        diastolic = hexToDec(rawBuffer[offset + 3]) * 16 + hexToDec(rawBuffer[offset + 4]);
        bpm = hexToDec(rawBuffer[offset + 9]) * 16 + hexToDec(rawBuffer[offset + 10]);

        // Validasi data
        if (systolic > 0 && systolic < 300 && 
            diastolic > 0 && diastolic < 300 && 
            bpm > 0 && bpm < 200) {
            
            dataValid = true;
            
            Serial.println("\n=== DATA TEKANAN DARAH ===");
            Serial.printf("Sistolik  : %d mmHg\n", systolic);
            Serial.printf("Diastolik : %d mmHg\n", diastolic);
            Serial.printf("Pulse     : %d bpm\n", bpm);
            Serial.println("=======================");
        } else {
            dataValid = false;
            Serial.println("Data BP tidak valid, diabaikan");
        }

        newDataAvailable = true;
    }
}

int BloodPressureSensor::hexToDec(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

int BloodPressureSensor::getSystolic() {
    return systolic;
}

int BloodPressureSensor::getDiastolic() {
    return diastolic;
}

int BloodPressureSensor::getBPM() {
    return bpm;
}

bool BloodPressureSensor::isDataAvailable() {
    return newDataAvailable;
}

void BloodPressureSensor::resetDataFlag() {
    newDataAvailable = false;
}

bool BloodPressureSensor::isMeasuring() {
    return measuring; // Selalu true saat dalam mode BP
}

bool BloodPressureSensor::hasValidData() {
    return dataValid;
}

void BloodPressureSensor::startMeasurement() {
    measuring = true;
    dataValid = false; // Reset data valid
    Serial.println("Blood Pressure mode: siap menerima data dari alat");
}

void BloodPressureSensor::stopMeasurement() {
    measuring = false;
    dataValid = false; // Reset data valid
    Serial.println("Blood Pressure mode dihentikan");
}