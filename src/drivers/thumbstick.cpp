#include "thumbstick.h"
#include "../config.h"

// Apply deadband filter: values within ±deadband are set to 0
int16_t apply_deadband(int16_t value, int16_t deadband) {
    if (abs(value) < deadband) {
        return 0;
    }
    return value;
}

Thumbstick::Thumbstick(uint8_t x_pin, uint8_t y_pin, uint8_t sw_pin, const char* name)
    : pin_x(x_pin)
    , pin_y(y_pin)
    , pin_sw(sw_pin)
    , stick_name(name)
    , x_raw(ADC_CENTER_VALUE)
    , y_raw(ADC_CENTER_VALUE)
    , x_value(0)
    , y_value(0)
    , sw_prev(HIGH)
    , clicked(false)
    , last_click_time(0)
{
}

void Thumbstick::begin() {
    // Configure pins - buttons use pull-up (active LOW when pressed)
    pinMode(pin_x, INPUT_ANALOG);
    pinMode(pin_y, INPUT_ANALOG);
    pinMode(pin_sw, INPUT_PULLUP);
    
    // Initial read
    update();
}

void Thumbstick::update() {
    // Read ADC (12-bit: 0-4095)
    x_raw = analogRead(pin_x);
    y_raw = analogRead(pin_y);
    
    // Center values: subtract center (2047) for range -2047 to +2047
    x_value = (int16_t)x_raw - ADC_CENTER_VALUE;
    y_value = (int16_t)y_raw - ADC_CENTER_VALUE;
    
    // Apply deadband to ignore small movements around center
    x_value = apply_deadband(x_value, ADC_DEADBAND);
    y_value = apply_deadband(y_value, ADC_DEADBAND);
    
    // Button debounce with cooldown
    bool sw_current = digitalRead(pin_sw);
    uint32_t now = millis();
    
    // Detect falling edge (button pressed: HIGH -> LOW)
    // Only register click if cooldown period has elapsed
    if (sw_current == LOW && sw_prev == HIGH) {
        if (now - last_click_time >= BUTTON_COOLDOWN_MS) {
            clicked = true;
            last_click_time = now;
        }
    }
    
    sw_prev = sw_current;
}