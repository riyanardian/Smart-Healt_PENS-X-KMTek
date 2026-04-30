#include "BodyPositionSnoreSensor.h"
#include "Config.h"

void BodyPositionSnoreSensor::begin() {
    Serial.println("Initializing Body Position & Snore Sensor...");

    Wire1.begin(MPU6050_SDA, MPU6050_SCL);
    Wire1.setClock(100000);
    delay(100);

    resetMPU6050();
    mpuAvailable = initMPU6050();

    if (mpuAvailable) {
        Serial.println("MPU6050 OK");
        calibrateMPU6050(100);
    } else {
        Serial.println("MPU6050 FAIL");
    }

    pinMode(SOUND_PIN, INPUT);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    Serial.println("Sensor ready");
}

// ===== MPU =====

bool BodyPositionSnoreSensor::initMPU6050() {
    mpuAddr = 0x68;
    Wire1.beginTransmission(mpuAddr);
    if (Wire1.endTransmission() != 0) {
        mpuAddr = 0x69;
        Wire1.beginTransmission(mpuAddr);
        if (Wire1.endTransmission() != 0) {
            return false;
        }
    }

    Wire1.beginTransmission(mpuAddr);
    Wire1.write(0x6B);
    Wire1.write(0x00);
    if (Wire1.endTransmission() != 0) return false;

    return true;
}

void BodyPositionSnoreSensor::resetMPU6050() {
    // MPU is reset during init if needed, just clear sleep here
    Wire1.beginTransmission(0x68);
    Wire1.write(0x6B);
    Wire1.write(0x80);
    Wire1.endTransmission();
    Wire1.beginTransmission(0x69);
    Wire1.write(0x6B);
    Wire1.write(0x80);
    Wire1.endTransmission();
    delay(50);
}

void BodyPositionSnoreSensor::calibrateMPU6050(int samples) {
    // Untuk deteksi posisi tubuh, kita memerlukan pembacaan gravitasi absolut.
    // Jika kita melakukan kalibrasi offset pada accelerometer saat alat tidak
    // dalam posisi datar sempurna, pengukuran posisi selanjutnya akan kacau.
    // Oleh karena itu, kita set offset accelerometer ke 0.
    accOffsetX = 0;
    accOffsetY = 0;
    accOffsetZ = 0;
    Serial.println("Kalibrasi MPU untuk accelerometer di-skip (menggunakan nilai absolut)");
    delay(500);
}

bool BodyPositionSnoreSensor::readMPU6050() {
    Wire1.beginTransmission(mpuAddr);
    Wire1.write(0x3B);

    if (Wire1.endTransmission(false) != 0) return false;

    Wire1.requestFrom((uint8_t)mpuAddr, (uint8_t)14);
    if (Wire1.available() < 14) return false;

    rax = Wire1.read()<<8 | Wire1.read();
    ray = Wire1.read()<<8 | Wire1.read();
    raz = Wire1.read()<<8 | Wire1.read();

    Wire1.read(); Wire1.read();

    rgx = Wire1.read()<<8 | Wire1.read();
    rgy = Wire1.read()<<8 | Wire1.read();
    rgz = Wire1.read()<<8 | Wire1.read();

    return true;
}

// ===== SOUND =====

int BodyPositionSnoreSensor::readAmplitude() {
    int minV = 4095;
    int maxV = 0;

    for (int i=0;i<100;i++) {
        int v = analogRead(this->SOUND_PIN);
        if (v < minV) minV = v;
        if (v > maxV) maxV = v;
        delayMicroseconds(1000);
    }

    return maxV - minV;
}

// ===== UPDATE =====

void BodyPositionSnoreSensor::update() {
    static unsigned long tSound=0;
    static unsigned long tMPU=0;
    unsigned long now = millis();

    // SOUND
    if (now - tSound >= 100) {
        tSound = now;

        int raw = this->readAmplitude();

        if (raw > 1000) raw = 1000;

        int norm = map(raw, 0, 1000, 0, 100);

        lastAmplitude = (lastAmplitude * 0.7f) + (norm * 0.3f);

        static unsigned long dbg=0;
        if (now - dbg > 2000) {
            Serial.printf("Snore: %d\n", lastAmplitude);

            if (lastAmplitude < 10) Serial.println("Tidak dengkur");
            else if (lastAmplitude < 30) Serial.println("Ringan");
            else if (lastAmplitude < 60) Serial.println("Sedang");
            else Serial.println("Keras");

            dbg = now;
        }
    }

    // MPU
    if (now - tMPU >= 150) {
        tMPU = now;

        if (mpuAvailable && readMPU6050()) {
            float ax = (rax - accOffsetX)/ACC_SCALE;
            float ay = (ray - accOffsetY)/ACC_SCALE;
            float az = (raz - accOffsetZ)/ACC_SCALE;

            if (az > 0.7f) lastPosisi = POS_TELENTANG;
            else if (az < -0.7f) lastPosisi = POS_TENGKURAP;
            else if (ax > 0.7f) lastPosisi = POS_MIRING_KANAN;
            else if (ax < -0.7f) lastPosisi = POS_MIRING_KIRI;
            else if (ay > 0.7f) lastPosisi = POS_TEGAK;
            else if (ay > 0.3f) lastPosisi = POS_DUDUK;
            else lastPosisi = POS_TRANSISI;

            // Debug print untuk posisi tubuh
            static unsigned long tDebugPos=0;
            if (now - tDebugPos > 2000) {
                Serial.printf("AX: %.2f AY: %.2f AZ: %.2f | Pos: %s\n", ax, ay, az, getPositionName().c_str());
                tDebugPos = now;
            }
        }
    }
}

// ===== POSITION NAME =====

String BodyPositionSnoreSensor::getPositionName() {
    switch (lastPosisi) {
        case POS_TEGAK: return "Berdiri";
        case POS_DUDUK: return "Duduk";
        case POS_TELENTANG: return "Telentang";
        case POS_TENGKURAP: return "Tengkurap";
        case POS_MIRING_KANAN: return "Miring Kanan";
        case POS_MIRING_KIRI: return "Miring Kiri";
        default: return "Transisi";
    }
}