#pragma once

#include <Arduino.h>
#include "../config.h"

// Deadband filter for joystick center zone
int16_t apply_deadband(int16_t value, int16_t deadband);

class Thumbstick {
public:
    Thumbstick(uint8_t x_pin, uint8_t y_pin, uint8_t sw_pin, const char* name);
    
    void begin();
    void update();
    
    // Axes: 0-4095 (12-bit ADC), returns centered value around 0
    int16_t getX() const { return x_value; }
    int16_t getY() const { return y_value; }
    
    // Button: true if clicked (edge detected, not current state)
    bool wasClicked() const { return clicked; }
    void clearClickFlag() { clicked = false; }
    
    // Raw ADC values for debugging
    uint16_t getRawX() const { return x_raw; }
    uint16_t getRawY() const { return y_raw; }
    
private:
    uint8_t pin_x;
    uint8_t pin_y;
    uint8_t pin_sw;
    const char* stick_name;
    
    uint16_t x_raw;
    uint16_t y_raw;
    int16_t x_value;  // Centered -2047 to +2047
    int16_t y_value;
    
    bool sw_prev;
    bool clicked;
    uint32_t last_click_time;
};