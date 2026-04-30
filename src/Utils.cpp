#include "Utils.h"

String readLineFromSerial(uint32_t timeoutMs) {
  String s;
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\r') continue;
      if (c == '\n') {
        if (s.length() > 0) return s;
        else continue; 
      }
      s += c;
    }
    delay(10);
  }
  return s;
}