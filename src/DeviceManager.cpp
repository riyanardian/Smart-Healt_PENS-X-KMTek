#include "DeviceManager.h"

void DeviceManager::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Gagal mount SPIFFS!");
        delay(2000);
    } else {
        loadConfig();
    }
}

bool DeviceManager::fileExists(const char* path) {
    return SPIFFS.exists(path);
}

void DeviceManager::createEmptyConfig() {
    StaticJsonDocument<256> doc;
    doc.createNestedArray("devices");
    File f = SPIFFS.open(CONFIG_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("Gagal membuat config.json");
        return;
    }
    serializeJson(doc, f);
    f.close();
}

void DeviceManager::loadConfig() {
    devices.clear();
    if (!fileExists(CONFIG_PATH)) {
        Serial.println("config.json tidak ada, membuat...");
        createEmptyConfig();
    }

    File f = SPIFFS.open(CONFIG_PATH, FILE_READ);
    if (!f) {
        Serial.println("Gagal buka config.json");
        return;
    }

    String content = f.readString();
    f.close();

    if (content.length() == 0) {
        createEmptyConfig();
        return;
    }

    StaticJsonDocument<4096> doc;
    auto err = deserializeJson(doc, content);
    if (err) {
        Serial.printf("Gagal parse config.json: %s\n", err.c_str());
        return;
    }

    JsonArray arr = doc["devices"].as<JsonArray>();
    for (JsonObject obj : arr) {
        Device d;
        d.name = obj["name"].as<String>();
        d.ubidotsToken = obj["ubidotsToken"].as<String>();
        d.ubidotsDeviceLabel = obj["ubidotsDeviceLabel"].as<String>();
        devices.push_back(d);
    }
}

void DeviceManager::saveConfig() {
    StaticJsonDocument<4096> doc;
    JsonArray arr = doc.createNestedArray("devices");
    for (auto &d : devices) {
        JsonObject o = arr.createNestedObject();
        o["name"] = d.name;
        o["ubidotsToken"] = d.ubidotsToken;
        o["ubidotsDeviceLabel"] = d.ubidotsDeviceLabel;
    }

    File f = SPIFFS.open(CONFIG_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("Gagal tulis config.json");
        return;
    }
    serializeJson(doc, f);
    f.close();
}

void DeviceManager::addDevice(String name, String token, String deviceLabel) {
    devices.push_back({name, token, deviceLabel});
    saveConfig();
}

void DeviceManager::removeDevice(int index) {
    if (index >= 0 && index < devices.size()) {
        devices.erase(devices.begin() + index);
        saveConfig();
    }
}

void DeviceManager::editDevice(int index, String newName, String newToken, String newDeviceLabel) {
    if (index >= 0 && index < devices.size()) {
        devices[index].name = newName;
        devices[index].ubidotsToken = newToken;
        devices[index].ubidotsDeviceLabel = newDeviceLabel;
        saveConfig();
        Serial.printf("User %d berhasil diupdate!\n", index);
    } else {
        Serial.println("Index tidak valid!");
    }
}

void DeviceManager::printDevices() {
    Serial.println("\n=== DAFTAR USER (Token Lengkap) ===");
    if (devices.empty()) {
        Serial.println("(kosong)");
        return;
    }
    for (size_t i = 0; i < devices.size(); i++) {
        Serial.printf("%d: %s\n", (int)i, devices[i].name.c_str());
        
        // Tampilkan token lengkap dengan format yang mudah dibaca
        String token = devices[i].ubidotsToken;
        Serial.print("   Token: ");
        
        // Tampilkan per 16 karakter per baris
        for (int j = 0; j < token.length(); j++) {
            Serial.print(token[j]);
            if ((j + 1) % 16 == 0 && j != token.length() - 1) {
                Serial.print("\n          "); // Indent untuk baris berikutnya
            }
        }
        Serial.println();
        
        Serial.printf("   Device Label: %s\n", devices[i].ubidotsDeviceLabel.c_str());
        Serial.printf("   EDA Variable: %s\n", UBIDOTS_VARIABLE_LABEL);
        Serial.println("   " + String(40, '-'));
    }
}

bool DeviceManager::isEmpty() {
    return devices.empty();
}