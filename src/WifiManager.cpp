#include "WifiManager.h"

void WifiManager::begin() {
    pref.begin(PREF_NAMESPACE, false); // RW mode
}

bool WifiManager::hasCredentials() {
    return pref.isKey("wifi_ssid");
}

String WifiManager::getSSID() {
    return pref.getString("wifi_ssid", "");
}

void WifiManager::saveCredentials(String ssid, String pass) {
    pref.putString("wifi_ssid", ssid);
    pref.putString("wifi_pass", pass);
}

void WifiManager::stopForScan() {
    // Nonaktifkan WiFi sementara untuk scan
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
}

void WifiManager::resumeAfterScan() {
    // Hidupkan kembali WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    
    // Coba reconnect jika ada kredensial
    if (hasCredentials()) {
        connectSaved();
    }
}

void WifiManager::clearCredentials() {
    pref.remove("wifi_ssid");
    pref.remove("wifi_pass");
}

void WifiManager::connectSaved() {
    if (!hasCredentials()) {
        Serial.println("Belum ada kredensial WiFi.");
        return;
    }
    
    String ssid = pref.getString("wifi_ssid", "");
    String pass = pref.getString("wifi_pass", "");
    
    Serial.printf("Mencoba konek ke: %s\n", ssid.c_str());
    
    // Matikan WiFi dulu untuk reset
    WiFi.disconnect(true);
    delay(100);
    
    // Mulai koneksi
    WiFi.begin(ssid.c_str(), pass.c_str());
    
    // Jangan tunggu di sini, biarkan di main loop yang mengecek
    // Hanya beri status awal
    Serial.println("Memulai koneksi WiFi...");
}

bool WifiManager::connectWithTimeout(int timeoutMs) {
    if (!hasCredentials()) return false;
    
    String ssid = pref.getString("wifi_ssid", "");
    String pass = pref.getString("wifi_pass", "");
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(ssid.c_str(), pass.c_str());
    
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeoutMs) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(500);
    }
    
    return false;
}

int WifiManager::scanNetworks() {
    Serial.println("Scanning WiFi networks...");
    
    // Clear previous scan results
    scannedSSIDs.clear();
    scannedRSSIs.clear();
    scannedEncTypes.clear();
    
    // Clear previous scan cache
    WiFi.scanDelete();
    
    // Start new scan
    int n = WiFi.scanNetworks();
    
    // Tambah delay untuk memastikan scan selesai
    delay(100);
    
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.printf("%d networks found:\n", n);
        for (int i = 0; i < n; i++) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            String encryption;
            
            // Get encryption type
            wifi_auth_mode_t enc = WiFi.encryptionType(i);
            switch (enc) {
                case WIFI_AUTH_OPEN:
                    encryption = "Open";
                    break;
                case WIFI_AUTH_WEP:
                    encryption = "WEP";
                    break;
                case WIFI_AUTH_WPA_PSK:
                    encryption = "WPA";
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    encryption = "WPA2";
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    encryption = "WPA/WPA2";
                    break;
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    encryption = "WPA2-E";
                    break;
                case WIFI_AUTH_WPA3_PSK:
                    encryption = "WPA3";
                    break;
                case WIFI_AUTH_WPA2_WPA3_PSK:
                    encryption = "WPA2/WPA3";
                    break;
                default:
                    encryption = "Unknown";
                    break;
            }
            
            // Store in vectors
            scannedSSIDs.push_back(ssid);
            scannedRSSIs.push_back(rssi);
            scannedEncTypes.push_back(encryption);
            
            Serial.printf("%2d: %s (%ddBm) %s\n", i, ssid.c_str(), rssi, encryption.c_str());
        }
    }
    
    return n;
}
String WifiManager::getScannedSSID(int index) {
    if (index >= 0 && index < (int)scannedSSIDs.size()) {
        return scannedSSIDs[index];
    }
    return "";
}

int WifiManager::getScannedRSSI(int index) {
    if (index >= 0 && index < (int)scannedRSSIs.size()) {
        return scannedRSSIs[index];
    }
    return 0;
}

String WifiManager::getScannedEncryption(int index) {
    if (index >= 0 && index < (int)scannedEncTypes.size()) {
        return scannedEncTypes[index];
    }
    return "";
}