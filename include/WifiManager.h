#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <vector>
#include "Config.h"

class WifiManager {
public:
    void begin();
    bool hasCredentials();
    void connectSaved();
    void saveCredentials(String ssid, String pass);
    void clearCredentials();
    String getSSID();
    
    // Scan WiFi networks
    int scanNetworks(); 
    
    // Get scanned network info
    String getScannedSSID(int index);
    int getScannedRSSI(int index);
    String getScannedEncryption(int index);

    bool connectWithTimeout(int timeoutMs = 10000); // Timeout 10 detik
    void disconnect(); // Fungsi untuk disconnect

    void stopForScan();
    void resumeAfterScan();

private:
    Preferences pref;
    
    // Vectors to store scan results
    std::vector<String> scannedSSIDs;
    std::vector<int> scannedRSSIs;
    std::vector<String> scannedEncTypes;
};

#endif