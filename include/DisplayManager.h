#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_FT6206.h>
#include <Wire.h>
#include <vector>
#include "Config.h"

// ==================== WARNA TEMA MODERN MEDICAL ====================
#define TFT_BG_COLOR 0xF7BE          // Background: Putih/Abu-abu muda
#define TFT_HEADER_COLOR 0x24B6      // Header/Primary: Biru Medis
#define TFT_TEXT_COLOR 0x2121        // Text: Abu-abu gelap
#define TFT_ACCENT_GREEN 0x2E67      // Accent Success: Hijau Emerald
#define TFT_ACCENT_ORANGE 0xEC27     // Accent Warning: Orange Soft
#define TFT_ACCENT_BLUE 0x24B6       // Biru Medis (sama dengan header)
#define TFT_ACCENT_RED 0xF800        // Tetap merah untuk peringatan penting
#define TFT_ACCENT_PURPLE 0x781F     // Ungu lembut untuk secondary
#define TFT_ACCENT_CYAN 0x04DF       // Cyan lembut untuk informasi
#define TFT_BUTTON_BG 0xD69A         // Background tombol: Abu-abu sangat muda
#define TFT_BUTTON_BORDER 0x9CD3     // Border tombol: Biru muda
#define TFT_BACK_BUTTON_COLOR 0xAD55  // Warna konsisten untuk tombol kembali - Abu-abu kehijauan
#define TFT_BACK_BUTTON_BORDER 0x9CD3 // Border tombol kembali

class DisplayManager {
public:
    void begin();

    // WiFi Scan Functions
    void showWifiScanList(const std::vector<String>& ssids, const std::vector<int>& rssis, const std::vector<String>& encTypes, int currentPage = 0);

    bool showConfirmationDialogSerial(String title, String message);
    int getCurrentScreenMode() { return currentScreenMode; }
    bool showOnScreenConfirmation(String title, String message, String yesBtn = "YA", String noBtn = "TIDAK");

    void drawTextCentered(String text, int x, int y, uint16_t color = TFT_WHITE, uint16_t bgColor = TFT_BLACK, int textSize = 2);
    void drawButtonPublic(int x, int y, int w, int h, String label, uint16_t color);
    void showConfirmationDialog(String title, String message, String btnYesLabel = "YA", String btnNoLabel = "TIDAK");
    int getTouchInput(int timeoutMs = 10000);
    
    // Getter untuk calibration values
    int getTouchMinX() { return touchMinX; }
    int getTouchMaxX() { return touchMaxX; }
    int getTouchMinY() { return touchMinY; }
    int getTouchMaxY() { return touchMaxY; }
    
    // Fungsi untuk membersihkan layar
    void clearContentArea();
    void clearScreen();
    
    // Fungsi untuk menggambar berbagai screen
    void drawHomeScreen(String currentDeviceName, bool isWiFiConnected);
    void drawInterface(String currentDeviceName);
    void drawMedicalRecordInterface(String currentDeviceName); // Fungsi baru untuk Medical Record
    
    // PERBAIKAN: Update parameter fungsi showUserInfo
    void showUserInfo(String userName, String ubidotsToken, String deviceLabel, int currentIndex, int totalUsers);
    
    // PERBAIKAN: Update parameter fungsi showEditUser
    void showEditUser(int currentIndex, String currentName, String currentToken, String currentDeviceLabel);
    
    void showWifiConfig(String ssid, String ip, int rssi, bool isConnected);
    void updateSensorData(int gsrValue, String stressLevel, bool isSending);
    void updateMedicalRecordData(int edaValue, String stressLevel, int heartRate, int spo2, float objectTemp, bool isSending);
    void updateStatus(String statusMsg);

    // Daftar user
    void showUserList(const std::vector<String>& userNames, int currentIndex, bool resetPage = true);
    
    // Fungsi checkTouch dengan parameter screen mode
    int checkTouch(int screenMode = 0); // 0=home, 1=measurement, 2=userInfo, 3=wifiConfig, 4=editUser, 5=keyboard, 6=userList, 7=wifiScan
    
    void startCalibration();

