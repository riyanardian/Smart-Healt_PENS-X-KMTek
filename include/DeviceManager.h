#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "Config.h"

struct Device {
  String name;
  String ubidotsToken;       // Ganti dari writeKey
  String ubidotsDeviceLabel; // Tambah device label
};

class DeviceManager {
public:
    std::vector<Device> devices;

    void begin();
    void loadConfig();
    void saveConfig();
    void addDevice(String name, String token, String deviceLabel);
    void removeDevice(int index);
    void editDevice(int index, String newName, String newToken, String newDeviceLabel);
    void printDevices();
    bool isEmpty();

private:
    bool fileExists(const char* path);
    void createEmptyConfig();
};

#endif