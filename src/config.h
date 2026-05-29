#pragma once
#include <Arduino.h>

// === UART ===
#define DEBUG_UART Serial

// === SPI1 (NRF24L01) ===
#define NRF24_SPI_MOSI  PA7
#define NRF24_SPI_MISO  PA6
#define NRF24_SPI_SCK   PA5
#define NRF24_CE        PA8
#define NRF24_CSN       PB12

// === NRF24 Konfiguration ===
#define NRF24_CHANNEL    76
#define NRF24_DATA_RATE  RF24_2MBPS
#define NRF24_PA_LEVEL   RF24_PA_MAX

// === NRF24 Adressen ===
constexpr uint8_t CONTROLLER_ADDRESS[6] = "CTR01";
constexpr uint8_t RECEIVER_ADDRESS[6]   = "RCV01";

// === I2C (OLED + optional BMP280) ===
#define I2C_SDA_PIN     PB7
#define I2C_SCL_PIN     PB6
#define OLED_I2C_ADDR   0x3C
#define BMP280_ADDRESS  0x76

// === KY-023 Thumbsticks ===
#define LEFT_X_PIN      PA0
#define LEFT_Y_PIN      PA1
#define LEFT_SW_PIN     PA4
#define RIGHT_X_PIN     PA2
#define RIGHT_Y_PIN     PA3
#define RIGHT_SW_PIN    PB3

// === Timing ===
#define SEND_INTERVAL_MS    20
#define BUTTON_COOLDOWN_MS  200

// === ADC ===
#define ADC_CENTER_VALUE    2047
#define ADC_DEADBAND        50
#define ADC_MIN_VALUE       0
#define ADC_MAX_VALUE       4095