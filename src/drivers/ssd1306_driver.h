
/**
 * Minimal SSD1306 128x64 OLED Driver
 *
 * Uses raw Wire (I2C) calls — no Adafruit GFX, no U8g2, no cstdlib dependency.
 * Compatible with STM32 Arduino (arm-none-eabi newlib-nano) without namespace issues.
 *
 * Font: 5x7 pixels per character, 1 px gap = 6 px column advance.
 * Screen: 128 px wide = 21 chars per line (last col partially visible).
 * Pages: 8 pages x 8 px = 64 px height = 8 lines of text.
 *
 * Usage:
 *   SSD1306Driver oled;
 *   oled.init(0x3C);       // I2C address (Wire must already be started)
 *   oled.clear();
 *   oled.setCursor(0, 0);  // col (0..127), page (0..7)
 *   oled.print("Hello");
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

class SSD1306Driver {
public:
    SSD1306Driver() : _addr(0x3C), _col(0), _page(0) {}

    /** Initialise display. Call Wire.begin() + setSDA/SCL before this. */
    void init(uint8_t i2c_addr = 0x3C);

    /** Fill entire display with zeros */
    void clear();

    /**
     * Set text cursor position.
     * @param col  Pixel column (0-127). Snapped to nearest char boundary internally.
     * @param page Display page (0-7). Each page = 8 pixel rows.
     */
    void setCursor(uint8_t col, uint8_t page);

    /** Print a null-terminated C-string at current cursor */
    void print(const char* str);

    /** Print a flash-string at current cursor */
    void print(const __FlashStringHelper* str);

    /** Print an unsigned long at current cursor */
    void print(unsigned long value);

    /** Print an unsigned int at current cursor */
    void print(unsigned int value);

private:
    uint8_t _addr;
    uint8_t _col;
    uint8_t _page;

    void sendCmd(uint8_t cmd);
    void sendCmdPair(uint8_t cmd, uint8_t arg);
    void writeChar(char c);

    // 5x7 ASCII font, chars 0x20-0x7E
    static const uint8_t FONT[][5];
};