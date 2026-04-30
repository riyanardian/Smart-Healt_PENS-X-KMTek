#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Membaca input serial sampai newline dengan timeout
String readLineFromSerial(uint32_t timeoutMs = 30000);

#endif