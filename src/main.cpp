#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "drivers/shared_types.h"
#include "drivers/nrf24_driver.h"
#include "drivers/thumbstick.h"
#include "drivers/ssd1306_driver.h"

// === Global Objects ===
Thumbstick leftStick(LEFT_X_PIN,  LEFT_Y_PIN,  LEFT_SW_PIN,  "LEFT");
Thumbstick rightStick(RIGHT_X_PIN, RIGHT_Y_PIN, RIGHT_SW_PIN, "RIGHT");
NRF24Driver radio(NRF24_CE, NRF24_CSN);
SSD1306Driver oled;

// === Telemetrie-Puffer ===
static TelemetryData telemetry = {};

// === Timing ===
static uint32_t last_send_time  = 0;
static uint32_t last_oled_time  = 0;
static uint32_t tx_count        = 0;
static uint32_t tx_fail_count   = 0;
static uint32_t tx_total        = 0;   // ← aus loop() raus, global
static uint32_t tx_fail         = 0;   // ← aus loop() raus, global

void printStartupBanner() {
    DEBUG_UART.println(F("\n==================================="));
    DEBUG_UART.println(F("STM32F411CEU6 Controller - TX Mode"));
    DEBUG_UART.println(F("==================================="));
    DEBUG_UART.print(F("Build: "));
    DEBUG_UART.print(__DATE__);
    DEBUG_UART.print(F(" "));
    DEBUG_UART.println(__TIME__);
}

void updateOLED(const ControllerData& data) {
    oled.clear();

    oled.setCursor(0, 0);
    oled.print(F("L:"));
    oled.print((unsigned int)data.left_x);
    oled.print(F(","));
    oled.print((unsigned int)data.left_y);

    oled.setCursor(0, 1);
    oled.print(F("R:"));
    oled.print((unsigned int)data.right_x);
    oled.print(F(","));
    oled.print((unsigned int)data.right_y);

    oled.setCursor(0, 2);
    oled.print(F("G:"));
    oled.print((unsigned int)(uint16_t)telemetry.gyro_x);
    oled.print(F(","));
    oled.print((unsigned int)(uint16_t)telemetry.gyro_y);

    oled.setCursor(0, 3);
    oled.print(F("Alt:"));
    oled.print((unsigned int)(uint16_t)telemetry.altitude_cm);
    oled.print(F("cm"));

    oled.setCursor(0, 4);
    oled.print(F("TX:"));
    oled.print(tx_count);
    oled.print(F(" F:"));
    oled.print(tx_fail_count);

    // Zeile 5: Fehlerrate
    oled.setCursor(0, 5);
    oled.print(F("Fail:"));
    oled.print(tx_total > 0 ? (tx_fail * 100) / tx_total : 0);
    oled.print(F("%"));
}

void setup() {
    analogReadResolution(12);
    DEBUG_UART.begin(115200);
    delay(1000);

    pinMode(PC13, OUTPUT);         // ← NEU
    digitalWrite(PC13, HIGH);      // ← LED aus (active LOW)

    printStartupBanner();

    // I2C für OLED
    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
    oled.init(OLED_I2C_ADDR);
    oled.clear();
    oled.setCursor(0, 0);
    oled.print(F("Initializing..."));
    DEBUG_UART.println(F("[OLED] OK"));

    // Thumbsticks
    leftStick.begin();
    rightStick.begin();
    DEBUG_UART.println(F("[THUMBSTICK] OK"));

    // NRF24 – TX-Modus
    DEBUG_UART.print(F("[NRF24] Initializing... "));
    if (!radio.beginTX()) {
        DEBUG_UART.println(F("FAILED!"));
        oled.setCursor(0, 2);
        oled.print(F("NRF24 FAILED!"));
        while (1) { delay(1000); }
    }
    radio.printDetails();
    DEBUG_UART.println(F("OK"));

    // Handshake
    oled.setCursor(0, 1);
    oled.print(F("Waiting drone..."));
    if (!radio.waitForDrone(0)) {
        DEBUG_UART.println(F("[ERROR] Drone not responding!"));
        while (1) { delay(1000); }
    }

    oled.clear();
    oled.setCursor(0, 0);
    oled.print(F("Ready!"));

    DEBUG_UART.println(F("==================================="));
    DEBUG_UART.println(F("System ready."));
    DEBUG_UART.println(F("===================================\n"));
}

void loop() {
    uint32_t now = millis();

    // ─── 50Hz senden ────────────────────────────────────────────────────────
    if (now - last_send_time >= SEND_INTERVAL_MS) {
        last_send_time = now;

        leftStick.update();
        rightStick.update();

        ControllerData data;
        data.left_x        = leftStick.getRawX();
        data.left_y        = leftStick.getRawY();
        data.right_x       = rightStick.getRawX();
        data.right_y       = rightStick.getRawY();
        data.left_click    = leftStick.wasClicked()  ? 1 : 0;
        data.right_click   = rightStick.wasClicked() ? 1 : 0;
        data.battery_level = 255;
        data.checksum      = 0;

        leftStick.clearClickFlag();
        rightStick.clearClickFlag();

        bool success = radio.sendMessage(data, &telemetry);
        tx_total++;
        tx_count++;
        if (!success) {
            tx_fail++;
            tx_fail_count++;
            DEBUG_UART.print(F("[NRF24] TX failed! isReady="));
            DEBUG_UART.println(radio.isReady() ? F("ja") : F("nein"));  // ← NEU
        }

        // LED-Blink bei Erfolg
        if (success) {
            digitalWrite(PC13, LOW);
            delayMicroseconds(100);
            digitalWrite(PC13, HIGH);
        }

        if (data.left_click)  DEBUG_UART.println(F("[EVENT] Left click"));
        if (data.right_click) DEBUG_UART.println(F("[EVENT] Right click"));
    }

    // ─── 10Hz OLED + UART ───────────────────────────────────────────────────
    if (now - last_oled_time >= 100) {
        last_oled_time = now;

        ControllerData display_data;
        display_data.left_x  = leftStick.getRawX();
        display_data.left_y  = leftStick.getRawY();
        display_data.right_x = rightStick.getRawX();
        display_data.right_y = rightStick.getRawY();

        updateOLED(display_data);

        DEBUG_UART.print(F(" TX:"));
        DEBUG_UART.print(tx_total);
        DEBUG_UART.print(F(" Fail:"));
        DEBUG_UART.print(tx_fail);
        DEBUG_UART.print(F(" ("));
        DEBUG_UART.print(tx_total > 0 ? (tx_fail * 100) / tx_total : 0);
        DEBUG_UART.print(F("%) "));
        DEBUG_UART.print(F("[TELEM] G("));
        DEBUG_UART.print(telemetry.gyro_x / 10.0f, 1);
        DEBUG_UART.print(F(","));
        DEBUG_UART.print(telemetry.gyro_y / 10.0f, 1);
        DEBUG_UART.print(F(","));
        DEBUG_UART.print(telemetry.gyro_z / 10.0f, 1);
        DEBUG_UART.print(F(")dps Alt:"));
        DEBUG_UART.print(telemetry.altitude_cm / 100.0f, 2);
        DEBUG_UART.println(F("m"));
    }
}