#pragma once
#include <Arduino.h>

// Sender → Empfänger
#pragma pack(push, 1)
struct ControllerData {
    uint16_t left_x;
    uint16_t left_y;
    uint16_t right_x;
    uint16_t right_y;
    uint8_t  left_click;
    uint8_t  right_click;
    uint8_t  battery_level;
    uint8_t  checksum;
};
#pragma pack(pop)

// Empfänger → Sender (Telemetrie-Rückkanal)
#pragma pack(push, 1)
struct TelemetryData {
    int16_t  gyro_x;        // dps × 10
    int16_t  gyro_y;
    int16_t  gyro_z;
    int16_t  accel_x;       // g × 1000
    int16_t  accel_y;
    int16_t  accel_z;
    int16_t  altitude_cm;   // cm
    uint8_t  battery_pct;   // 0-100%
    uint8_t  status;
    uint8_t  checksum;
};
#pragma pack(pop)           // ← war vorher push, jetzt korrekt pop