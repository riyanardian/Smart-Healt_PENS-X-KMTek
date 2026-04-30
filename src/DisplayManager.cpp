#include "DisplayManager.h"

void DisplayManager::begin() {
    // Inisialisasi TFT
    tft.init();
    tft.setRotation(1); // Landscape
    tft.fillScreen(TFT_BG_COLOR);
    
    // Inisialisasi I2C untuk touchscreen
    Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
    delay(100);
    
    // Init Touch (I2C)
    if (!ts.begin(40, &Wire)) {
        Serial.println("Touchscreen tidak terdeteksi!");
        touchSupported = false;
    } else {
        Serial.println("Touchscreen OK.");
        touchSupported = true;
    }

    // Header dengan tema baru
    tft.fillRect(0, 0, 480, 50, TFT_HEADER_COLOR);
    tft.setTextColor(TFT_BG_COLOR, TFT_HEADER_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Smart Health PENS", 240, 25);

    // Inisialisasi mode pengukuran
    measurementModes.clear();
    measurementModes.push_back("Medical Record");
    measurementModes.push_back("Blood Pressure");
    measurementModes.push_back("ECG");
    measurementModes.push_back("EMG");
    measurementModes.push_back("Body Position & Snore");
}

bool DisplayManager::showOnScreenConfirmation(String title, String message, String yesBtn, String noBtn) {
    // Simpan screen mode sebelumnya
    int prevScreenMode = currentScreenMode;
    currentScreenMode = 99;

    clearContentArea();

    // Title
    drawTextCentered(title, 240, 100, TFT_ACCENT_ORANGE, TFT_BG_COLOR, 2);

    // Message
    int yPos = 140;
    int startPos = 0;
    int newlinePos;

    do {
        newlinePos = message.indexOf('\n', startPos);
        String line;
        if (newlinePos == -1) {
            line = message.substring(startPos);
        } else {
            line = message.substring(startPos, newlinePos);
            startPos = newlinePos + 1;
        }

        // Jika terlalu panjang
        if (line.length() > 30) {
            String part1 = line.substring(0, 30);
            String part2 = line.substring(30);
            drawTextCentered(part1, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
            yPos += 20;
            drawTextCentered(part2, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
        } else {
            drawTextCentered(line, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
        }
        yPos += 20;
    } while (newlinePos != -1 && yPos < 200);

    // Buttons
    drawButton(140, 200, 80, 40, yesBtn, TFT_ACCENT_GREEN);
    drawButton(260, 200, 80, 40, noBtn, TFT_ACCENT_RED);

    updateStatus("Sentuh YA atau TIDAK");

    // Tunggu input
    unsigned long startTime = millis();
    while (millis() - startTime < 15000) {
        if (!touchSupported || !ts.touched()) {
            delay(50);
            continue;
        }

        TS_Point p = ts.getPoint();
        int screenX = map(p.y, touchMinX, touchMaxX, 0, 480);
        int screenY = map(p.x, touchMinY, touchMaxY, 320, 0);

        screenX = constrain(screenX, 0, 479);
        screenY = constrain(screenY, 0, 319);

        // Debounce
        static unsigned long lastTouchTime = 0;
        unsigned long now = millis();
        if (now - lastTouchTime < 200) continue;
        lastTouchTime = now;

        // Cek button area
        if (screenX > 140 && screenX < 220 && screenY > 200 && screenY < 240) {
            // Restore screen mode
            currentScreenMode = prevScreenMode;
            return true;
        } else if (screenX > 260 && screenX < 340 && screenY > 200 && screenY < 240) {
            // Restore screen mode
            currentScreenMode = prevScreenMode;
            return false;
        }
    }

    // Restore screen mode
    currentScreenMode = prevScreenMode;
    return false;
}

bool DisplayManager::showConfirmationDialogSerial(String title, String message) {
    // Simpan screen mode sebelumnya
    int prevScreenMode = currentScreenMode;
    currentScreenMode = 99; // Mode khusus untuk dialog konfirmasi
    
    clearContentArea();
    
    // Title
    drawTextCentered(title, 240, 100, TFT_ACCENT_ORANGE, TFT_BG_COLOR, 2);
    
    // Message (bisa multi-line)
    int yPos = 140;
    int startPos = 0;
    int newlinePos;
    
    do {
        newlinePos = message.indexOf('\n', startPos);
        String line;
        if (newlinePos == -1) {
            line = message.substring(startPos);
        } else {
            line = message.substring(startPos, newlinePos);
            startPos = newlinePos + 1;
        }
        
        drawTextCentered(line, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
        yPos += 20;
    } while (newlinePos != -1 && yPos < 200);
    
    // Buttons
    drawButton(140, 180, 80, 40, "YA", TFT_ACCENT_GREEN);
    drawButton(260, 180, 80, 40, "TIDAK", TFT_ACCENT_RED);
    
    updateStatus("Sentuh YA atau TIDAK (15 detik)");
    
    // Tunggu input touch
    unsigned long startTime = millis();
    while (millis() - startTime < 15000) { // 15 detik timeout
        if (!touchSupported || !ts.touched()) {
            delay(50);
            continue;
        }
        
        TS_Point p = ts.getPoint();
        int screenX = map(p.y, touchMinX, touchMaxX, 0, 480);
        int screenY = map(p.x, touchMinY, touchMaxY, 320, 0);
        
        screenX = constrain(screenX, 0, 479);
        screenY = constrain(screenY, 0, 319);
        
        // Debounce
        static unsigned long lastTouchTime = 0;
        unsigned long now = millis();
        if (now - lastTouchTime < 200) continue;
        lastTouchTime = now;
        
        // Check button areas
        if (screenX > 140 && screenX < 220 && screenY > 180 && screenY < 220) {
            // Restore screen mode
            currentScreenMode = prevScreenMode;
            return true; // Button YA
        } else if (screenX > 260 && screenX < 340 && screenY > 180 && screenY < 220) {
            // Restore screen mode
            currentScreenMode = prevScreenMode;
            return false; // Button TIDAK
        }
    }
    
    // Restore screen mode
    currentScreenMode = prevScreenMode;
    return false; // Timeout (default TIDAK)
}

void DisplayManager::drawTextCentered(String text, int x, int y, uint16_t color, uint16_t bgColor, int textSize) {
    tft.setTextColor(color, bgColor);
    tft.setTextSize(textSize);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(text, x, y);
}

void DisplayManager::drawButtonPublic(int x, int y, int w, int h, String label, uint16_t color) {
    drawButton(x, y, w, h, label, color);
}

void DisplayManager::showConfirmationDialog(String title, String message, String btnYesLabel, String btnNoLabel) {
    clearContentArea();
    
    // Title
    drawTextCentered(title, 240, 100, TFT_ACCENT_ORANGE, TFT_BG_COLOR, 2);
    
    // Message
    drawTextCentered(message, 240, 140, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
    
    // Buttons
    drawButton(140, 180, 80, 40, btnYesLabel, TFT_ACCENT_RED);
    drawButton(260, 180, 80, 40, btnNoLabel, TFT_ACCENT_GREEN);
    
    updateStatus("Konfirmasi pilihan");
}

int DisplayManager::getTouchInput(int timeoutMs) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeoutMs) {
        if (!touchSupported || !ts.touched()) {
            delay(50);
            continue;
        }
        
        TS_Point p = ts.getPoint();
        
        // Mapping koordinat
        int screenX = map(p.y, touchMinX, touchMaxX, 0, 480);
        int screenY = map(p.x, touchMinY, touchMaxY, 320, 0);
        
        screenX = constrain(screenX, 0, 479);
        screenY = constrain(screenY, 0, 319);
        
        // Debounce
        static unsigned long lastTouchTime = 0;
        unsigned long now = millis();
        if (now - lastTouchTime < 200) continue;
        lastTouchTime = now;
        
        // Check button areas
        if (screenX > 140 && screenX < 220 && screenY > 180 && screenY < 220) {
            return 1; // Button YES
        } else if (screenX > 260 && screenX < 340 && screenY > 180 && screenY < 220) {
            return 2; // Button NO
        }
    }
    
    return 0; // Timeout
}

void DisplayManager::clearContentArea() {
    // Bersihkan area konten utama dari y=50 sampai y=319 (seluruh area kecuali header)
    tft.fillRect(0, 50, 480, 270, TFT_BG_COLOR);
    
    // Juga bersihkan area status bar jika ada
    tft.fillRect(0, 300, 480, 20, TFT_BG_COLOR);
}

bool DisplayManager::showSingleButtonDialog(String title, String message, String btnLabel) {
    // Simpan screen mode sebelumnya
    int prevScreenMode = currentScreenMode;
    currentScreenMode = 99; // Mode dialog tunggal

    clearContentArea();

    // Title
    drawTextCentered(title, 240, 100, TFT_ACCENT_ORANGE, TFT_BG_COLOR, 2);

    // Message
    int yPos = 140;
    int startPos = 0;
    int newlinePos;

    do {
        newlinePos = message.indexOf('\n', startPos);
        String line;
        if (newlinePos == -1) {
            line = message.substring(startPos);
        } else {
            line = message.substring(startPos, newlinePos);
            startPos = newlinePos + 1;
        }

        // Jika terlalu panjang
        if (line.length() > 30) {
            String part1 = line.substring(0, 30);
            String part2 = line.substring(30);
            drawTextCentered(part1, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
            yPos += 20;
            drawTextCentered(part2, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
        } else {
            drawTextCentered(line, 240, yPos, TFT_TEXT_COLOR, TFT_BG_COLOR, 1);
        }
        yPos += 20;
    } while (newlinePos != -1 && yPos < 200);

    // Single button
    drawButton(190, 200, 100, 40, btnLabel, TFT_ACCENT_GREEN);
    updateStatus("Sentuh tombol untuk lanjut");

    // Tunggu input
    unsigned long startTime = millis();
    while (millis() - startTime < 15000) {
        if (!touchSupported || !ts.touched()) {
            delay(50);
            continue;
        }

        TS_Point p = ts.getPoint();
        int screenX = map(p.y, touchMinX, touchMaxX, 0, 480);
        int screenY = map(p.x, touchMinY, touchMaxY, 320, 0);

        screenX = constrain(screenX, 0, 479);
        screenY = constrain(screenY, 0, 319);

        // Debounce
        static unsigned long lastTouchTime = 0;
        unsigned long now = millis();
        if (now - lastTouchTime < 200) continue;
        lastTouchTime = now;

        // Cek button area
        if (screenX > 190 && screenX < 290 && screenY > 200 && screenY < 240) {
            // Restore screen mode
            currentScreenMode = prevScreenMode;
            return true;
        }
    }

    // Restore screen mode (timeout)
    currentScreenMode = prevScreenMode;
    return false;
}

// FUNGSI BARU: Bersihkan area user list khusus
void DisplayManager::clearUserListArea() {
    // Hapus area daftar user (40, 120 sampai 440, 240)
    tft.fillRect(40, 100, 400, 155, TFT_BG_COLOR);
    
    // Hapus area tombol navigasi halaman
    tft.fillRect(50, 255, 100, 40, TFT_BG_COLOR);  // Tombol sebelumnya
    tft.fillRect(330, 255, 100, 40, TFT_BG_COLOR); // Tombol berikut
    tft.fillRect(190, 255, 100, 40, TFT_BG_COLOR); // Tombol kembali
}

void DisplayManager::clearScreen() {
    tft.fillScreen(TFT_BG_COLOR);
    tft.fillRect(0, 0, 480, 50, TFT_HEADER_COLOR);
    tft.setTextColor(TFT_BG_COLOR, TFT_HEADER_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Smart Health PENS", 240, 25);
}

void DisplayManager::drawHomeScreen(String currentDeviceName, bool isWiFiConnected) {
    clearContentArea();
    currentScreenMode = 0; // Set mode ke HOME
    
    // Judul Home Screen
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Home Screen", 240, 55);
    
    // Gambar 4 tombol besar
    // 1. Tombol User Sekarang
    String userSubtitle = (currentDeviceName == "BELUM ADA USER") ? "Tidak ada user" : currentDeviceName;
    if (userSubtitle.length() > 20) userSubtitle = userSubtitle.substring(0, 17) + "...";
    drawHomeButton(btnUserX, btnUserY, "USER", userSubtitle, TFT_ACCENT_BLUE);
    
    // 2. Tombol Pengukuran
    drawHomeButton(btnMeasureX, btnMeasureY, "PENGUKURAN", "Mulai/Stop", TFT_ACCENT_CYAN);
    
    // 3. Tombol Ganti User
    drawHomeButton(btnChangeX, btnChangeY, "GANTI USER", "Pilih user lain", TFT_ACCENT_ORANGE);
    
    // 4. Tombol WiFi
    String wifiStatus = isWiFiConnected ? "Tersambung" : "Putus";
    uint16_t wifiColor = isWiFiConnected ? TFT_ACCENT_GREEN : TFT_ACCENT_RED;
    drawHomeButton(btnWifiX, btnWifiY, "WIFI", wifiStatus, wifiColor);
    
    // Footer informasi
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Sentuh tombol untuk memilih", 240, 280);
}

void DisplayManager::drawHomeButton(int x, int y, String label, String subtitle, uint16_t color) {
    // Background tombol
    tft.fillRoundRect(x, y, btnWidth, btnHeight, 15, color);
    tft.drawRoundRect(x, y, btnWidth, btnHeight, 15, TFT_BUTTON_BORDER);
    
    // Label utama (besar)
    tft.setTextColor(TFT_BG_COLOR, color);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(label, x + (btnWidth/2), y + 20);
    
    // Subtitle (kecil)
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    
    // Jika subtitle terlalu panjang, potong
    if (subtitle.length() > 20) {
        subtitle = subtitle.substring(0, 17) + "...";
    }
    
    tft.drawString(subtitle, x + (btnWidth/2), y + 45);
}

void DisplayManager::drawInterface(String currentDeviceName) {
    clearContentArea();
    currentScreenMode = 1; // Set mode ke MEASUREMENT
    
    // Label Nama Device
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("User: " + currentDeviceName, 20, 60, 4); 

    // Tombol Start/Stop
    drawButton(btnStartX, btnStartY, btnW, btnH, "START/STOP", TFT_ACCENT_GREEN);
    
    // Tombol Ganti User
    drawButton(btnNextX, btnNextY, btnW, btnH, "GANTI USER", TFT_ACCENT_BLUE);
    
    // TOMBOL BARU: Ganti Mode (di samping tombol Ganti User)
    drawButton(btnNextX + 150, btnNextY, btnW, btnH, "GANTI MODE", TFT_ACCENT_PURPLE);
}

void DisplayManager::drawECGInterface(String currentDeviceName) {
    clearContentArea();
    currentScreenMode = 1;

    // ==================== LAYOUT ECG YANG DIPERBAIKI ====================
    
    // --- JUDUL ECG MONITOR (TETAP ADA, TURUNKAN POSISI) ---
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);  // Warna biru/cyan seperti Medical Record
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("ECG MONITOR", 240, 60); // Turunkan ke y=60 (dari 40)
    
    // --- GRAFIK ECG (POSISI DI BAWAH JUDUL) ---
    // ecgGraphY = 90; // Tetap 90 karena sudah sesuai di header
    int defaultThreshold = 2500; // Threshold default
    drawECGGraph(nullptr, 0, defaultThreshold);
    
    // --- TOMBOL (NAIKKAN 2px dari posisi sebelumnya) ---
    // Tombol ECG menggunakan posisi yang sama dengan Medical Record
    ecgBtnStartY = 245; // Naikkan 2px dari 247 ke 245
    ecgBtnModeY = 245;  // Naikkan 2px dari 247 ke 245
    
    drawButton(ecgBtnStartX, ecgBtnStartY, ecgBtnW, ecgBtnH, "START/STOP", TFT_ACCENT_GREEN);
    drawButton(ecgBtnModeX, ecgBtnModeY, ecgBtnModeW, ecgBtnModeH, "GANTI MODE", TFT_ACCENT_PURPLE);
    
    // Status bar - tanpa info user
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Mode: ECG Monitor (AD8232)", 240, 320);
}

static const int ECG_BUFFER_SIZE = 400; // Sama dengan ECG_GRAPH_WIDTH
int ecgBuffer[ECG_BUFFER_SIZE];
int ecgBufferIndex = 0;
bool ecgBufferFull = false;

void DisplayManager::drawECGGraph(int* ecgData, int dataSize, int threshold) {
    // Reset buffer dan indeks saat pertama kali menggambar
    ecgBufferIndex = 0;
    ecgBufferFull = false;
    memset(ecgBuffer, 0, sizeof(ecgBuffer));
    
    // Bersihkan area grafik
    tft.fillRect(ecgGraphX, ecgGraphY, ECG_GRAPH_WIDTH, ECG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // Gambar border grafik
    tft.drawRect(ecgGraphX, ecgGraphY, ECG_GRAPH_WIDTH, ECG_GRAPH_HEIGHT, TFT_TEXT_COLOR);
    
    // Gambar grid horizontal (setiap 25 piksel)
    for (int y = ecgGraphY; y <= ecgGraphY + ECG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(ecgGraphX, y, ECG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    
    // Gambar grid vertikal (setiap 50 piksel)
    for (int x = ecgGraphX; x <= ecgGraphX + ECG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, ecgGraphY, ECG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    
    // Gambar garis threshold (skala 0-2000)
    int thresholdY = map(constrain(threshold, 0, 3000), 0, 3000, 
                         ecgGraphY + ECG_GRAPH_HEIGHT, ecgGraphY);
    tft.drawFastHLine(ecgGraphX, thresholdY, ECG_GRAPH_WIDTH, TFT_ACCENT_RED);
    
    // Reset last position
    lastGraphX = ecgGraphX;
    lastGraphY = ecgGraphY + (ECG_GRAPH_HEIGHT / 2);
}

void DisplayManager::updateECGGraph(int* ecgData, int dataSize, int threshold, int heartRate) {
    // Jika data terlalu sedikit, tunggu
    if (dataSize < 2) return;
    
    // 1. HAPUS SELURUH AREA GRAFIK
    tft.fillRect(ecgGraphX, ecgGraphY, ECG_GRAPH_WIDTH, ECG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // 2. Gambar grid horizontal
    for (int y = ecgGraphY; y <= ecgGraphY + ECG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(ecgGraphX, y, ECG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    
    // 3. Gambar grid vertikal
    for (int x = ecgGraphX; x <= ecgGraphX + ECG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, ecgGraphY, ECG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    
    // 4. Gambar border grafik ULANG
    tft.drawRect(ecgGraphX, ecgGraphY, ECG_GRAPH_WIDTH, ECG_GRAPH_HEIGHT, TFT_TEXT_COLOR);
    
    // 5. Gambar garis threshold
    int thresholdY = map(constrain(threshold, 0, 2000), 0, 2000, 
                         ecgGraphY + ECG_GRAPH_HEIGHT, ecgGraphY);
    tft.drawFastHLine(ecgGraphX, thresholdY, ECG_GRAPH_WIDTH, TFT_ACCENT_RED);
    
    // 6. Proses data ECG ke dalam buffer
    for (int i = 0; i < dataSize; i++) {
        int clampedValue = constrain(ecgData[i], 0, 3000);
        ecgBuffer[ecgBufferIndex] = clampedValue;
        ecgBufferIndex = (ecgBufferIndex + 1) % ECG_GRAPH_WIDTH;
        if (!ecgBufferFull && ecgBufferIndex == 0) ecgBufferFull = true;
    }
    
    // 7. Gambar grafik dari buffer
    int startX = ecgGraphX;
    int pointsToDraw = ecgBufferFull ? ECG_GRAPH_WIDTH : ecgBufferIndex;
    int displayedPoints = min(pointsToDraw, ECG_GRAPH_WIDTH / 2);
    
    for (int i = 1; i < displayedPoints; i++) {
        int idx1 = (ecgBufferIndex - displayedPoints + i - 1 + ECG_GRAPH_WIDTH) % ECG_GRAPH_WIDTH;
        int idx2 = (ecgBufferIndex - displayedPoints + i + ECG_GRAPH_WIDTH) % ECG_GRAPH_WIDTH;
        
        int x1 = startX + (i - 1) * 2;
        int x2 = startX + i * 2;
        
        int y1 = map(ecgBuffer[idx1], 0, 3000, ecgGraphY + ECG_GRAPH_HEIGHT, ecgGraphY);
        int y2 = map(ecgBuffer[idx2], 0, 3000, ecgGraphY + ECG_GRAPH_HEIGHT, ecgGraphY);
        
        drawThickLine(x1, y1, x2, y2, TFT_ACCENT_GREEN, 2);
    }
    
    // ================= MODIFIKASI UKURAN KOTAK (LEBAR DIKECILKAN) =================
    
    // Setting Font
    tft.setTextFont(2); 
    tft.setTextSize(1); 

    int infoY = ecgGraphY - 18; // Posisi Y (tetap seperti sebelumnya)

    // 8. Tampilkan Heart Rate
    // UBAH DISINI: Lebar (parameter ke-3) diganti dari 150 menjadi 100
    // (100px sudah cukup untuk "HR: 120 BPM" dengan font kecil)
    tft.fillRect(ecgGraphX, infoY, 100, 14, TFT_BG_COLOR); 
    
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextDatum(TL_DATUM); 
    
    if (heartRate > 0) {
        tft.drawString("HR: " + String(heartRate) + " BPM", ecgGraphX, infoY + 1); 
    } else {
        tft.drawString("HR: --- BPM", ecgGraphX, infoY + 1);
    }
    
    // 9. Tampilkan Threshold
    // UBAH DISINI: Kita kecilkan juga lebarnya jadi 80 (biar tidak terlalu makan tempat ke kiri)
    // Perhatikan: Rumus X awalnya (ecgGraphX + ECG_GRAPH_WIDTH - 100)
    // Sekarang diganti jadi (... - 80) agar pas dengan lebar kotak barunya.
    int thWidth = 80; 
    tft.fillRect(ecgGraphX + ECG_GRAPH_WIDTH - thWidth, infoY, thWidth, 14, TFT_BG_COLOR); 
    
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextDatum(TR_DATUM); 
    
    int displayThreshold = constrain(threshold, 0, 2000);
    tft.drawString("Th: " + String(displayThreshold), ecgGraphX + ECG_GRAPH_WIDTH, infoY + 1);

    // Kembalikan ke Font Standar
    tft.setTextFont(1);
}

void DisplayManager::drawThickLine(int x1, int y1, int x2, int y2, uint16_t color, int thickness) {
    // Untuk garis horizontal (y1 == y2) atau hampir horizontal
    if (abs(y2 - y1) < abs(x2 - x1)) {
        // Gambar beberapa garis horizontal untuk ketebalan
        int halfThickness = thickness / 2;
        for (int i = -halfThickness; i <= halfThickness; i++) {
            tft.drawLine(x1, y1 + i, x2, y2 + i, color);
        }
    } 
    // Untuk garis vertikal (x1 == x2) atau hampir vertikal
    else {
        // Gambar beberapa garis vertikal untuk ketebalan
        int halfThickness = thickness / 2;
        for (int i = -halfThickness; i <= halfThickness; i++) {
            tft.drawLine(x1 + i, y1, x2 + i, y2, color);
        }
    }
}

// FUNGSI BLOOD PRESSURE INTERFACE
void DisplayManager::drawBloodPressureInterface(String currentDeviceName) {
    clearContentArea();
    currentScreenMode = 1;
    currentMeasurementMode = 1; // Set mode ke Blood Pressure

    // ==================== LAYOUT BLOOD PRESSURE ====================
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("BLOOD PRESSURE MONITOR", 240, 60);
    
    // --- KOLOM KIRI (SISTOLIK) ---
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SISTOLIK", 50, 100);
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 50, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("mmHg", 50, 170);
    
    // --- KOLOM TENGAH (DIASTOLIK) ---
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("DIASTOLIK", 190, 100);
    tft.setTextColor(TFT_ACCENT_BLUE, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 190, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("mmHg", 190, 170);
    
    // --- KOLOM KANAN (HEART RATE) ---
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("HEART RATE", 330, 100);
    tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 330, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("BPM", 330, 170);
    
    // Tanda pemisah (/) antara sistolik dan diastolik
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(4);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("/", 240, 140);
    
    // Instruksi
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Tekan tombol di alat tensimeter", 240, 200);
    tft.drawString("untuk mulai pengukuran", 240, 215);
    
    // Hanya tombol GANTI MODE (tanpa START/STOP)
    // Posisi tombol di tengah bawah
    drawButton(190, 255, 100, 40, "GANTI MODE", TFT_ACCENT_PURPLE);
    
    // Status bar
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Mode: Blood Pressure - Siap menerima data", 240, 320);
}

void DisplayManager::updateBloodPressureData(int systolic, int diastolic, int heartRate, bool isSending) {
    // Hapus area angka lama
    tft.fillRect(50, 120, 100, 50, TFT_BG_COLOR);   // Area sistolik
    tft.fillRect(190, 120, 100, 50, TFT_BG_COLOR);  // Area diastolik
    tft.fillRect(330, 120, 100, 50, TFT_BG_COLOR);  // Area heart rate
    
    // --- KOLOM KIRI: SISTOLIK ---
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.setTextDatum(TL_DATUM);
    if (systolic > 0) {
        tft.drawString(String(systolic), 50, 120);
    } else {
        tft.drawString("---", 50, 120);
    }
    
    // --- KOLOM TENGAH: DIASTOLIK ---
    tft.setTextColor(TFT_ACCENT_BLUE, TFT_BG_COLOR);
    tft.setTextSize(3);
    if (diastolic > 0) {
        tft.drawString(String(diastolic), 190, 120);
    } else {
        tft.drawString("---", 190, 120);
    }
    
    // --- KOLOM KANAN: HEART RATE ---
    tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
    tft.setTextSize(3);
    if (heartRate > 0) {
        tft.drawString(String(heartRate), 330, 120);
    } else {
        tft.drawString("---", 330, 120);
    }
    
    // Update instruksi berdasarkan status
    tft.fillRect(50, 195, 380, 40, TFT_BG_COLOR);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    
    if (systolic > 0 && diastolic > 0) {
        // HAPUS KATEGORI - Hanya tampilkan pesan sukses
        tft.drawString("Data telah tersimpan ke cloud", 240, 200);
        tft.drawString("Sentuh tombol GANTI MODE untuk kembali", 240, 215);
    } else {
        tft.drawString("Tekan tombol di alat tensimeter", 240, 200);
        tft.drawString("untuk mulai pengukuran", 240, 215);
    }
    
    // Indikator Upload (lingkaran kecil di pojok kanan atas)
    if (isSending) {
        tft.fillCircle(460, 70, 8, TFT_ACCENT_RED);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    } else {
        tft.fillCircle(460, 70, 8, TFT_BG_COLOR);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    }
}

void DisplayManager::drawHeader(String title, String userName) {
    // Bersihkan area header
    tft.fillRect(0, 0, 480, 50, TFT_HEADER_COLOR);
    
    // Gambar judul
    tft.setTextColor(TFT_BG_COLOR, TFT_HEADER_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(title, 240, 25);
    
    // Jika ada nama user, tampilkan di pojok kanan
    if (userName.length() > 0) {
        tft.setTextSize(1);
        tft.setTextDatum(MR_DATUM);
        String displayName = userName;
        if (displayName.length() > 15) {
            displayName = displayName.substring(0, 12) + "...";
        }
        tft.drawString("User: " + displayName, 470, 25);
    }
}

void DisplayManager::drawEMGInterface(String userName) {
    clearContentArea();
    currentScreenMode = 1;
    currentMeasurementMode = 3; // Set ke mode EMG

    // Judul Mode
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("EMG MONITOR", 240, 60);

    // Gambar grafik sederhana (seperti ECG)
    drawEMGGraphSimple();
    
    // Tombol di bagian bawah
    drawButton(emgBtnStartX, emgBtnStartY, emgBtnW, emgBtnH, "START/STOP", TFT_ACCENT_GREEN);
    drawButton(emgBtnModeX, emgBtnModeY, emgBtnModeW, medBtnModeH, "GANTI MODE", TFT_ACCENT_PURPLE);
    
    // Status bar di paling bawah
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Mode: EMG Monitor", 240, 320);
}

void DisplayManager::drawEMGGraph() {
    // Bersihkan area grafik
    tft.fillRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // Gambar border grafik
    tft.drawRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_TEXT_COLOR);
    
    // Gambar grid horizontal (setiap 25 piksel)
    for (int y = emgGraphY; y <= emgGraphY + EMG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(emgGraphX, y, EMG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    
    // Gambar grid vertikal (setiap 50 piksel)
    for (int x = emgGraphX; x <= emgGraphX + EMG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, emgGraphY, EMG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    
    // Reset last position
    lastEMGGraphX = emgGraphX;
    lastEMGGraphY = emgGraphY + (EMG_GRAPH_HEIGHT / 2);
}

void DisplayManager::updateEMGGraphFromBuffer(int range, String level) {
    // 1. Bersihkan area grafik
    tft.fillRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // 2. Gambar ulang grid dan border
    for (int y = emgGraphY; y <= emgGraphY + EMG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(emgGraphX, y, EMG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    for (int x = emgGraphX; x <= emgGraphX + EMG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, emgGraphY, EMG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    tft.drawRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_TEXT_COLOR);

    // 3. Tentukan jumlah data yang akan ditampilkan (Maksimal 200 atau lebar grafik)
    int currentSize = emgBufferFull ? EMG_BUFFER_SIZE : emgBufferIndex;
    int displayPoints = min(currentSize, 200); 
    
    int startDataIndex = 0;
    if (emgBufferFull) {
        startDataIndex = (emgBufferIndex - displayPoints + EMG_BUFFER_SIZE) % EMG_BUFFER_SIZE;
    }
    
    // 4. Cari min/max untuk auto-scaling
    int minVal = 4095, maxVal = 0;
    for (int i = 0; i < displayPoints; i++) {
        int idx = (startDataIndex + i) % EMG_BUFFER_SIZE;
        int val = emgBuffer[idx];
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    // Rentang minimal agar grafik tidak flat
    if (maxVal - minVal < 100) {
        int mid = (minVal + maxVal) / 2;
        minVal = max(0, mid - 250);
        maxVal = min(4095, mid + 250);
    }

    // 5. Gambar grafik
    for (int i = 1; i < displayPoints; i++) {
        int idx1 = (startDataIndex + i - 1) % EMG_BUFFER_SIZE;
        int idx2 = (startDataIndex + i) % EMG_BUFFER_SIZE;
        int x1 = emgGraphX + (i-1) * 2;
        int x2 = emgGraphX + i * 2;
        
        int margin = (maxVal - minVal) * 0.1;
        int y1 = map(emgBuffer[idx1], minVal - margin, maxVal + margin, emgGraphY + EMG_GRAPH_HEIGHT, emgGraphY);
        int y2 = map(emgBuffer[idx2], minVal - margin, maxVal + margin, emgGraphY + EMG_GRAPH_HEIGHT, emgGraphY);
        
        y1 = constrain(y1, emgGraphY, emgGraphY + EMG_GRAPH_HEIGHT);
        y2 = constrain(y2, emgGraphY, emgGraphY + EMG_GRAPH_HEIGHT);

        drawThickLine(x1, y1, x2, y2, TFT_ACCENT_GREEN, 2);
    }

    // 6. Update Label Amp & Act (Gaya ECG di atas grafik)
    tft.setTextFont(2);
    tft.setTextSize(1);
    int infoY = emgGraphY - 18;

    // PERUBAHAN: Label Amp (Amplitudo dalam mV)
    tft.fillRect(emgGraphX, infoY, 100, 14, TFT_BG_COLOR);
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextDatum(TL_DATUM);
    
    // Konversi range ke mV (asumsi: 0.8 mV per count)
    float amplitude_mV = range * 0.8f;  // Sesuaikan dengan konversi di EMGSensor.cpp
    if (amplitude_mV > 3000.0f) amplitude_mV = 3000.0f;  // Maksimal 3000 mV
    tft.drawString("Amp: " + String(amplitude_mV, 1) + " mV", emgGraphX, infoY + 1);  // PERUBAHAN

    // PERUBAHAN: Label Act (Persentase Aktivasi)
    int actWidth = 80;
    tft.fillRect(emgGraphX + EMG_GRAPH_WIDTH - actWidth, infoY, actWidth, 14, TFT_BG_COLOR);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextDatum(TR_DATUM);
    
    // Konversi ke persentase (asumsi: 100% = 1000 mV = 1V)
    int activation_percent = (int)((amplitude_mV / 1000.0f) * 100.0f);
    if (activation_percent < 0) activation_percent = 0;
    if (activation_percent > 100) activation_percent = 100;
    tft.drawString("Act: " + String(activation_percent) + "%", emgGraphX + EMG_GRAPH_WIDTH, infoY + 1);  // PERUBAHAN

    tft.setTextFont(1);
}

void DisplayManager::updateEMGGraph(int* emgData, int dataSize, int range, String level) {
    // Jika ada data, tambahkan ke buffer
    if (emgData != nullptr && dataSize > 0) {
        for (int i = 0; i < dataSize; i++) {
            // Simpan data ke buffer dengan konstrain
            int value = constrain(emgData[i], 0, 4095);
            emgBuffer[emgBufferIndex] = value;
            emgBufferIndex = (emgBufferIndex + 1) % EMG_BUFFER_SIZE;
            
            // Tandai buffer penuh
            if (!emgBufferFull && emgBufferIndex == 0) {
                emgBufferFull = true;
            }
        }
        
        // Gambar dari buffer
        updateEMGGraphFromBuffer(range, level);
    }
}

void DisplayManager::updateEMGData(int range, float voltage, String level, bool isSending, int* emgData, int dataSize) {
    // Hitung amplitude dan activation dari EMG sensor
    float amplitude_mV = range * 0.8f;  // 0.8 mV per count
    int activation_percent = (int)((amplitude_mV / 1000.0f) * 100.0f);
    
    // Update grafik sederhana
    if (emgData != nullptr && dataSize > 0) {
        updateEMGGraphSimple(emgData, dataSize, amplitude_mV, activation_percent);
    }
    
    // Indikator Upload
    if (isSending) {
        tft.fillCircle(460, 70, 8, TFT_ACCENT_RED);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    } else {
        tft.fillCircle(460, 70, 8, TFT_BG_COLOR);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    }
}

void DisplayManager::drawEMGGraphSimple() {
    // Reset buffer dan indeks
    emgSimpleBufferIndex = 0;
    emgSimpleBufferFull = false;
    memset(emgSimpleBuffer, 0, sizeof(emgSimpleBuffer));
    
    // Bersihkan area grafik
    tft.fillRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // Gambar border grafik
    tft.drawRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_TEXT_COLOR);
    
    // Gambar grid horizontal (setiap 25 piksel)
    for (int y = emgGraphY; y <= emgGraphY + EMG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(emgGraphX, y, EMG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    
    // Gambar grid vertikal (setiap 50 piksel)
    for (int x = emgGraphX; x <= emgGraphX + EMG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, emgGraphY, EMG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    
    // Reset last position
    lastEMGGraphX = emgGraphX;
    lastEMGGraphY = emgGraphY + (EMG_GRAPH_HEIGHT / 2);
}

void DisplayManager::updateEMGGraphSimple(int* emgData, int dataSize, float amplitude, int activationPercent) {
    // Jika data terlalu sedikit, tunggu
    if (dataSize < 2) return;
    
    // 1. HAPUS SELURUH AREA GRAFIK
    tft.fillRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_BG_COLOR);
    
    // 2. Gambar grid horizontal
    for (int y = emgGraphY; y <= emgGraphY + EMG_GRAPH_HEIGHT; y += 25) {
        tft.drawFastHLine(emgGraphX, y, EMG_GRAPH_WIDTH, TFT_BUTTON_BORDER);
    }
    
    // 3. Gambar grid vertikal
    for (int x = emgGraphX; x <= emgGraphX + EMG_GRAPH_WIDTH; x += 50) {
        tft.drawFastVLine(x, emgGraphY, EMG_GRAPH_HEIGHT, TFT_BUTTON_BORDER);
    }
    
    // 4. Gambar border grafik ULANG
    tft.drawRect(emgGraphX, emgGraphY, EMG_GRAPH_WIDTH, EMG_GRAPH_HEIGHT, TFT_TEXT_COLOR);
    
    // 5. Proses data EMG ke dalam buffer sederhana
    for (int i = 0; i < dataSize; i++) {
        // Normalisasi data ke 0-3000 seperti ECG
        int clampedValue = constrain(emgData[i], 0, 3000);
        emgSimpleBuffer[emgSimpleBufferIndex] = clampedValue;
        emgSimpleBufferIndex = (emgSimpleBufferIndex + 1) % EMG_SIMPLE_BUFFER_SIZE;
        if (!emgSimpleBufferFull && emgSimpleBufferIndex == 0) {
            emgSimpleBufferFull = true;
        }
    }
    
    // 6. Gambar grafik dari buffer sederhana
    int startX = emgGraphX;
    int pointsToDraw = emgSimpleBufferFull ? EMG_SIMPLE_BUFFER_SIZE : emgSimpleBufferIndex;
    int displayedPoints = min(pointsToDraw, EMG_SIMPLE_BUFFER_SIZE / 2);
    
    for (int i = 1; i < displayedPoints; i++) {
        int idx1 = (emgSimpleBufferIndex - displayedPoints + i - 1 + EMG_SIMPLE_BUFFER_SIZE) % EMG_SIMPLE_BUFFER_SIZE;
        int idx2 = (emgSimpleBufferIndex - displayedPoints + i + EMG_SIMPLE_BUFFER_SIZE) % EMG_SIMPLE_BUFFER_SIZE;
        
        int x1 = startX + (i - 1) * 2;
        int x2 = startX + i * 2;
        
        // Map ke 0-3000 untuk konsistensi dengan ECG
        int y1 = map(emgSimpleBuffer[idx1], 0, 3000, emgGraphY + EMG_GRAPH_HEIGHT, emgGraphY);
        int y2 = map(emgSimpleBuffer[idx2], 0, 3000, emgGraphY + EMG_GRAPH_HEIGHT, emgGraphY);
        
        // Gambar garis hijau tebal seperti ECG
        drawThickLine(x1, y1, x2, y2, TFT_ACCENT_GREEN, 2);
    }
    
    // 7. Tampilkan informasi di atas grafik (seperti ECG)
    tft.setTextFont(2); 
    tft.setTextSize(1); 

    int infoY = emgGraphY - 18; // Posisi Y di atas grafik

    // Tampilkan Amplitude (kiri atas)
    tft.fillRect(emgGraphX, infoY, 100, 14, TFT_BG_COLOR); 
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextDatum(TL_DATUM); 
    tft.drawString("Amp: " + String(amplitude, 1) + " mV", emgGraphX, infoY + 1); 

    // Tampilkan Activation Percentage (kanan atas)
    int actWidth = 80; 
    tft.fillRect(emgGraphX + EMG_GRAPH_WIDTH - actWidth, infoY, actWidth, 14, TFT_BG_COLOR); 
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextDatum(TR_DATUM); 
    tft.drawString("Act: " + String(activationPercent) + "%", emgGraphX + EMG_GRAPH_WIDTH, infoY + 1);

    // Kembalikan ke Font Standar
    tft.setTextFont(1);
}

void DisplayManager::resetMeasurementMode() {
    // Reset semua variabel mode pengukuran
    currentMeasurementMode = 0;
    inModeSelection = false;
    selectedModeIndex = 0;
    
    // Hapus semua buffer grafik jika ada
    if (graphBackBuffer != nullptr) {
        delete[] graphBackBuffer;
        graphBackBuffer = nullptr;
    }
    
    needsGraphRedraw = false;
    lastGraphX = 0;
    lastGraphY = 0;
}

// PERBAIKAN: Fungsi drawButton dengan parameter font yang tepat
void DisplayManager::drawButton(int x, int y, int w, int h, String label, uint16_t color) {
    tft.fillRoundRect(x, y, w, h, 10, color);
    
    // Gunakan border khusus untuk tombol kembali
    if (label == "KEMBALI" || label == "BACK") {
        tft.drawRoundRect(x, y, w, h, 10, TFT_BUTTON_BORDER);
    } else {
        tft.drawRoundRect(x, y, w, h, 10, TFT_BUTTON_BORDER);
    }
    
    tft.setTextColor(TFT_BG_COLOR, color);
    tft.setTextDatum(MC_DATUM);
    
    // GUNAKAN UKURAN FONT YANG LEBIH KECIL UNTUK TOMBOL MEDICAL RECORD DAN ECG
    if (label == "START/STOP" || label == "GANTI MODE") {
        tft.setTextSize(1);  // Font lebih kecil untuk tombol Medical Record dan ECG
        tft.drawString(label, x + (w/2), y + (h/2), 1); // Font 1 (bukan 2)
    } else {
        tft.setTextSize(1);
        tft.drawString(label, x + (w/2), y + (h/2), 2); // Font 2 untuk tombol lain
    }
}

// FUNGSI YANG DIPERBARUI: showUserInfo dengan parameter Ubidots
void DisplayManager::showUserInfo(String userName, String ubidotsToken, String deviceLabel, int currentIndex, int totalUsers) {
    clearContentArea();
    currentScreenMode = 2; // Set mode ke USER INFO
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("USER SAAT INI", 240, 70);
    
    if (userName != "BELUM ADA USER") {
        // Nama User
        tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
        tft.setTextSize(3);
        tft.drawString(userName, 240, 110);
        
        // Ubidots Token (dipotong)
        tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
        tft.setTextSize(1);
        tft.drawString("Ubidots Token:", 240, 160);
        
        String displayToken = ubidotsToken;
        if (ubidotsToken.length() > 20) {
            // PERUBAHAN: Tampilkan 17 karakter terakhir dengan "..." di depan
            displayToken = "..." + ubidotsToken.substring(ubidotsToken.length() - 17);
        }
        tft.drawString(displayToken, 240, 175);
        
        // Device Label
        tft.drawString("Device Label:", 240, 195);
        tft.drawString(deviceLabel, 240, 210);
        
        // Info Index
        tft.setTextSize(2);
        tft.drawString("Index: " + String(currentIndex + 1) + " / " + String(totalUsers), 240, 230);
        
        // TOMBOL EDIT USER (posisi baru: Y = 240, ukuran lebih kecil)
        drawButton(btnEditUserX, btnEditUserY, btnEditW, btnEditH, "EDIT USER", TFT_ACCENT_BLUE);
    } else {
        // Tidak ada user
        tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
        tft.setTextSize(2);
        tft.drawString("BELUM ADA USER", 240, 120);
        tft.setTextSize(1);
        tft.drawString("Gunakan Serial untuk menambah user", 240, 160);
    }
    
    // Tombol Kembali (posisi baru: Y = 240, ukuran sama dengan tombol edit)
    drawButton(btnBackUserX, btnBackUserY, btnBackW, btnBackH, "KEMBALI", TFT_BUTTON_BG);
}

void DisplayManager::drawPlusIcon(int x, int y) {
    tft.fillRect(x + 4, y, 3,11, TFT_TEXT_COLOR);
    tft.fillRect(x, y + 4, 11, 3, TFT_TEXT_COLOR);
}

void DisplayManager::drawMinusIcon(int x, int y) {
    tft.fillRect(x, y + 4, 11, 3, TFT_TEXT_COLOR);
}

// FUNGSI YANG DIPERBARUI: showEditUser untuk menampilkan token dan device label Ubidots
void DisplayManager::showEditUser(int currentIndex, String currentName, String currentToken, String currentDeviceLabel) {
    clearContentArea();
    currentScreenMode = 4; // Set mode ke EDIT USER
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("EDIT USER", 240, 70);
    
    // Nama User Saat Ini
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Nama Saat Ini:", 50, 110);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.drawString(currentName, 50, 130);
    
    // Ubidots Token Saat Ini
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Token Saat Ini:", 50, 160);
    
    String displayToken = currentToken;
    if (currentToken.length() > 20) {
        displayToken = currentToken.substring(0, 17) + "...";
    }
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.drawString(displayToken, 50, 180);
    
    // Device Label Saat Ini
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Device Label Saat Ini:", 50, 210);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.drawString(currentDeviceLabel, 50, 230);
    
    // Tombol Edit Nama - Naik (dari 270 ke 240)
    drawButton(btnEditNameX, btnEditNameY, btnEditNameW, btnEditNameH, "EDIT NAMA", TFT_ACCENT_BLUE);
    
    // Tombol Edit Token - Naik (dari 270 ke 240)
    drawButton(btnEditKeyX, btnEditKeyY, btnEditKeyW, btnEditKeyH, "EDIT TOKEN", TFT_ACCENT_GREEN);
    
    // Tombol Edit Device Label - Naik (dari 270 ke 240)
    drawButton(btnEditDeviceX, btnEditDeviceY, btnEditKeyW, btnEditKeyH, "EDIT DEVICE", TFT_ACCENT_PURPLE);
    
    // Tombol Batal - Naik (dari 270 ke 240)
    drawButton(btnCancelEditX, btnCancelEditY, btnCancelEditW, btnCancelEditH, "KEMBALI", TFT_BUTTON_BG);
}

void DisplayManager::showWifiConfig(String ssid, String ip, int rssi, bool isConnected) {
    clearContentArea();
    currentScreenMode = 3; // Set mode ke WIFI CONFIG
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("KONFIGURASI WiFi", 240, 70);
    
    if (isConnected) {
        // Status: Terhubung
        tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
        tft.drawString("TERHUBUNG", 240, 110);
        
        // Detail WiFi
        tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
        tft.setTextSize(1);
        tft.drawString("SSID: " + ssid, 240, 140);
        tft.drawString("IP: " + ip, 240, 160);
        tft.drawString("RSSI: " + String(rssi) + " dBm", 240, 180);
    } else {
        // Status: Terputus
        tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
        tft.drawString("TERPUTUS", 240, 110);
        
        // Instruksi
        tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
        tft.setTextSize(1);
        tft.drawString("Gunakan Serial untuk setup WiFi", 240, 140);
        tft.drawString("Ketik 'wifi' di Serial Monitor", 240, 160);
    }
    
    // Tombol Scan WiFi
    drawButton(btnScanX, btnScanY, btnScanW, btnScanH, "SCAN WiFi", TFT_ACCENT_BLUE);
    
    // Tombol Kembali
    drawButton(btnBackWifiX, btnBackWifiY, btnBackWifiW, btnBackWifiH, "KEMBALI", TFT_BUTTON_BG);
}

void DisplayManager::drawMedicalRecordInterface(String currentDeviceName) {
    clearContentArea();
    currentScreenMode = 1;

    // ==================== LAYOUT BARU ====================
    
    // --- KOLOM KIRI (EDA & BPM) ---
    // EDA - Bagian Atas Kiri
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("EDA Sensor", 30, 100);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.drawString("---", 30, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Status: ---", 30, 150);
    
    // BPM - Bagian Bawah Kiri
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Heart Rate", 30, 180);
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 30, 200);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("BPM", 30, 235);
    
    // --- KOLOM KANAN (SUHU & SpO2) ---
    // Suhu Tubuh - Bagian Atas Kanan
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Suhu Tubuh", 250, 100);
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.drawString("---.-", 250, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("°C", 250, 150);
    
    // SpO2 - Bagian Bawah Kanan
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("SpO2", 250, 180);
    tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 250, 200);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("%", 250, 235);
    
    // TOMBOL MEDICAL RECORD (hanya 2 tombol)
    // Tombol Start/Stop (kiri bawah)
    drawButton(medBtnStartX, medBtnStartY, medBtnW, medBtnH, "START/STOP", TFT_ACCENT_GREEN);
    
    // Tombol Ganti Mode (kanan bawah)
    drawButton(medBtnModeX, medBtnModeY, medBtnModeW, medBtnModeH, "GANTI MODE", TFT_ACCENT_PURPLE);
    
    // Status bar kecil di bawah tombol
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Mode: Medical Record", 240, 320);
}

// FUNGSI BARU: Update data ECG di layar
void DisplayManager::updateECGData(int ecgRaw, float ecgVoltage, int heartRate, String leadStatus, bool isSending, int* ecgBuffer, int bufferSize, int threshold) {
    // 1. Update grafik (dan teks HR/Th yang sudah dipindah ke atas di dalam fungsi ini)
    if (ecgBuffer != nullptr && bufferSize > 0) {
        updateECGGraph(ecgBuffer, bufferSize, threshold, heartRate);
    }
    
    // --- BAGIAN TEKS LAMA DIHAPUS SAJA ---
    // (Karena sudah ditangani di dalam updateECGGraph dengan posisi yang benar)
    
    // 2. Indikator Upload (tetap pertahankan yang ini karena posisinya di luar grafik)
    if (isSending) {
        tft.fillCircle(460, 30, 8, TFT_ACCENT_RED);
        tft.drawCircle(460, 30, 8, TFT_BUTTON_BORDER);
    } else {
        tft.fillCircle(460, 30, 8, TFT_BG_COLOR);
        tft.drawCircle(460, 30, 8, TFT_BUTTON_BORDER);
    }
}

void DisplayManager::updateMedicalRecordData(int edaValue, String stressLevel, int heartRate, 
                                            int spo2, float objectTemp, bool isSending) {
    // Hapus area angka lama dengan warna hitam
    tft.fillRect(30, 120, 180, 30, TFT_BG_COLOR);     // Area nilai EDA
    tft.fillRect(30, 150, 180, 20, TFT_BG_COLOR);     // Area status EDA
    tft.fillRect(30, 200, 180, 35, TFT_BG_COLOR);     // Area BPM
    tft.fillRect(250, 120, 180, 30, TFT_BG_COLOR);    // Area suhu
    tft.fillRect(250, 200, 180, 35, TFT_BG_COLOR);    // Area SpO2
    
    // --- KOLOM KIRI: EDA dan BPM ---
    // Tampilkan Nilai EDA
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(String(edaValue), 30, 120);
    
    // Tampilkan Level Stress dengan warna sesuai
    uint16_t stressColor = TFT_ACCENT_GREEN;
    if (stressLevel == "Cemas Ringan") {
        stressColor = TFT_ACCENT_ORANGE;
    } else if (stressLevel == "Cemas Sedang") {
        stressColor = TFT_ACCENT_ORANGE;
    } else if (stressLevel == "Stres Tinggi") {
        stressColor = TFT_ACCENT_RED;
    }
    
    tft.setTextColor(stressColor, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Status: " + stressLevel, 30, 150);
    
    // Tampilkan Heart Rate
    tft.setTextColor(TFT_ACCENT_RED, TFT_BG_COLOR);
    tft.setTextSize(3);
    if (heartRate > 0) {
        tft.drawString(String(heartRate), 30, 200);
    } else {
        tft.drawString("---", 30, 200);
    }
    
    // --- KOLOM KANAN: Suhu dan SpO2 ---
    // Tampilkan Suhu Tubuh
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    if (objectTemp > 0) {
        tft.drawString(String(objectTemp, 1), 250, 120);
    } else {
        tft.drawString("---.-", 250, 120);
    }
    
    // Tampilkan SpO2
    tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
    tft.setTextSize(3);
    if (spo2 > 0) {
        tft.drawString(String(spo2), 250, 200);
    } else {
        tft.drawString("---", 250, 200);
    }
    
    // Indikator Upload (lingkaran kecil di pojok kanan atas)
    if (isSending) {
        tft.fillCircle(460, 70, 8, TFT_ACCENT_RED);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    } else {
        tft.fillCircle(460, 70, 8, TFT_BG_COLOR);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    }
    
    // Tampilkan nilai konduktansi EDA kecil di bawah nilai EDA (opsional)
    static unsigned long lastConductanceUpdate = 0;
    if (millis() - lastConductanceUpdate > 2000) { // Update setiap 2 detik
        tft.fillRect(30, 145, 100, 10, TFT_BG_COLOR);
        lastConductanceUpdate = millis();
    }
}

void DisplayManager::updateSensorData(int edaValue, String stressLevel, bool isSending) {
    // Hapus area angka lama dengan menimpa kotak hitam
    tft.fillRect(20, 100, 200, 100, TFT_BG_COLOR);

    // Tampilkan Nilai EDA
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.drawString("EDA: " + String(edaValue), 20, 100);

    // Tampilkan Level Stress
    uint16_t color = TFT_ACCENT_GREEN;
    if(stressLevel == "Cemas Sedang") color = TFT_ACCENT_ORANGE;
    if(stressLevel == "Stres Tinggi") color = TFT_ACCENT_RED;

    tft.setTextColor(color, TFT_BG_COLOR);
    tft.drawString(stressLevel, 20, 140);

    // Indikator Upload
    if(isSending) {
        tft.fillCircle(440, 80, 10, TFT_ACCENT_RED);
    } else {
        tft.fillCircle(440, 80, 10, TFT_BG_COLOR);
    }
}

void DisplayManager::updateStatus(String statusMsg) {
    // Status Bar di bawah
    tft.fillRect(0, 300, 480, 20, TFT_BUTTON_BORDER);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BUTTON_BORDER);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(1);
    tft.drawString(statusMsg, 240, 305);
}

// Fungsi User List - DENGAN PARAMETER resetPage
void DisplayManager::showUserList(const std::vector<String>& userNames, int currentIndex, bool resetPage) {
    clearContentArea();
    currentScreenMode = 6; // Mode user list

    // Simpan daftar user
    userList = userNames;
    selectedUserIndex = currentIndex;
    
    // Hanya reset halaman jika diminta (default: true)
    if (resetPage) {
        userListPage = currentIndex / USER_PER_PAGE;
    }
    
    // Gambar tampilan
    refreshUserListDisplay();
}

// FUNGSI BARU: Refresh tampilan user list tanpa reset halaman
void DisplayManager::refreshUserListDisplay() {
    // Bersihkan area user list (tapi jangan hapus header dan tombol +/-)
    clearUserListArea();

    // Judul - HARUS DITAMPILKAN KEMBALI
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("PILIH USER", 240, 60);

    // PERBAIKAN: Gambar tombol + dengan warna yang kontras
    tft.fillRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_ACCENT_GREEN);
    tft.drawRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_BUTTON_BORDER);
    drawPlusIcon(btnAddUserX + 10, btnAddUserY + 10);  // Tambah offset untuk icon

    // PERBAIKAN: Gambar tombol - jika ada user
    if (userList.size() > 0) {
        tft.fillRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_ACCENT_RED);
        tft.drawRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_BUTTON_BORDER);
        drawMinusIcon(btnDeleteUserX + 10, btnDeleteUserY + 10);  // Tambah offset untuk icon
    }
    
    // Tampilkan jumlah user
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Total: " + String(userList.size()) + " user", 240, 90);

    // Hitung range user
    int startIdx = userListPage * USER_PER_PAGE;
    int endIdx = min(startIdx + USER_PER_PAGE, (int)userList.size());

    // Menampilkan user di halaman
    tft.setTextSize(2);
    for (int i = startIdx; i < endIdx; i++) {
        int yPos = 120 + (i - startIdx) * 30;

        // Highlight user yang aktif
        if (i == selectedUserIndex) {
            tft.fillRect(40, yPos - 15, 400, 28, TFT_ACCENT_BLUE);
        } else {
            tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
        }

        // Format: no nama user
        String displayText = "[" + String(i+1) + "]" + userList[i];
        if (displayText.length() > 25) {
            displayText = displayText.substring(0,22) + "...";
        }

        tft.setTextDatum(ML_DATUM);
        tft.drawString(displayText, 50, yPos);

        // Gambar kotak seleksi
        tft.drawRect(40, yPos - 15, 400, 28, TFT_BUTTON_BORDER);
    }

    // Tombol navigasi halaman
    if (userList.size() > USER_PER_PAGE) {
        // Menampilkan halaman
        tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
        tft.setTextSize(1);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Halaman: " + String(userListPage + 1) + "/" + String((userList.size() + USER_PER_PAGE - 1) / USER_PER_PAGE), 240, 270);

        // Tombol sebelum
        if (userListPage > 0) {
            drawButton(btnPrevPageX, btnPrevPageY, btnPageW, btnPageH, "<SEBELUM>", TFT_ACCENT_BLUE);
        }

        // Tombol berikut
        if ((userListPage + 1) * USER_PER_PAGE < userList.size()) {
            drawButton(btnNextPageX, btnNextPageY, btnNextPageW, btnNextPageH, "<BERIKUT>", TFT_ACCENT_BLUE);
        }
    }

    // Tombol batal
    drawButton(btnCancelListX, btnCancelListY, btnCancelListW, btnCancelListH, "KEMBALI", TFT_BUTTON_BG);
    
    updateStatus("Sentuh user untuk memilih");
}

void DisplayManager::drawBodyPositionSnoreInterface(String currentDeviceName) {
    clearContentArea();
    currentScreenMode = 1;
    currentMeasurementMode = 4; // Set mode ke Body Position & Snore

    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("BODY POSITION & SNORE", 240, 60);
    
    // --- KOLOM KIRI: POSISI TUBUH ---
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("POSISI TUBUH", 50, 100);
    tft.setTextColor(TFT_ACCENT_GREEN, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 50, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Status: ---", 50, 170);
    
    // --- KOLOM KANAN: SKALA DENGKUR ---
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("SKALA DENGKUR", 250, 100);
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString("---", 250, 120);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Level: ---", 250, 170);
    
    // Instruksi
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Sensor MPU6050 & Sound", 240, 200);
    tft.drawString("untuk deteksi posisi dan dengkur", 240, 215);
    
    // Tombol START/STOP dan GANTI MODE
    drawButton(20, 245, 200, 50, "START/STOP", TFT_ACCENT_GREEN);
    drawButton(240, 245, 200, 50, "GANTI MODE", TFT_ACCENT_PURPLE);
    
    // Status bar
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Mode: Body Position & Snore", 240, 320);
}

void DisplayManager::updateBodyPositionSnoreData(int position, int snoreScale, bool isSending) {
    // Hapus area data lama
    tft.fillRect(50, 120, 150, 50, TFT_BG_COLOR);   // Area posisi
    tft.fillRect(50, 170, 150, 20, TFT_BG_COLOR);   // Status posisi
    tft.fillRect(250, 120, 150, 50, TFT_BG_COLOR);  // Area dengkur
    tft.fillRect(250, 170, 150, 20, TFT_BG_COLOR);  // Level dengkur
    
    // --- POSISI TUBUH ---
    String positionName = "Unknown";
    uint16_t positionColor = TFT_ACCENT_GREEN;
    
    switch (position) {
        case 0: positionName = "Transisi"; positionColor = TFT_ACCENT_ORANGE; break;
        case 1: positionName = "Berdiri"; positionColor = TFT_ACCENT_GREEN; break;
        case 2: positionName = "Duduk"; positionColor = TFT_ACCENT_CYAN; break;
        case 3: positionName = "Telentang"; positionColor = TFT_ACCENT_BLUE; break;
        case 4: positionName = "Tengkurap"; positionColor = TFT_ACCENT_RED; break;
        case 5: positionName = "Miring Kanan"; positionColor = TFT_ACCENT_PURPLE; break;
        case 6: positionName = "Miring Kiri"; positionColor = TFT_ACCENT_PURPLE; break;
    }
    
    tft.setTextColor(positionColor, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(String(position), 50, 120);
    
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Status: " + positionName, 50, 170);
    
    // --- SKALA DENGKUR ---
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
    tft.setTextSize(3);
    tft.drawString(String(snoreScale), 250, 120);
    
    String snoreLevel = "Tidak Mendengkur";
    if (snoreScale > 0) {
        if (snoreScale <= 30) snoreLevel = "Ringan";
        else if (snoreScale <= 60) snoreLevel = "Sedang";
        else snoreLevel = "Berat";
    }
    
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.drawString("Level: " + snoreLevel, 250, 170);
    
    // Indikator Upload
    if (isSending) {
        tft.fillCircle(460, 70, 8, TFT_ACCENT_RED);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    } else {
        tft.fillCircle(460, 70, 8, TFT_BG_COLOR);
        tft.drawCircle(460, 70, 8, TFT_BUTTON_BORDER);
    }
}

// ==================== FUNGSI KEYBOARD VIRTUAL ====================

void DisplayManager::toggleShift() {
    // Jika dalam mode simbol, shift tidak berlaku
    if (symbolMode) {
        return;
    }

    if (capsLock) {
        capsLock = false;
        shiftActive = false;
    } else if (shiftActive) {
        shiftActive = false;
        capsLock = true; // Set caps lock setelah shift kedua kali
    } else {
        shiftActive = true;
    }
    
    // Redraw keyboard dengan state baru
    drawKeyboard();
}

void DisplayManager::setKeyboardMode(bool forName) {
    forNameField = forName;
    if (!forName) {
        shiftActive = false;
        capsLock = false;
        symbolMode = false; // Reset mode simbol untuk API key
    }
}

void DisplayManager::showKeyboard(String currentValue, String title, bool forName) {
    clearContentArea();
    keyboardActive = true;
    keyboardCancelled = false;
    keyboardBuffer = currentValue;
    keyboardTitle = title;
    forNameField = forName;
    shiftActive = false;
    capsLock = false;
    symbolMode = false;  // Reset ke mode huruf
    cursorPos = keyboardBuffer.length();
    currentScreenMode = 5; // Mode keyboard
    
    // Draw input field
    drawInputField();
    
    // Draw keyboard
    drawKeyboard();
    
    // Status
    if (forName) {
        updateStatus("SFT: huruf besar/kecil, SYM: simbol, ESC: kembali");
    } else {
        updateStatus("SYM: simbol, ABC: kembali ke huruf, ESC: kembali");
    }
}

void DisplayManager::drawInputField() {
    // Background input field
    tft.fillRoundRect(20, 55, 440, 40, 5, TFT_BUTTON_BG);
    tft.drawRoundRect(20, 55, 440, 40, 5, TFT_BUTTON_BORDER);
    
    // Tampilkan teks
    tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BUTTON_BG);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    
    // Jika buffer kosong, tampilkan placeholder
    if (keyboardBuffer.length() == 0) {
        tft.drawString("Ketik di sini...", 30, 65);
    } else {
        // Tampilkan teks dengan ellipsis jika terlalu panjang
        String displayText = keyboardBuffer;
        if (displayText.length() > 30) {
            // Tampilkan 27 karakter terakhir dengan "..." di depan
            displayText = "..." + displayText.substring(displayText.length() - 27);
        }
        tft.drawString(displayText, 30, 65);
    }
    
    // Tampilkan panjang teks
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(String(keyboardBuffer.length()) + " chars", 470, 70);
    
    // Cursor indicator - LOGIKA BARU
    int cursorX = 30; // Posisi default
    
    // Hitung posisi kursor berdasarkan apakah teks dipotong atau tidak
    if (keyboardBuffer.length() <= 30) {
        // Teks tidak dipotong, kursor di posisi normal
        if (cursorPos <= keyboardBuffer.length()) {
            cursorX = 30 + (cursorPos * 12);
        } else {
            cursorX = 30 + (keyboardBuffer.length() * 12);
        }
    } else {
        // Teks dipotong (ditampilkan "... + 27 karakter terakhir")
        // Kita perlu menghitung offset untuk kursor
        int startIndex = keyboardBuffer.length() - 27; // Indeks awal yang ditampilkan
        
        if (cursorPos >= startIndex) {
            // Kursor berada di bagian yang ditampilkan
            int displayPos = cursorPos - startIndex;
            // Tambah 3 untuk memperhitungkan "..." di depan
            cursorX = 30 + ((displayPos + 3) * 12);
        } else {
            // Kursor berada di bagian yang tidak ditampilkan (sebelum "..." + karakter yang tampil)
            // Tampilkan kursor di posisi setelah "..." (karakter pertama yang terlihat)
            cursorX = 30 + (3 * 12); // Posisi setelah "..."
        }
        
        // Batasi cursorX agar tidak keluar area input
        if (cursorX > 30 + (30 * 12)) {
            cursorX = 30 + (30 * 12); // Maksimal di akhir area
        }
    }
    
    // Gambar kursor
    if (cursorX < 450) {
        tft.drawFastVLine(cursorX, 60, 25, TFT_ACCENT_CYAN);
    }
}

void DisplayManager::drawKeyboard() {
    const char (*currentLayout)[KEY_COLS];
    
    // Pilih layout berdasarkan mode
    if (symbolMode) {
        currentLayout = keyboardSymbol;
    } else if (shiftActive || capsLock) {
        currentLayout = keyboardUpper;
    } else {
        currentLayout = keyboardLower;
    }
    
    // Gambar tombol keyboard utama
    for (int row = 0; row < KEY_ROWS; row++) {
        for (int col = 0; col < KEY_COLS; col++) {
            int x = keyStartX + col * (keyWidth + keySpacing);
            int y = keyStartY + row * (keyHeight + keySpacing);
            
            char key = currentLayout[row][col];
            
            // Skip tombol spasi (jika ada)
            if (key == ' ') {
                continue;
            }
            
            String keyStr;
            
            // Convert karakter khusus ke string yang bisa ditampilkan
            if (key == 8) {
                keyStr = "BS";
            } else if (key == 13) {
                keyStr = "OK";
            } else if (key == '^') {
                keyStr = "SFT";
            } else if (key == 26) {
                keyStr = "ABC";
            } else if (key == 27) {
                keyStr = "ESC";
            } else if (key == 0) {
                // Tombol kosong, lewati
                continue;
            } else {
                keyStr = String(key);
            }
            
            // Warna tombol khusus
            uint16_t color = TFT_ACCENT_BLUE;
            
            if (key == 8) {
                color = TFT_ACCENT_RED;
            } else if (key == 13) {
                color = TFT_ACCENT_GREEN;
            } else if (key == '^') {
                color = (shiftActive || capsLock) ? TFT_ACCENT_ORANGE : TFT_ACCENT_ORANGE;
            } else if (key == 26) {
                color = TFT_ACCENT_PURPLE; // Tombol ABC
            } else if (key == 27) {
                color = TFT_ACCENT_ORANGE;
            } else if (isdigit(key)) {
                color = TFT_ACCENT_PURPLE;
            } else if (symbolMode) {
                color = TFT_ACCENT_CYAN; // Warna khusus untuk simbol
            }
            
            // Gambar tombol
            tft.fillRoundRect(x, y, keyWidth, keyHeight, 5, color);
            tft.drawRoundRect(x, y, keyWidth, keyHeight, 5, TFT_BUTTON_BORDER);
            
            // Tampilkan karakter
            tft.setTextColor(TFT_BG_COLOR, color);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString(keyStr, x + keyWidth/2, y + keyHeight/2);
        }
    }
    
    // Tombol SPACE
    drawButton(spaceKeyX, spaceKeyY, spaceKeyW, spaceKeyH, "SPACE", TFT_BUTTON_BG);
    
    // Tombol BACK/KEMBALI
    drawButton(backKeyX, backKeyY, backKeyW, backKeyH, "BACK", TFT_ACCENT_ORANGE);
    
    // Tombol CLEAR
    drawButton(clearKeyX, clearKeyY, clearKeyW, clearKeyH, "CLEAR", TFT_ACCENT_RED);
    
    // Tombol SYM (jika dalam mode huruf) - POSISI DIUBAH
    if (!symbolMode) {
        // Posisi baru: baris ke-4, kolom ke-9 (sebelum tombol Enter)
        int symBtnX = keyStartX + 8 * (keyWidth + keySpacing);
        int symBtnY = keyStartY + 3 * (keyHeight + keySpacing);
        
        tft.fillRoundRect(symBtnX, symBtnY, keyWidth, keyHeight, 5, TFT_ACCENT_CYAN);
        tft.drawRoundRect(symBtnX, symBtnY, keyWidth, keyHeight, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_CYAN);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("SYM", symBtnX + keyWidth/2, symBtnY + keyHeight/2);
    }
}

String DisplayManager::getKeyboardInput() {
    return keyboardBuffer;
}

void DisplayManager::clearKeyboard() {
    keyboardActive = false;
    keyboardCancelled = false;
    keyboardBuffer = "";
    keyboardTitle = "";
    shiftActive = false;
    capsLock = false;
    symbolMode = false;  // Reset mode simbol
    cursorPos = 0;
}

void DisplayManager::processKeyPress(char key) {
    // Handle special keys
    if (key == 8) { // Backspace (ASCII 8)
        if (keyboardBuffer.length() > 0 && cursorPos > 0) {
            keyboardBuffer.remove(cursorPos - 1, 1);
            cursorPos--;
        }
    } 
    else if (key == 13) { // Enter (ASCII 13)
        keyboardActive = false;
        return;
    }
    else if (key == 27) { // ESC/Kembali (ASCII 27) - BARU
        keyboardCancelled = true;
        keyboardActive = false;
        return;
    }
    else if (key == 26) { // BARU: Tombol ABC (kembali ke mode huruf)
        symbolMode = false;
        // Reset shift jika untuk API key
        if (!forNameField) {
            shiftActive = false;
            capsLock = false;
        }
        drawKeyboard();
        return;
    }
    else if (key == '^') { // Shift
        toggleShift();
        return;
    }
    else { // Regular key
        // Jangan proses tombol spasi (' ') dari layout
        if (key == ' ') {
            return;
        }
        
        if (keyboardBuffer.length() < 100) { // Max length
            // Insert karakter di posisi cursor
            keyboardBuffer = keyboardBuffer.substring(0, cursorPos) + 
                           String(key) + 
                           keyboardBuffer.substring(cursorPos);
            cursorPos++;
        }
    }
    
    // Jika shift ditekan (bukan caps lock), matikan setelah satu karakter
    if (shiftActive && !capsLock && key != '^' && key != 26) {
        shiftActive = false;
    }
    
    // Update tampilan
    drawInputField();
    drawKeyboard();
}

// ==================== FUNGSI WIFI SCAN ====================

void DisplayManager::showWifiScanList(const std::vector<String>& ssids, const std::vector<int>& rssis, const std::vector<String>& encTypes, int currentPage) {
    clearContentArea();
    currentScreenMode = 7; // Mode WiFi scan list
    
    wifiSSIDList = ssids;
    wifiRSSIList = rssis;
    wifiEncTypeList = encTypes;
    wifiScanPage = currentPage;
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("SCAN WiFi NETWORKS", 240, 60);
    
    // Tampilkan jumlah jaringan
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Total: " + String(ssids.size()) + " networks found", 240, 90);
    
    if (ssids.size() == 0) {
        tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
        tft.setTextSize(2);
        tft.drawString("No networks found", 240, 140);
        tft.setTextSize(1);
        tft.drawString("Tap REFRESH to scan again", 240, 170);
        tft.drawString("or BACK to return", 240, 190);
        
        // Tombol Refresh dan Back saja (tengah layar)
        drawButton(140, 220, 80, 40, "REFRESH", TFT_ACCENT_BLUE);
        drawButton(260, 220, 80, 40, "BACK", TFT_BUTTON_BG);
        
        updateStatus("No networks found. Tap REFRESH or BACK");
        return;
    }
    
    // Tampilkan daftar WiFi
    int startIdx = wifiScanPage * WIFI_PER_PAGE;
    int endIdx = min(startIdx + WIFI_PER_PAGE, (int)ssids.size());
    
    tft.setTextSize(1);
    for (int i = startIdx; i < endIdx; i++) {
        int yPos = 115 + (i - startIdx) * 30;
        
        // Highlight jika sedang dipilih
        tft.drawRect(40, yPos - 10, 400, 28, TFT_BUTTON_BORDER);
        
        // Tampilkan SSID
        String displayText = "[" + String(i+1) + "] " + ssids[i];
        if (displayText.length() > 25) {
            displayText = displayText.substring(0, 22) + "...";
        }
        
        tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
        tft.setTextDatum(ML_DATUM);
        tft.drawString(displayText, 50, yPos);
        
        // Tampilkan RSSI dan enkripsi
        String info = String(rssis[i]) + " dBm";
        if (encTypes[i] != "Open") {
            info += " (" + encTypes[i] + ")";
        }
        
        tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
        tft.setTextDatum(MR_DATUM);
        tft.drawString(info, 430, yPos);
    }
    
    // Tombol navigasi halaman
    if (ssids.size() > WIFI_PER_PAGE) {
        // Menampilkan halaman
        tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR);
        tft.setTextSize(1);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Page: " + String(wifiScanPage + 1) + "/" + 
                      String((ssids.size() + WIFI_PER_PAGE - 1) / WIFI_PER_PAGE), 
                      240, 270);
        
        // Tombol sebelumnya - TAMPILKAN JIKA BUKAN HALAMAN PERTAMA
        if (wifiScanPage > 0) {
            drawButton(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, "<PREV", TFT_ACCENT_BLUE);
        }
        
        // Tombol berikutnya
        if ((wifiScanPage + 1) * WIFI_PER_PAGE < ssids.size()) {
            drawButton(wifiBtnNextPageX, wifiBtnNextPageY, wifiBtnNextPageW, wifiBtnNextPageH, "NEXT>", TFT_ACCENT_BLUE);
        }
    }
    
    // Tombol Refresh (selalu tampil)
    drawButton(wifiBtnRefreshX, wifiBtnRefreshY, wifiBtnRefreshW, wifiBtnRefreshH, "REFRESH", TFT_ACCENT_GREEN);
    
    // Tombol Back - HANYA TAMPIL DI HALAMAN PERTAMA
    if (wifiScanPage == 0) {
        drawButton(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, "BACK", TFT_BUTTON_BG);
    }
    
    updateStatus("Tap network to select, REFRESH to scan again");
}

int DisplayManager::checkTouchWifiScanList(int screenX, int screenY) {
    if (wifiSSIDList.size() == 0) {
        // Cek tombol Refresh (ketika tidak ada jaringan)
        if (screenX > 140 && screenX < 220 &&
            screenY > 220 && screenY < 260) {
            // Feedback visual - TOMBOL KUNING
            tft.fillRect(140, 220, 80, 40, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(140, 220, 80, 40, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("REFRESH", 180, 240, 2);
            delay(100); // Efek visual
            
            // KEMBALIKAN KE WARNA ASLI (BIRU) SEBELUM SCAN
            tft.fillRect(140, 220, 80, 40, TFT_ACCENT_BLUE);
            tft.drawRoundRect(140, 220, 80, 40, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
            tft.drawString("REFRESH", 180, 240, 2);
            
            return 71; // Kode untuk refresh scan
        }
        
        // Cek tombol Back (ketika tidak ada jaringan)
        if (screenX > 260 && screenX < 340 &&
            screenY > 220 && screenY < 260) {
            // Feedback visual
            tft.fillRect(260, 220, 80, 40, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(260, 220, 80, 40, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("BACK", 300, 240, 2);
            delay(100);
            
            // Kembalikan ke warna asli
            tft.fillRect(260, 220, 80, 40, TFT_BUTTON_BG);
            tft.drawRoundRect(260, 220, 80, 40, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
            tft.drawString("BACK", 300, 240, 2);
            
            return 90; // Kode untuk kembali
        }
        
        return 0;
    }
    
    // Cek sentuhan area daftar WiFi
    int startIdx = wifiScanPage * WIFI_PER_PAGE;
    int endIdx = min(startIdx + WIFI_PER_PAGE, (int)wifiSSIDList.size());
    
    for (int i = startIdx; i < endIdx; i++) {
        int yPos = 115 + (i - startIdx) * 30;
        
        if (screenX > 40 && screenX < 440 && screenY > yPos - 10 && screenY < yPos + 18) {
            // Feedback visual
            tft.fillRect(40, yPos - 10, 400, 28, TFT_ACCENT_ORANGE);
            
            // Tampilkan SSID dengan warna berbeda
            String displayText = "[" + String(i+1) + "] " + wifiSSIDList[i];
            if (displayText.length() > 25) {
                displayText = displayText.substring(0, 22) + "...";
            }
            
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(ML_DATUM);
            tft.drawString(displayText, 50, yPos);
            
            // Tampilkan info dengan warna berbeda
            String info = String(wifiRSSIList[i]) + " dBm";
            if (wifiEncTypeList[i] != "Open") {
                info += " (" + wifiEncTypeList[i] + ")";
            }
            
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MR_DATUM);
            tft.drawString(info, 430, yPos);
            
            delay(100);
            
            // Kembalikan ke tampilan semula
            tft.fillRect(40, yPos - 10, 400, 28, TFT_BG_COLOR);
            tft.drawRect(40, yPos - 10, 400, 28, TFT_BUTTON_BORDER);
            
            tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
            tft.drawString(displayText, 50, yPos);
            
            tft.setTextColor(TFT_ACCENT_ORANGE, TFT_BG_COLOR);
            tft.drawString(info, 430, yPos);
            
            return 200 + i; // Kode 200 + index WiFi
        }
    }
    
    // Cek tombol Refresh (ketika ada jaringan)
    if (screenX > wifiBtnRefreshX && screenX < wifiBtnRefreshX + wifiBtnRefreshW &&
        screenY > wifiBtnRefreshY && screenY < wifiBtnRefreshY + wifiBtnRefreshH) {
        // Feedback visual
        tft.fillRect(wifiBtnRefreshX, wifiBtnRefreshY, wifiBtnRefreshW, wifiBtnRefreshH, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(wifiBtnRefreshX, wifiBtnRefreshY, wifiBtnRefreshW, wifiBtnRefreshH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("REFRESH", wifiBtnRefreshX + (wifiBtnRefreshW/2), 
                      wifiBtnRefreshY + (wifiBtnRefreshH/2), 2);
        delay(100);
        
        // Kembali ke warna asli
        tft.fillRect(wifiBtnRefreshX, wifiBtnRefreshY, wifiBtnRefreshW, wifiBtnRefreshH, TFT_ACCENT_GREEN);
        tft.drawRoundRect(wifiBtnRefreshX, wifiBtnRefreshY, wifiBtnRefreshW, wifiBtnRefreshH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_GREEN);
        tft.drawString("REFRESH", wifiBtnRefreshX + (wifiBtnRefreshW/2), 
                      wifiBtnRefreshY + (wifiBtnRefreshH/2), 2);
        
        return 71; // Kode untuk refresh scan
    }
    
    // Cek tombol sebelumnya (jika ada dan bukan halaman pertama)
    if (wifiScanPage > 0 && screenX > wifiBtnPrevPageX && screenX < wifiBtnPrevPageX + wifiBtnPageW &&
        screenY > wifiBtnPrevPageY && screenY < wifiBtnPrevPageY + wifiBtnPageH) {
        // Feedback visual
        tft.fillRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("<PREV", wifiBtnPrevPageX + (wifiBtnPageW/2), 
                      wifiBtnPrevPageY + (wifiBtnPageH/2), 2);
        delay(100);
        
        // Kembalikan ke warna asli
        tft.fillRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, TFT_ACCENT_BLUE);
        tft.drawRoundRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
        tft.drawString("<PREV", wifiBtnPrevPageX + (wifiBtnPageW/2), 
                      wifiBtnPrevPageY + (wifiBtnPageH/2), 2);
        
        // Kurangi halaman dan refresh tampilan
        wifiScanPage--;
        showWifiScanList(wifiSSIDList, wifiRSSIList, wifiEncTypeList, wifiScanPage);
        return 81; // Kode untuk halaman sebelumnya
    }
    
    // Cek tombol berikutnya (jika ada)
    if ((wifiScanPage + 1) * WIFI_PER_PAGE < wifiSSIDList.size() &&
        screenX > wifiBtnNextPageX && screenX < wifiBtnNextPageX + wifiBtnNextPageW &&
        screenY > wifiBtnNextPageY && screenY < wifiBtnNextPageY + wifiBtnNextPageH) {
        // Feedback visual
        tft.fillRect(wifiBtnNextPageX, wifiBtnNextPageY, wifiBtnNextPageW, wifiBtnNextPageH, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(wifiBtnNextPageX, wifiBtnNextPageY, wifiBtnNextPageW, wifiBtnNextPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("NEXT>", wifiBtnNextPageX + (wifiBtnNextPageW/2), 
                      wifiBtnNextPageY + (wifiBtnNextPageH/2), 2);
        delay(100);
        
        // Kembalikan ke warna asli
        tft.fillRect(wifiBtnNextPageX, wifiBtnNextPageY, wifiBtnNextPageW, wifiBtnNextPageH, TFT_ACCENT_BLUE);
        tft.drawRoundRect(wifiBtnNextPageX, wifiBtnNextPageY, wifiBtnNextPageW, wifiBtnNextPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
        tft.drawString("NEXT>", wifiBtnNextPageX + (wifiBtnNextPageW/2), 
                      wifiBtnNextPageY + (wifiBtnNextPageH/2), 2);
        
        // Tambah halaman dan refresh tampilan
        wifiScanPage++;
        showWifiScanList(wifiSSIDList, wifiRSSIList, wifiEncTypeList, wifiScanPage);
        return 82; // Kode untuk halaman berikutnya
    }
    
    // Cek tombol Back (hanya di halaman pertama)
    if (wifiScanPage == 0 && screenX > wifiBtnPrevPageX && screenX < wifiBtnPrevPageX + wifiBtnPageW &&
        screenY > wifiBtnPrevPageY && screenY < wifiBtnPrevPageY + wifiBtnPageH) {
        // Feedback visual
        tft.fillRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("BACK", wifiBtnPrevPageX + (wifiBtnPageW/2), 
                      wifiBtnPrevPageY + (wifiBtnPageH/2), 2);
        delay(100);
        
        // Kembalikan ke warna asli
        tft.fillRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, TFT_BUTTON_BG);
        tft.drawRoundRect(wifiBtnPrevPageX, wifiBtnPrevPageY, wifiBtnPageW, wifiBtnPageH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("BACK", wifiBtnPrevPageX + (wifiBtnPageW/2), 
                      wifiBtnPrevPageY + (wifiBtnPageH/2), 2);
        
        return 90; // Kode untuk kembali
    }
    
    return 0;
}

// ==================== FUNGSI CHECK TOUCH ====================

int DisplayManager::checkTouch(int screenMode) {
    if (!touchSupported || !ts.touched()) return 0;

    TS_Point p = ts.getPoint();
    
    // Mapping koordinat
    int screenX = map(p.y, touchMinX, touchMaxX, 0, 480);
    int screenY = map(p.x, touchMinY, touchMaxY, 320, 0);
    
    screenX = constrain(screenX, 0, 479);
    screenY = constrain(screenY, 0, 319);
    
    // Debounce
    static unsigned long lastTouchTime = 0;
    unsigned long now = millis();
    if (now - lastTouchTime < 200) return 0;
    lastTouchTime = now;
    
    // CEK BERDASARKAN SCREEN MODE
    if (screenMode == 0) { // HOME SCREEN
        return checkTouchHome(screenX, screenY);
    }
    else if (screenMode == 1) { // MEASUREMENT SCREEN
        return checkTouchMeasurement(screenX, screenY);
    }
    else if (screenMode == 2) { // USER INFO SCREEN
        return checkTouchUserInfo(screenX, screenY);
    }
    else if (screenMode == 3) { // WIFI CONFIG SCREEN
        return checkTouchWifiConfig(screenX, screenY);
    }
    else if (screenMode == 4) { // EDIT USER SCREEN
        return checkTouchEditUser(screenX, screenY);
    }
    else if (screenMode == 5) { // KEYBOARD SCREEN
        return checkTouchKeyboard(screenX, screenY);
    }
    else if (screenMode == 6) { // USER LIST SCREEN
        return checkTouchUserList(screenX, screenY);
    }
    else if (screenMode == 7) { // WIFI SCAN LIST SCREEN
        return checkTouchWifiScanList(screenX, screenY);
    }
    else if (screenMode == 8) { // MODE SELECTION SCREEN (BARU)
        return checkTouchModeSelection(screenX, screenY);
    }
    
    return 0;
}

int DisplayManager::checkTouchHome(int screenX, int screenY) {
    // 1. Tombol User Sekarang
    if (screenX > btnUserX && screenX < btnUserX + btnWidth &&
        screenY > btnUserY && screenY < btnUserY + btnHeight) {
        // Feedback visual
        tft.fillRoundRect(btnUserX, btnUserY, btnWidth, btnHeight, 15, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextSize(2);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("USER", btnUserX + (btnWidth/2), btnUserY + 20);
        delay(100);
        return 10;
    }
    
    // 2. Tombol Pengukuran
    if (screenX > btnMeasureX && screenX < btnMeasureX + btnWidth &&
        screenY > btnMeasureY && screenY < btnMeasureY + btnHeight) {
        // Feedback visual
        tft.fillRoundRect(btnMeasureX, btnMeasureY, btnWidth, btnHeight, 15, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextSize(2);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("PENGUKURAN", btnMeasureX + (btnWidth/2), btnMeasureY + 20);
        delay(100);
        return 20;
    }
    
    // 3. Tombol Ganti User
    if (screenX > btnChangeX && screenX < btnChangeX + btnWidth &&
        screenY > btnChangeY && screenY < btnChangeY + btnHeight) {
        // Feedback visual
        tft.fillRoundRect(btnChangeX, btnChangeY, btnWidth, btnHeight, 15, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextSize(2);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("GANTI USER", btnChangeX + (btnWidth/2), btnChangeY + 20);
        delay(100);
        return 30;
    }
    
    // 4. Tombol WiFi Config
    if (screenX > btnWifiX && screenX < btnWifiX + btnWidth &&
        screenY > btnWifiY && screenY < btnWifiY + btnHeight) {
        // Feedback visual
        tft.fillRoundRect(btnWifiX, btnWifiY, btnWidth, btnHeight, 15, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextSize(2);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("WIFI", btnWifiX + (btnWidth/2), btnWifiY + 20);
        delay(100);
        return 40;
    }
    
    return 0;
}

int DisplayManager::checkTouchMeasurement(int screenX, int screenY) {
    // Periksa mode saat ini untuk menentukan tombol mana yang aktif
    if (currentMeasurementMode == 0) { // Medical Record
        // 1. Tombol Start/Stop (Medical Record)
        if (screenX > medBtnStartX && screenX < medBtnStartX + medBtnW &&
            screenY > medBtnStartY && screenY < medBtnStartY + medBtnH) {
            
            // SIMPAN WARNA ASLI untuk mengembalikan nanti
            static uint16_t originalColor = TFT_ACCENT_GREEN;
            
            // EFEK VISUAL: tombol berubah warna
            tft.fillRoundRect(medBtnStartX, medBtnStartY, medBtnW, medBtnH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(medBtnStartX, medBtnStartY, medBtnW, medBtnH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("START/STOP", medBtnStartX + (medBtnW/2), medBtnStartY + (medBtnH/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI - PASTIKAN PERSIS SAMA DENGAN AWAL
            drawButton(medBtnStartX, medBtnStartY, medBtnW, medBtnH, "START/STOP", originalColor);
            return 1;
        }
        
        // 2. Tombol Ganti Mode (Medical Record)
        if (screenX > medBtnModeX && screenX < medBtnModeX + medBtnModeW &&
            screenY > medBtnModeY && screenY < medBtnModeY + medBtnModeH) {
            
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_PURPLE;
            
            // EFEK VISUAL
            tft.fillRoundRect(medBtnModeX, medBtnModeY, medBtnModeW, medBtnModeH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(medBtnModeX, medBtnModeY, medBtnModeW, medBtnModeH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("GANTI MODE", medBtnModeX + (medBtnModeW/2), medBtnModeY + (medBtnModeH/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(medBtnModeX, medBtnModeY, medBtnModeW, medBtnModeH, "GANTI MODE", originalColor);
            return 3;
        }
    }
    else if (currentMeasurementMode == 1) { // Blood Pressure
    // 2. Tombol Ganti Mode (Blood Pressure)
    if (screenX > bpBtnModeX && screenX < bpBtnModeX + bpBtnModeW &&
        screenY > bpBtnModeY && screenY < bpBtnModeY + bpBtnModeH) {
        
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_PURPLE;
            
            // EFEK VISUAL
            tft.fillRoundRect(bpBtnModeX, bpBtnModeY, bpBtnModeW, bpBtnModeH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(bpBtnModeX, bpBtnModeY, bpBtnModeW, bpBtnModeH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("GANTI MODE", bpBtnModeX + (bpBtnModeW/2), bpBtnModeY + (bpBtnModeH/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(bpBtnModeX, bpBtnModeY, bpBtnModeW, bpBtnModeH, "GANTI MODE", originalColor);
            return 3;
        }
    }
    else if (currentMeasurementMode == 2) { // ECG (BARU)
        // 1. Tombol Start/Stop (ECG)
        if (screenX > ecgBtnStartX && screenX < ecgBtnStartX + ecgBtnW &&
            screenY > ecgBtnStartY && screenY < ecgBtnStartY + ecgBtnH) {
            
            // SIMPAN WARNA ASLI untuk mengembalikan nanti
            static uint16_t originalColor = TFT_ACCENT_GREEN;
            
            // EFEK VISUAL: tombol berubah warna
            tft.fillRoundRect(ecgBtnStartX, ecgBtnStartY, ecgBtnW, ecgBtnH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(ecgBtnStartX, ecgBtnStartY, ecgBtnW, ecgBtnH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("START/STOP", ecgBtnStartX + (ecgBtnW/2), ecgBtnStartY + (ecgBtnH/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI - PASTIKAN PERSIS SAMA DENGAN AWAL
            drawButton(ecgBtnStartX, ecgBtnStartY, ecgBtnW, ecgBtnH, "START/STOP", originalColor);
            return 1;
        }
        
        // 2. Tombol Ganti Mode (ECG)
        if (screenX > ecgBtnModeX && screenX < ecgBtnModeX + ecgBtnModeW &&
            screenY > ecgBtnModeY && screenY < ecgBtnModeY + ecgBtnModeH) {
            
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_PURPLE;
            
            // EFEK VISUAL
            tft.fillRoundRect(ecgBtnModeX, ecgBtnModeY, ecgBtnModeW, ecgBtnModeH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(ecgBtnModeX, ecgBtnModeY, ecgBtnModeW, ecgBtnModeH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("GANTI MODE", ecgBtnModeX + (ecgBtnModeW/2), ecgBtnModeY + (ecgBtnModeH/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(ecgBtnModeX, ecgBtnModeY, ecgBtnModeW, ecgBtnModeH, "GANTI MODE", originalColor);
            return 3;
        }
    }
    else if (currentMeasurementMode == 3) { // EMG
        // 1. Tombol Start/Stop (EMG)
        if (screenX > 20 && screenX < 220 &&
            screenY > 245 && screenY < 295) {
            
            // Feedback visual - langsung panggil drawButton dengan warna asli
            drawButton(20, 245, 200, 50, "START/STOP", TFT_ACCENT_ORANGE);
            delay(150);
            
            // Kembalikan ke warna asli dan return
            drawButton(20, 245, 200, 50, "START/STOP", TFT_ACCENT_GREEN);
            return 1; // Kode untuk Start/Stop EMG
        }
        
        // 2. Tombol Ganti Mode (EMG)
        if (screenX > 240 && screenX < 440 &&
            screenY > 245 && screenY < 295) {
            
            // Feedback visual
            drawButton(240, 245, 200, 50, "GANTI MODE", TFT_ACCENT_ORANGE);
            delay(150);
            
            // Kembalikan ke warna asli
            drawButton(240, 245, 200, 50, "GANTI MODE", TFT_ACCENT_PURPLE);
            
            // KODE PENTING: Setelah tombol GANTI MODE ditekan, 
            // kembali ke mode selection dan reset semuanya
            inModeSelection = true;
            return 3; // Kode untuk Ganti Mode
        }
    }
    else if (currentMeasurementMode == 4) { // Body Position & Snore
        // Tombol Start/Stop (kiri bawah)
        if (screenX > 20 && screenX < 220 &&
            screenY > 245 && screenY < 295) {
            
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_GREEN;
            
            // EFEK VISUAL
            tft.fillRect(20, 245, 200, 50, TFT_ACCENT_ORANGE);
            tft.drawRect(20, 245, 200, 50, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("START/STOP", 20 + (200/2), 245 + (50/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(20, 245, 200, 50, "START/STOP", originalColor);
            return 1;
        }
        
        // Tombol Ganti Mode (kanan bawah)
        if (screenX > 240 && screenX < 440 &&
            screenY > 245 && screenY < 295) {
            
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_PURPLE;
            
            // EFEK VISUAL
            tft.fillRect(240, 245, 200, 50, TFT_ACCENT_ORANGE);
            tft.drawRect(240, 245, 200, 50, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("GANTI MODE", 240 + (200/2), 245 + (50/2), 1);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(240, 245, 200, 50, "GANTI MODE", originalColor);
            return 3;
        }
    }
    else { // Mode lainnya (1,3,4)
        // 1. Tombol Start/Stop
        if (screenX > btnStartX && screenX < btnStartX + btnW &&
            screenY > btnStartY && screenY < btnStartY + btnH) {
            
            // SIMPAN WARNA ASLI untuk mengembalikan nanti
            static uint16_t originalColor = TFT_ACCENT_GREEN;
            
            // EFEK VISUAL: tombol berubah warna
            tft.fillRoundRect(btnStartX, btnStartY, btnW, btnH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(btnStartX, btnStartY, btnW, btnH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(2);
            tft.drawString("START/STOP", btnStartX + (btnW/2), btnStartY + (btnH/2), 2);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI - PASTIKAN PERSIS SAMA DENGAN AWAL
            drawButton(btnStartX, btnStartY, btnW, btnH, "START/STOP", originalColor);
            return 1;
        }
        
        // 2. Tombol Ganti Mode
        if (screenX > btnNextX + 150 && screenX < btnNextX + 150 + btnW &&
            screenY > btnNextY && screenY < btnNextY + btnH) {
            
            // SIMPAN WARNA ASLI
            static uint16_t originalColor = TFT_ACCENT_PURPLE;
            
            // EFEK VISUAL
            tft.fillRoundRect(btnNextX + 150, btnNextY, btnW, btnH, 10, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(btnNextX + 150, btnNextY, btnW, btnH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(2);
            tft.drawString("GANTI MODE", btnNextX + 150 + (btnW/2), btnNextY + (btnH/2), 2);
            delay(100);
            
            // KEMBALIKAN KE WARNA ASLI
            drawButton(btnNextX + 150, btnNextY, btnW, btnH, "GANTI MODE", originalColor);
            return 3;
        }
    }
    
    return 0;
}

int DisplayManager::checkTouchUserInfo(int screenX, int screenY) {
    // Tombol Edit User (hanya jika ada user berdasarkan flag)
    if (hasUsersFlag) {
        if (screenX > btnEditUserX && screenX < btnEditUserX + btnEditW &&
            screenY > btnEditUserY && screenY < btnEditUserY + btnEditH) {
            // Feedback visual
            tft.fillRect(btnEditUserX, btnEditUserY, btnEditW, btnEditH, TFT_ACCENT_ORANGE);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("EDIT USER", btnEditUserX + (btnEditW/2), btnEditUserY + (btnEditH/2), 2);
            delay(100);
            // Restore tombol ke warna konsisten
            tft.fillRect(btnEditUserX, btnEditUserY, btnEditW, btnEditH, TFT_ACCENT_BLUE);
            tft.drawRoundRect(btnEditUserX, btnEditUserY, btnEditW, btnEditH, 10, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
            tft.drawString("EDIT USER", btnEditUserX + (btnEditW/2), btnEditUserY + (btnEditH/2), 2);
            return 91; // Kode untuk edit user
        }
    }
    
    // Tombol Kembali
    if (screenX > btnBackUserX && screenX < btnBackUserX + btnBackW &&
        screenY > btnBackUserY && screenY < btnBackUserY + btnBackH) {
        // Feedback visual
        tft.fillRect(btnBackUserX, btnBackUserY, btnBackW, btnBackH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("KEMBALI", btnBackUserX + (btnBackW/2), btnBackUserY + (btnBackH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnBackUserX, btnBackUserY, btnBackW, btnBackH, TFT_BUTTON_BG);
        tft.drawRoundRect(btnBackUserX, btnBackUserY, btnBackW, btnBackH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("KEMBALI", btnBackUserX + (btnBackW/2), btnBackUserY + (btnBackH/2), 2);
        return 90;
    }
    
    return 0;
}

int DisplayManager::checkTouchWifiConfig(int screenX, int screenY) {
    // Tombol Scan WiFi
    if (screenX > btnScanX && screenX < btnScanX + btnScanW &&
        screenY > btnScanY && screenY < btnScanY + btnScanH) {
        // Feedback visual
        tft.fillRect(btnScanX, btnScanY, btnScanW, btnScanH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SCAN WiFi", btnScanX + (btnScanW/2), btnScanY + (btnScanH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnScanX, btnScanY, btnScanW, btnScanH, TFT_ACCENT_BLUE);
        tft.drawRoundRect(btnScanX, btnScanY, btnScanW, btnScanH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
        tft.drawString("SCAN WiFi", btnScanX + (btnScanW/2), btnScanY + (btnScanH/2), 2);
        return 41;
    }
    
    // Tombol Kembali
    if (screenX > btnBackWifiX && screenX < btnBackWifiX + btnBackWifiW &&
        screenY > btnBackWifiY && screenY < btnBackWifiY + btnBackWifiH) {
        // Feedback visual
        tft.fillRect(btnBackWifiX, btnBackWifiY, btnBackWifiW, btnBackWifiH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("KEMBALI", btnBackWifiX + (btnBackWifiW/2), btnBackWifiY + (btnBackWifiH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnBackWifiX, btnBackWifiY, btnBackWifiW, btnBackWifiH, TFT_BUTTON_BG);
        tft.drawRoundRect(btnBackWifiX, btnBackWifiY, btnBackWifiW, btnBackWifiH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("KEMBALI", btnBackWifiX + (btnBackWifiW/2), btnBackWifiY + (btnBackWifiH/2), 2);
        return 90;
    }
    
    return 0;
}

// FUNGSI YANG DIPERBARUI: checkTouchEditUser untuk menangani 4 tombol
int DisplayManager::checkTouchEditUser(int screenX, int screenY) {
    // Tombol Edit Nama
    if (screenX > btnEditNameX && screenX < btnEditNameX + btnEditNameW &&
        screenY > btnEditNameY && screenY < btnEditNameY + btnEditNameH) {
        // Feedback visual
        tft.fillRect(btnEditNameX, btnEditNameY, btnEditNameW, btnEditNameH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("EDIT NAMA", btnEditNameX + (btnEditNameW/2), btnEditNameY + (btnEditNameH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnEditNameX, btnEditNameY, btnEditNameW, btnEditNameH, TFT_ACCENT_BLUE);
        tft.drawRoundRect(btnEditNameX, btnEditNameY, btnEditNameW, btnEditNameH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
        tft.drawString("EDIT NAMA", btnEditNameX + (btnEditNameW/2), btnEditNameY + (btnEditNameH/2), 2);
        return 92; // Kode untuk edit nama
    }
    
    // Tombol Edit Token
    if (screenX > btnEditKeyX && screenX < btnEditKeyX + btnEditKeyW &&
        screenY > btnEditKeyY && screenY < btnEditKeyY + btnEditKeyH) {
        // Feedback visual
        tft.fillRect(btnEditKeyX, btnEditKeyY, btnEditKeyW, btnEditKeyH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("EDIT TOKEN", btnEditKeyX + (btnEditKeyW/2), btnEditKeyY + (btnEditKeyH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnEditKeyX, btnEditKeyY, btnEditKeyW, btnEditKeyH, TFT_ACCENT_GREEN);
        tft.drawRoundRect(btnEditKeyX, btnEditKeyY, btnEditKeyW, btnEditKeyH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_GREEN);
        tft.drawString("EDIT TOKEN", btnEditKeyX + (btnEditKeyW/2), btnEditKeyY + (btnEditKeyH/2), 2);
        return 93; // Kode untuk edit token
    }
    
    // Tombol Edit Device Label
    if (screenX > btnEditDeviceX && screenX < btnEditDeviceX + btnEditKeyW &&
        screenY > btnEditDeviceY && screenY < btnEditDeviceY + btnEditKeyH) {
        // Feedback visual
        tft.fillRect(btnEditDeviceX, btnEditDeviceY, btnEditKeyW, btnEditKeyH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("EDIT DEVICE", btnEditDeviceX + (btnEditKeyW/2), btnEditDeviceY + (btnEditKeyH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnEditDeviceX, btnEditDeviceY, btnEditKeyW, btnEditKeyH, TFT_ACCENT_PURPLE);
        tft.drawRoundRect(btnEditDeviceX, btnEditDeviceY, btnEditKeyW, btnEditKeyH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_PURPLE);
        tft.drawString("EDIT DEVICE", btnEditDeviceX + (btnEditKeyW/2), btnEditDeviceY + (btnEditKeyH/2), 2);
        return 94; // Kode untuk edit device label
    }
    
    // Tombol Batal
    if (screenX > btnCancelEditX && screenX < btnCancelEditX + btnCancelEditW &&
        screenY > btnCancelEditY && screenY < btnCancelEditY + btnCancelEditH) {
        // Feedback visual
        tft.fillRect(btnCancelEditX, btnCancelEditY, btnCancelEditW, btnCancelEditH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("KEMBALI", btnCancelEditX + (btnCancelEditW/2), btnCancelEditY + (btnCancelEditH/2), 2);
        delay(100);
        // Restore tombol ke warna konsisten
        tft.fillRect(btnCancelEditX, btnCancelEditY, btnCancelEditW, btnCancelEditH, TFT_BUTTON_BG);
        tft.drawRoundRect(btnCancelEditX, btnCancelEditY, btnCancelEditW, btnCancelEditH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("KEMBALI", btnCancelEditX + (btnCancelEditW/2), btnCancelEditY + (btnCancelEditH/2), 2);
        return 95; // Kode untuk batal edit
    }
    
    return 0;
}

int DisplayManager::checkTouchKeyboard(int screenX, int screenY) {
    if (!keyboardActive) return 0;
    
    // Cek tombol SYM (jika dalam mode huruf)
    if (!symbolMode) {
        // Posisi baru: baris ke-4, kolom ke-9 (sebelum tombol Enter)
        int symBtnX = keyStartX + 8 * (keyWidth + keySpacing);
        int symBtnY = keyStartY + 3 * (keyHeight + keySpacing);
        
        if (screenX > symBtnX && screenX < symBtnX + keyWidth &&
            screenY > symBtnY && screenY < symBtnY + keyHeight) {
            
            // Feedback visual
            tft.fillRoundRect(symBtnX, symBtnY, keyWidth, keyHeight, 5, TFT_ACCENT_ORANGE);
            tft.drawRoundRect(symBtnX, symBtnY, keyWidth, keyHeight, 5, TFT_BUTTON_BORDER);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(MC_DATUM);
            tft.setTextSize(1);
            tft.drawString("SYM", symBtnX + keyWidth/2, symBtnY + keyHeight/2);
            delay(50);
            
            // Switch to symbol mode
            symbolMode = true;
            drawKeyboard();
            return 5;
        }
    }
    
    // Cek tombol keyboard utama
    for (int row = 0; row < KEY_ROWS; row++) {
        for (int col = 0; col < KEY_COLS; col++) {
            int x = keyStartX + col * (keyWidth + keySpacing);
            int y = keyStartY + row * (keyHeight + keySpacing);
            
            // Skip tombol kosong (spasi)
            if (symbolMode) {
                char key = keyboardSymbol[row][col];
                if (key == ' ') continue;
            }
            
            if (screenX > x && screenX < x + keyWidth &&
                screenY > y && screenY < y + keyHeight) {
                
                // Get the key based on current layout
                char key;
                if (symbolMode) {
                    key = keyboardSymbol[row][col];
                } else if (shiftActive || capsLock) {
                    key = keyboardUpper[row][col];
                } else {
                    key = keyboardLower[row][col];
                }
                
                // Skip spasi kosong
                if (key == ' ') continue;
                
                // Convert ke string untuk display
                String keyStr;
                if (key == 8) keyStr = "BS";
                else if (key == 13) keyStr = "OK";
                else if (key == '^') keyStr = "SFT";
                else if (key == 26) keyStr = "ABC";
                else if (key == 27) keyStr = "ESC";
                else keyStr = String(key);
                
                // Warna asli tombol
                uint16_t originalColor = TFT_ACCENT_BLUE;
                if (key == 8) originalColor = TFT_ACCENT_RED;
                else if (key == 13) originalColor = TFT_ACCENT_GREEN;
                else if (key == '^') originalColor = (shiftActive || capsLock) ? TFT_ACCENT_ORANGE : TFT_ACCENT_ORANGE;
                else if (key == 26) originalColor = TFT_ACCENT_PURPLE;
                else if (key == 27) originalColor = TFT_ACCENT_ORANGE;
                else if (isdigit(key)) originalColor = TFT_ACCENT_PURPLE;
                else if (symbolMode) originalColor = TFT_ACCENT_CYAN;
                
                // Feedback visual - TOMBOL KUNING
                tft.fillRoundRect(x, y, keyWidth, keyHeight, 5, TFT_ACCENT_ORANGE);
                tft.drawRoundRect(x, y, keyWidth, keyHeight, 5, TFT_BUTTON_BORDER);
                tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
                tft.setTextDatum(MC_DATUM);
                tft.setTextSize(1);
                tft.drawString(keyStr, x + keyWidth/2, y + keyHeight/2);
                delay(50);
                
                // KEMBALIKAN KE WARNA ASLI
                tft.fillRoundRect(x, y, keyWidth, keyHeight, 5, originalColor);
                tft.drawRoundRect(x, y, keyWidth, keyHeight, 5, TFT_BUTTON_BORDER);
                tft.setTextColor(TFT_BG_COLOR, originalColor);
                tft.drawString(keyStr, x + keyWidth/2, y + keyHeight/2);
                
                // Process key
                processKeyPress(key);
                return 1;
            }
        }
    }
    
    // Cek tombol SPACE
    if (screenX > spaceKeyX && screenX < spaceKeyX + spaceKeyW &&
        screenY > spaceKeyY && screenY < spaceKeyY + spaceKeyH) {
        
        // Feedback visual - TOMBOL KUNING
        tft.fillRoundRect(spaceKeyX, spaceKeyY, spaceKeyW, spaceKeyH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(spaceKeyX, spaceKeyY, spaceKeyW, spaceKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("SPACE", spaceKeyX + spaceKeyW/2, spaceKeyY + spaceKeyH/2);
        delay(50); // Efek visual lebih singkat
        
        // KEMBALIKAN KE WARNA ASLI
        tft.fillRoundRect(spaceKeyX, spaceKeyY, spaceKeyW, spaceKeyH, 5, TFT_BUTTON_BG);
        tft.drawRoundRect(spaceKeyX, spaceKeyY, spaceKeyW, spaceKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("SPACE", spaceKeyX + spaceKeyW/2, spaceKeyY + spaceKeyH/2);
        
        // Add space
        if (keyboardBuffer.length() < 100) {
            keyboardBuffer = keyboardBuffer.substring(0, cursorPos) + 
                           " " + 
                           keyboardBuffer.substring(cursorPos);
            cursorPos++;
            drawInputField();
            // Jika shift aktif (temporary), matikan dan redraw
            if (shiftActive && !capsLock) {
                shiftActive = false;
                drawKeyboard();
            }
        }
        return 2;
    }
    
    // Cek tombol BACK/KEMBALI (BARU)
    if (screenX > backKeyX && screenX < backKeyX + backKeyW &&
        screenY > backKeyY && screenY < backKeyY + backKeyH) {
        
        // Feedback visual - TOMBOL KUNING
        tft.fillRoundRect(backKeyX, backKeyY, backKeyW, backKeyH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(backKeyX, backKeyY, backKeyW, backKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("BACK", backKeyX + backKeyW/2, backKeyY + backKeyH/2);
        delay(50); // Efek visual lebih singkat
        
        // KEMBALIKAN KE WARNA ASLI
        tft.fillRoundRect(backKeyX, backKeyY, backKeyW, backKeyH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(backKeyX, backKeyY, backKeyW, backKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.drawString("BACK", backKeyX + backKeyW/2, backKeyY + backKeyH/2);
        
        // Simulasikan tombol ESC
        keyboardCancelled = true;
        keyboardActive = false;
        return 4;
    }
    
    // Cek tombol CLEAR
    if (screenX > clearKeyX && screenX < clearKeyX + clearKeyW &&
        screenY > clearKeyY && screenY < clearKeyY + clearKeyH) {
        
        // Feedback visual - TOMBOL KUNING
        tft.fillRoundRect(clearKeyX, clearKeyY, clearKeyW, clearKeyH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(clearKeyX, clearKeyY, clearKeyW, clearKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("CLEAR", clearKeyX + clearKeyW/2, clearKeyY + clearKeyH/2);
        delay(50); // Efek visual lebih singkat
        
        // KEMBALIKAN KE WARNA ASLI
        tft.fillRoundRect(clearKeyX, clearKeyY, clearKeyW, clearKeyH, 5, TFT_ACCENT_RED);
        tft.drawRoundRect(clearKeyX, clearKeyY, clearKeyW, clearKeyH, 5, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_RED);
        tft.drawString("CLEAR", clearKeyX + clearKeyW/2, clearKeyY + clearKeyH/2);
        
        // Clear buffer
        keyboardBuffer = "";
        cursorPos = 0;
        drawInputField();
        // Jika shift aktif (temporary), matikan dan redraw
        if (shiftActive && !capsLock) {
            shiftActive = false;
            drawKeyboard();
        }
        return 3;
    }
    
    return 0;
}

int DisplayManager::checkTouchUserList(int screenX, int screenY) {
    // Cek tombol +
    if (screenX > btnAddUserX && screenX < btnAddUserX + btnAddW && screenY > btnAddUserY && screenY < btnAddUserY + btnAddH) {
        // Feedback visual
        tft.fillRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_BUTTON_BORDER);
        drawPlusIcon(btnAddUserX + 10, btnAddUserY + 10); // Gunakan offset yang sama
        delay(100);

        // Kembali ke warna asli
        tft.fillRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_ACCENT_GREEN);
        tft.drawRoundRect(btnAddUserX, btnAddUserY, btnAddW, btnAddH, 5, TFT_BUTTON_BORDER);
        drawPlusIcon(btnAddUserX + 10, btnAddUserY + 10);
        return 95;
    }

    // Cek tombol -
    if (userList.size() > 0 && screenX > btnDeleteUserX && screenX < btnDeleteUserX + btnDeleteW && screenY > btnDeleteUserY && screenY < btnDeleteUserY + btnDeleteH) {
        // Feedback visual
        tft.fillRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_ACCENT_ORANGE);
        tft.drawRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_BUTTON_BORDER);
        drawMinusIcon(btnDeleteUserX + 10, btnDeleteUserY + 10);
        delay(100);

        // Kembali ke warna asli
        tft.fillRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_ACCENT_RED);
        tft.drawRoundRect(btnDeleteUserX, btnDeleteUserY, btnDeleteW, btnDeleteH, 5, TFT_BUTTON_BORDER);
        drawMinusIcon(btnDeleteUserX + 10, btnDeleteUserY + 10);
        return 96;
    }

    // Cek sentuh area user
    int startIdx = userListPage * USER_PER_PAGE;
    int endIdx = min(startIdx + USER_PER_PAGE, (int)userList.size());

    for (int i = startIdx; i < endIdx; i++) {
        int yPos = 120 + (i - startIdx) * 30;

        // Cek apakah menyentuh item user
        if (screenX > 40 && screenX < 440 && screenY > yPos - 15 && screenY < yPos + 13) {
            // Feedback visual
            tft.fillRect(40, yPos - 15, 400, 28, TFT_ACCENT_ORANGE);
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
            tft.setTextDatum(ML_DATUM);
            tft.setTextSize(2);

            String displayText = "[" + String(i+1) + "]" + userList[i];
            if (displayText.length() > 25) {
                displayText = displayText.substring(0, 22) + "...";
            }

            tft.drawString(displayText, 50, yPos);
            delay(100);

            // Kembalikan warna asli setelah delay
            if (i == selectedUserIndex) {
                tft.fillRect(40, yPos - 15, 400, 28, TFT_ACCENT_BLUE);
                tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_BLUE);
            } else {
                tft.fillRect(40, yPos - 15, 400, 28, TFT_BG_COLOR);
                tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
            }
            tft.drawString(displayText, 50, yPos);
            tft.drawRect(40, yPos - 15, 400, 28, TFT_BUTTON_BORDER);

            return 100 + i;
        }
    }

    // Cek tombol sebelumnya - PERBAIKAN DI SINI
    if (userListPage > 0 && screenX > btnPrevPageX && screenX < btnPrevPageX + btnPageW && screenY > btnPrevPageY && screenY < btnPrevPageY + btnPageH) {
        // Feedback visual
        tft.fillRect(btnPrevPageX, btnPrevPageY, btnPageW, btnPageH, TFT_ACCENT_ORANGE);
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("<SEBELUM>", btnPrevPageX + (btnPageW/2), btnPrevPageY + (btnPageH/2), 2);
        delay(100);
        
        // Kurangi halaman dan refresh tampilan
        userListPage--;
        refreshUserListDisplay();
        return 80;
    }

    // Cek tombol berikutnya - PERBAIKAN DI SINI
    if ((userListPage + 1) * USER_PER_PAGE < userList.size() && screenX > btnNextPageX && screenX < btnNextPageX + btnNextPageW && screenY > btnNextPageY && screenY < btnNextPageY + btnNextPageH) {
        // Feedback visual
        tft.fillRect(btnNextPageX, btnNextPageY, btnNextPageW, btnNextPageH, TFT_ACCENT_ORANGE); // Ubah dari TFT_YELLOW ke TFT_ACCENT_ORANGE
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE); // Ubah dari TFT_BLACK ke TFT_BG_COLOR (putih/abu-abu muda)
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("<BERIKUT>", btnNextPageX + (btnNextPageW/2), btnNextPageY + (btnNextPageH/2), 2);
        delay(100);
        
        // Tambah halaman dan refresh tampilan
        userListPage++;
        refreshUserListDisplay();
        return 81;
    }

    // Cek tombol batal - PERBAIKAN DI SINI
    if (screenX > btnCancelListX && screenX < btnCancelListX + btnCancelListW && screenY > btnCancelListY && screenY < btnCancelListY + btnCancelListH) {
        // Feedback visual
        tft.fillRect(btnCancelListX, btnCancelListY, btnCancelListW, btnCancelListH, TFT_ACCENT_ORANGE); // Ubah dari TFT_YELLOW ke TFT_ACCENT_ORANGE
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE); // Ubah dari TFT_BLACK ke TFT_BG_COLOR
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.drawString("KEMBALI", btnCancelListX + (btnCancelListW/2), btnCancelListY + (btnCancelListH/2), 2);
        delay(100);
        
        // Kembalikan warna asli
        tft.fillRect(btnCancelListX, btnCancelListY, btnCancelListW, btnCancelListH, TFT_BUTTON_BG);
        tft.drawRoundRect(btnCancelListX, btnCancelListY, btnCancelListW, btnCancelListH, 10, TFT_BUTTON_BORDER);
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG);
        tft.drawString("KEMBALI", btnCancelListX + (btnCancelListW/2), btnCancelListY + (btnCancelListH/2), 2);
        
        return 90;
    }

    return 0;
}

// ==================== FUNGSI MODE PENGUKURAN ====================

void DisplayManager::showMeasurementModeSelection(int currentModeIndex) {
    clearContentArea();
    inModeSelection = true;
    selectedModeIndex = currentModeIndex;
    currentScreenMode = 8; // Mode khusus untuk pemilihan mode
    
    // Judul
    tft.setTextColor(TFT_ACCENT_CYAN, TFT_BG_COLOR); // Ubah dari TFT_CYAN, TFT_BLACK
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("PILIH MODE PENGUKURAN", 240, 60);
    
    // Tampilkan daftar 5 mode dalam bentuk list
    for (int i = 0; i < 5; i++) {
        int yPos = 100 + (i * 35);
        
        // Highlight mode yang sedang dipilih
        if (i == selectedModeIndex) {
            tft.fillRect(40, yPos - 15, 400, 30, TFT_ACCENT_GREEN); // Ubah dari TFT_DARKGREEN ke TFT_ACCENT_GREEN
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_GREEN); // Ubah dari TFT_WHITE ke TFT_BG_COLOR
        } else {
            tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Ubah dari TFT_WHITE, TFT_BLACK
        }
        
        // Format: [nomor] nama mode
        String displayText = "[" + String(i+1) + "] " + measurementModes[i];
        
        // Jika teks terlalu panjang, potong
        if (displayText.length() > 30) {
            displayText = displayText.substring(0, 27) + "...";
        }
        
        tft.setTextDatum(ML_DATUM);
        tft.setTextSize(2);
        tft.drawString(displayText, 50, yPos);
        
        // Gambar kotak seleksi
        tft.drawRect(40, yPos - 15, 400, 30, TFT_BUTTON_BORDER); // Ubah dari TFT_WHITE ke TFT_BUTTON_BORDER
    }
    
    // Tombol Kembali
    drawButton(190, 257, 100, 40, "KEMBALI", TFT_BUTTON_BG); // Ubah dari TFT_DARKGREY ke TFT_BUTTON_BG
    
    updateStatus("Sentuh mode untuk memilih");
}

String DisplayManager::getMeasurementModeName(int index) {
    if (index >= 0 && index < (int)measurementModes.size()) {
        return measurementModes[index];
    }
    return "Unknown";
}

int DisplayManager::checkTouchModeSelection(int screenX, int screenY) {
    // Cek sentuhan pada daftar mode
    for (int i = 0; i < 5; i++) {
        int yPos = 100 + (i * 35);
        
        if (screenX > 40 && screenX < 440 &&
            screenY > yPos - 15 && screenY < yPos + 15) {
            
            // Feedback visual
            tft.fillRect(40, yPos - 15, 400, 30, TFT_ACCENT_ORANGE); // Ubah dari TFT_YELLOW
            tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE); // Ubah dari TFT_BLACK
            tft.setTextDatum(ML_DATUM);
            tft.setTextSize(2);
            
            String displayText = "[" + String(i+1) + "] " + measurementModes[i];
            if (displayText.length() > 30) {
                displayText = displayText.substring(0, 27) + "...";
            }
            
            tft.drawString(displayText, 50, yPos);
            delay(100);
            
            // Kembalikan ke warna asli
            if (i == selectedModeIndex) {
                tft.fillRect(40, yPos - 15, 400, 30, TFT_ACCENT_GREEN); // Ubah dari TFT_DARKGREEN
                tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_GREEN); // Ubah dari TFT_WHITE
            } else {
                tft.fillRect(40, yPos - 15, 400, 30, TFT_BG_COLOR); // Ubah dari TFT_BLACK
                tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Ubah dari TFT_WHITE
            }
            tft.drawString(displayText, 50, yPos);
            tft.drawRect(40, yPos - 15, 400, 30, TFT_BUTTON_BORDER); // Ubah dari TFT_WHITE
            
            return 200 + i; // Kode 200 + index mode
        }
    }
    
    // Cek tombol Kembali
    if (screenX > 190 && screenX < 290 && screenY > 260 && screenY < 300) {
        // Feedback visual
        tft.fillRect(190, 257, 100, 40, TFT_ACCENT_ORANGE); // Ubah dari TFT_YELLOW
        tft.setTextColor(TFT_BG_COLOR, TFT_ACCENT_ORANGE); // Ubah dari TFT_BLACK
        tft.setTextDatum(MC_DATUM);
        tft.drawString("KEMBALI", 240, 280, 2);
        delay(100);
        
        // Kembalikan ke warna asli
        tft.fillRect(190, 257, 100, 40, TFT_BUTTON_BG); // Ubah dari TFT_DARKGREY
        tft.drawRoundRect(190, 257, 100, 40, 10, TFT_BUTTON_BORDER); // Ubah dari TFT_WHITE
        tft.setTextColor(TFT_BG_COLOR, TFT_BUTTON_BG); // Ubah dari TFT_WHITE
        tft.drawString("KEMBALI", 240, 280, 2);
        
        return 90; // Kode untuk kembali
    }
    
    return 0;
}

void DisplayManager::startCalibration() {
    if (!touchSupported) {
        Serial.println("Touchscreen tidak didukung");
        return;
    }
    
    tft.fillScreen(TFT_BG_COLOR); // Ubah dari TFT_BLACK
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Ubah dari TFT_WHITE, TFT_BLACK
    tft.setTextSize(2);
    tft.drawString("TOUCH CALIBRATION", 100, 30);
    tft.setTextSize(1);
    tft.drawString("Sentuh 4 pojok berurutan", 100, 70);
    
    Serial.println("\n=== CALIBRATION MODE ===");
    
    int points[4][2] = {{10, 10}, {470, 10}, {10, 310}, {470, 310}};
    int touchX[4], touchY[4];
    
    for (int i = 0; i < 4; i++) {
        // Tampilkan titik
        tft.fillCircle(points[i][0], points[i][1], 8, TFT_ACCENT_RED); // Ubah dari TFT_RED
        tft.setCursor(points[i][0] - 20, points[i][1] + 15);
        tft.printf("P%d", i+1);
        
        Serial.printf("\nPOINT %d: Sentuh (%d, %d)\n", i+1, points[i][0], points[i][1]);
        
        // Tunggu sentuhan
        while (!ts.touched()) delay(50);
        
        TS_Point p = ts.getPoint();
        touchX[i] = p.x;
        touchY[i] = p.y;
        
        Serial.printf("  Raw: p.x=%d, p.y=%d\n", p.x, p.y);
        
        // Tampilkan nilai
        tft.setCursor(points[i][0] - 30, points[i][1] + 30);
        tft.printf("(%d,%d)", p.x, p.y);
        
        // Hapus titik merah
        tft.fillCircle(points[i][0], points[i][1], 10, TFT_BG_COLOR); // Ubah dari TFT_BLACK
        delay(1000);
    }
    
    // Analisis dan rekomendasi
    int minX = min(min(touchX[0], touchX[2]), min(touchX[1], touchX[3]));
    int maxX = max(max(touchX[0], touchX[2]), max(touchX[1], touchX[3]));
    int minY = min(min(touchY[0], touchY[1]), min(touchY[2], touchY[3]));
    int maxY = max(max(touchY[0], touchY[1]), max(touchY[2], touchY[3]));
    
    Serial.println("\n=== REKOMENDASI UNTUK DisplayManager.h ===");
    Serial.printf("int touchMinX = %d;    // Minimal p.y\n", minY);
    Serial.printf("int touchMaxX = %d;    // Maksimal p.y\n", maxY);
    Serial.printf("int touchMinY = %d;    // Minimal p.x\n", minX);
    Serial.printf("int touchMaxY = %d;    // Maksimal p.x\n", maxX);
    
    // Update nilai calibration
    touchMinX = minY;
    touchMaxX = maxY;
    touchMinY = minX;
    touchMaxY = maxX;
    
    // Tampilkan hasil
    tft.fillScreen(TFT_BG_COLOR); // Ubah dari TFT_BLACK
    tft.setTextSize(2);
    tft.drawString("CALIBRATION SELESAI", 80, 100);
    tft.setTextSize(1);
    tft.setCursor(50, 150);
    tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR); // Pastikan warna teks konsisten
    tft.printf("p.x: %d - %d", minX, maxX);
    tft.setCursor(50, 170);
    tft.printf("p.y: %d - %d", minY, maxY);
    tft.setCursor(50, 200);
    tft.print("Update nilai di kode");
    
    delay(5000);
}