    // Tombol untuk Medical Record (posisi baru)
    int medBtnStartX = 20, medBtnStartY = 245, medBtnW = 200, medBtnH = 50;      // Lebar 220 (hanya 2 tombol)
    int medBtnModeX = 240, medBtnModeY = 245, medBtnModeW = 200, medBtnModeH = 50; // Lebar 220 (hanya 2 tombol)

    // Tambah fungsi untuk set flag
    void setHasUsers(bool hasUsers) { hasUsersFlag = hasUsers; }

    // FUNGSI KEYBOARD VIRTUAL
    void showKeyboard(String currentValue, String title, bool forName = true);
    String getKeyboardInput();
    void clearKeyboard();
    bool isKeyboardActive() { return keyboardActive; }
    bool isShiftActive() { return shiftActive; }
    void toggleShift();
    void setKeyboardMode(bool forName); // true=for name (alfanumerik), false=for key (numeric/special)
    
    // Fungsi untuk mengetahui apakah keyboard dibatalkan
    bool isKeyboardCancelled() { return keyboardCancelled; }

    bool showSingleButtonDialog(String title, String message, String btnLabel = "OK");

    void showMeasurementModeSelection(int currentModeIndex);
    String getMeasurementModeName(int index);
    bool isModeSelectionActive() { return inModeSelection; }

    // Fungsi untuk ECG Interface
    void drawECGInterface(String currentDeviceName);
    void updateECGData(int ecgRaw, float ecgVoltage, int heartRate, String leadStatus, bool isSending, int* ecgBuffer, int bufferSize, int threshold);

    void setMeasurementMode(int mode) { currentMeasurementMode = mode; }
    int getMeasurementMode() { return currentMeasurementMode; }

    void drawECGGraph(int* ecgData, int dataSize, int threshold);
    void updateECGGraph(int* ecgData, int dataSize, int threshold, int heartRate);

    // Fungsi untuk Blood Pressure Interface
    void drawBloodPressureInterface(String currentDeviceName);
    void updateBloodPressureData(int systolic, int diastolic, int heartRate, bool isSending);

    void drawBodyPositionSnoreInterface(String currentDeviceName);
    void updateBodyPositionSnoreData(int position, int snoreScale, bool isSending);

    void drawEMGInterface(String userName);
    void updateEMGData(int range, float voltage, String level, bool isSending, int* emgData = nullptr, int dataSize = 0);

    void resetMeasurementMode();
    
    // Fungsi untuk menggambar garis tebal
    void drawThickLine(int x1, int y1, int x2, int y2, uint16_t color, int thickness);
    
    void drawEMGGraphSimple();  // Fungsi baru untuk menggambar grafik sederhana
    void updateEMGGraphSimple(int* emgData, int dataSize, float amplitude, int activationPercent);

private:
    TFT_eSPI tft = TFT_eSPI();
    Adafruit_FT6206 ts = Adafruit_FT6206(); 
    bool touchSupported = false;
    bool hasUsersFlag = false;

    int currentScreenMode = 0;

    // POSISI TOMBOL UNTUK HOME SCREEN
    int btnWidth = 200;
    int btnHeight = 70;
    
    // Tombol Home Screen
    int btnUserX = 20, btnUserY = 90;
    int btnMeasureX = 260, btnMeasureY = 90;
    int btnChangeX = 20, btnChangeY = 190;
    int btnWifiX = 260, btnWifiY = 190;
    
    // Tombol untuk mode pengukuran
    int btnStartX = 20, btnStartY = 240, btnW = 130, btnH = 60;
    int btnNextX = 170, btnNextY = 240;

    // Tombol untuk User Info Screen
    int btnEditUserX = 50, btnEditUserY = 250, btnEditW = 160, btnEditH = 40;
    int btnBackUserX = 250, btnBackUserY = 250, btnBackW = 160, btnBackH = 40;
    
    // Tombol untuk WiFi Config Screen
    int btnScanX = 50, btnScanY = 200, btnScanW = 160, btnScanH = 40;
    int btnBackWifiX = 270, btnBackWifiY = 200, btnBackWifiW = 160, btnBackWifiH = 40;

