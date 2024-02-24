#include <Arduino.h>

void printBytes(uint8_t *bytes, uint32_t count) {
    for(uint32_t i = 0; i < count; i++) {
      Serial.printf("0x%x ", bytes[i]);
    }
}