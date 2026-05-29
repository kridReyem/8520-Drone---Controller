/**
 * BMP-280 Barometric Pressure Sensor Driver
 * 
 * Hardware: BMP-280 barometric pressure sensor module
 * Interface: I2C (shared bus with LSM6DS3)
 * I2C Address: 0x76 (default) or 0x77 (SDO pin HIGH)
 * Voltage: 3.3V
 * 
 * Features:
 *   - Barometric pressure (300-1100 hPa, ±1 hPa accuracy)
 *   - Temperature (-40°C to +85°C, ±1°C accuracy)
 *   - Altitude calculation (requires sea-level pressure reference)
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

class BMP280Driver {
public:
    /**
     * Constructor
     * @param address I2C address (0x76 or 0x77)
     */
    BMP280Driver(uint8_t address = 0x76);
    
    /**
     * Initialize the BMP-280 sensor
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * Read all sensor data
     * @param pressure Reference to store pressure in Pa
     * @param temperature Reference to store temperature in °C
     * @param altitude Reference to store altitude in meters
     */
    void readData(float& pressure, float& temperature, float& altitude);
    
    /**
     * Get pressure in hectopascals (hPa)
     * @return Pressure in hPa
     */
    float getPressureHPa();

    // Setzt aktuellen Druck als Boden = 0m   
    void calibrateAltitude();  
    
    /**
     * Get temperature in Celsius
     * @return Temperature in °C
     */
    float getTemperature();
    
    /**
     * Get altitude in meters
     * Uses standard sea-level pressure (1013.25 hPa) by default
     * @param seaLevelPressure Sea-level pressure in hPa (default: 1013.25)
     * @return Altitude in meters
     */
   // Alt: beide Signaturen deklarieren
    float getAltitude();                          // ← NEU: nutzt _sea_level_pressure
    float getAltitude(float seaLevelPressure);    // bleibt für manuellen Aufruf
    
    /**
     * Set custom sea-level pressure for altitude calculation
     * Call this with local sea-level pressure for accurate altitude
     * @param pressure Sea-level pressure in hPa
     */
    void setSeaLevelPressure(float pressure);
    
    /**
     * Check if sensor is initialized
     * @return true if initialized
     */
    bool isInitialized();
    
private:
    Adafruit_BMP280 _bmp;
    uint8_t _address;
    bool _initialized;
    float _sea_level_pressure;  // hPa
    void configureSampling(); 
};