    // Tombol untuk Edit User Screen
    int btnEditNameX = 10, btnEditNameY = 250, btnEditNameW = 110, btnEditNameH = 40;
    int btnEditKeyX = 130, btnEditKeyY = 250, btnEditKeyW = 110, btnEditKeyH = 40;
    int btnEditDeviceX = 250, btnEditDeviceY = 250, btnEditDeviceW = 110, btnEditDeviceH = 40;
    int btnCancelEditX = 370, btnCancelEditY = 250, btnCancelEditW = 100, btnCancelEditH = 40;

    // KEYBOARD VIRTUAL
    bool keyboardActive = false;
    bool keyboardCancelled = false;
    bool shiftActive = false;
    bool capsLock = false;
    bool forNameField = true;
    bool symbolMode = false;
    String keyboardBuffer = "";
    String keyboardTitle = "";
    int cursorPos = 0;
    
    // Layout keyboard
    static const int KEY_ROWS = 4;
    static const int KEY_COLS = 10;
    
    const char keyboardLower[KEY_ROWS][KEY_COLS] = {
        {'1','2','3','4','5','6','7','8','9','0'},
        {'q','w','e','r','t','y','u','i','o','p'},
        {'a','s','d','f','g','h','j','k','l', 8},
        {'^','z','x','c','v','b','n','m', 26, 13}
    };

    const char keyboardUpper[KEY_ROWS][KEY_COLS] = {
        {'1','2','3','4','5','6','7','8','9','0'},
        {'Q','W','E','R','T','Y','U','I','O','P'},
        {'A','S','D','F','G','H','J','K','L', 8},
        {'^','Z','X','C','V','B','N','M', 26, 13}
    };

    const char keyboardSymbol[KEY_ROWS][KEY_COLS] = {
        {'!','@','#','$','%','^','&','*','(',')'},
        {'_','-','+','=','{','}','[',']','|','\\'},
        {':',';','"','\'','<','>',',','.','?','/'},
        {'~','`','@','#','$','%','&','=', 26, 13}
    };
    
    // Posisi dan ukuran tombol keyboard
    int keyStartX = 40;
    int keyStartY = 100;
    int keyWidth = 37;
    int keyHeight = 37;
    int keySpacing = 4;
    
    // Special keys positions
    int backKeyX = 80, backKeyY = 265, backKeyW = 80, backKeyH = 30;
    int spaceKeyX = 170, spaceKeyY = 265, spaceKeyW = 120, spaceKeyH = 30;
    int clearKeyX = 300, clearKeyY = 265, clearKeyW = 80, clearKeyH = 30;

    // Calibration values
    int touchMinX = 0;
    int touchMaxX = 479;
    int touchMinY = 0;
    int touchMaxY = 289;

    // Variable user list
    std::vector<String> userList;
    int selectedUserIndex = 0;
    int userListPage = 0;
    static const int USER_PER_PAGE = 5;

    // Tombol user list
    int btnPrevPageX = 50, btnPrevPageY = 255, btnPageW = 100, btnPageH = 40;
    int btnNextPageX = 330, btnNextPageY = 255, btnNextPageW = 100, btnNextPageH = 40;
    int btnCancelListX = 190, btnCancelListY = 255, btnCancelListW = 100, btnCancelListH = 40;

    // Tombol tambah/hapus user
    int btnAddUserX = 10, btnAddUserY = 55, btnAddW = 30, btnAddH = 30;
    int btnDeleteUserX = 440, btnDeleteUserY = 55, btnDeleteW = 30, btnDeleteH = 30;

    // WiFi scan list
    std::vector<String> wifiSSIDList;
    std::vector<int> wifiRSSIList;
    std::vector<String> wifiEncTypeList;
    int wifiScanPage = 0;
    static const int WIFI_PER_PAGE = 5;

    // Tombol untuk WiFi scan list
    int wifiBtnPrevPageX = 50, wifiBtnPrevPageY = 255, wifiBtnPageW = 100, wifiBtnPageH = 40;
    int wifiBtnNextPageX = 330, wifiBtnNextPageY = 255, wifiBtnNextPageW = 100, wifiBtnNextPageH = 40;
    int wifiBtnRefreshX = 190, wifiBtnRefreshY = 255, wifiBtnRefreshW = 100, wifiBtnRefreshH = 40;

    // Fungsi menggambar icon
    void drawPlusIcon(int x, int y);
    void drawMinusIcon(int x, int y);
    
    // Fungsi untuk refresh tampilan user list tanpa reset halaman
    void refreshUserListDisplay();
    
    // Fungsi untuk clear area user list
    void clearUserListArea();

    void drawButton(int x, int y, int w, int h, String label, uint16_t color);
    void drawHomeButton(int x, int y, String label, String subtitle, uint16_t color);
    
    // Fungsi checkTouch untuk masing-masing screen
    int checkTouchHome(int screenX, int screenY);
    int checkTouchMeasurement(int screenX, int screenY);
    int checkTouchUserInfo(int screenX, int screenY);
    int checkTouchWifiConfig(int screenX, int screenY);
    int checkTouchEditUser(int screenX, int screenY);
    int checkTouchKeyboard(int screenX, int screenY);
    int checkTouchUserList(int screenX, int screenY);   
    int checkTouchWifiScanList(int screenX, int screenY);    
    int checkTouchModeSelection(int screenX, int screenY);
    
    // Fungsi keyboard
    void drawKeyboard();
    void drawInputField();
    void processKeyPress(char key);

    bool inModeSelection = false;
    int selectedModeIndex = 0;
    std::vector<String> measurementModes;
    static const int MEASUREMENT_MODES_COUNT = 5;

    // Tombol untuk ECG
    int ecgBtnStartX = 20, ecgBtnStartY = 245, ecgBtnW = 200, ecgBtnH = 50;
    int ecgBtnModeX = 240, ecgBtnModeY = 245, ecgBtnModeW = 200, ecgBtnModeH = 50;

    int currentMeasurementMode = 0;

    // Variabel untuk grafik ECG
    static const int ECG_GRAPH_WIDTH = 400;
    static const int ECG_GRAPH_HEIGHT = 150;
    int ecgGraphX = 40;
    int ecgGraphY = 90;
    int lastGraphX = 0;
    int lastGraphY = 0;

    // Buffer untuk data ECG
    int ecgBuffer[ECG_GRAPH_WIDTH];  // Buffer untuk data ECG (sama dengan ECG_GRAPH_WIDTH)
    int ecgBufferIndex = 0;
    bool ecgBufferFull = false;

    uint16_t* graphBackBuffer = nullptr;  // Buffer untuk data grafik
    bool needsGraphRedraw = false;        // Flag untuk redraw grafik

    // Tombol untuk Blood Pressure Interface
    int bpBtnModeX = 190, bpBtnModeY = 255, bpBtnModeW = 100, bpBtnModeH = 40;

    // HAPUS: jangan deklarasi ulang variabel yang sudah ada di bagian public
    // int medBtnStartX = 20, medBtnStartY = 245, medBtnW = 200, medBtnH = 50;
    // int medBtnModeX = 240, medBtnModeY = 245, medBtnModeW = 200, medBtnModeH = 50;
    void drawHeader(String title, String userName = "");

    int emgBtnStartX = 20, emgBtnStartY = 245, emgBtnW = 200, emgBtnH = 50;
    int emgBtnModeX = 240, emgBtnModeY = 245, emgBtnModeW = 200, emgBtnModeH = 50;

    static const int EMG_SIMPLE_BUFFER_SIZE = 400; // Sama dengan ECG
    int emgSimpleBuffer[EMG_SIMPLE_BUFFER_SIZE];
    int emgSimpleBufferIndex = 0;
    bool emgSimpleBufferFull = false;

    // Variabel untuk grafik EMG
    static const int EMG_GRAPH_WIDTH = 400;
    static const int EMG_GRAPH_HEIGHT = 150;
    int emgGraphX = 40;
    int emgGraphY = 90;
    int lastEMGGraphX = 0;
    int lastEMGGraphY = 0;

    // Fungsi untuk grafik EMG
    void drawEMGGraph();
    void updateEMGGraph(int* emgData, int dataSize, int range, String level);

    static const int EMG_BUFFER_SIZE = 200;  // Buffer lebih kecil untuk grafik yang lebih jelas
    int emgBuffer[EMG_BUFFER_SIZE];
    int emgBufferIndex = 0;
    bool emgBufferFull = false;

    void updateEMGGraphFromBuffer(int range, String level);
};

#endif