#include "bmp280_driver.h"

BMP280Driver::BMP280Driver(uint8_t address)
    : _address(address)
    , _initialized(false)
    , _sea_level_pressure(1013.25f)
{
}

// ← zentrale Sampling-Konfiguration, einmal definiert
void BMP280Driver::configureSampling() {
    _bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,
        Adafruit_BMP280::SAMPLING_X2,    // Temperatur
        Adafruit_BMP280::SAMPLING_X16,   // Druck: 16x mitteln
        Adafruit_BMP280::FILTER_X16,     // IIR-Filter
        Adafruit_BMP280::STANDBY_MS_63   // 63ms Messintervall
    );
}

bool BMP280Driver::init() {
    if (_bmp.begin(_address)) {
        _initialized = true;
        configureSampling();    // ← beide Pfade gleich
        return true;
    }

    // Adresse automatisch tauschen falls erster Versuch fehlschlägt
    uint8_t alt_address = (_address == 0x76) ? 0x77 : 0x76;
    if (_bmp.begin(alt_address)) {
        _address = alt_address;
        _initialized = true;
        configureSampling();    // ← beide Pfade gleich
        return true;
    }

    _initialized = false;
    return false;
}

void BMP280Driver::calibrateAltitude() {
    if (!_initialized) return;
    _sea_level_pressure = _bmp.readPressure() / 100.0f;
    Serial.print(F("[BMP280] Altitude calibrated. P0 = "));
    Serial.print(_sea_level_pressure, 2);
    Serial.println(F(" hPa (ground = 0m)"));
}

float BMP280Driver::getPressureHPa() {
    if (!_initialized) return 0;
    return _bmp.readPressure() / 100.0f;
}

float BMP280Driver::getTemperature() {
    if (!_initialized) return 0;
    return _bmp.readTemperature();
}

float BMP280Driver::getAltitude() {
    if (!_initialized) return 0;
    return _bmp.readAltitude(_sea_level_pressure);
}

float BMP280Driver::getAltitude(float seaLevelPressure) {
    if (!_initialized) return 0;
    return _bmp.readAltitude(seaLevelPressure);
}

void BMP280Driver::readData(float& pressure, float& temperature, float& altitude) {
    if (!_initialized) {
        pressure = temperature = altitude = 0;
        return;
    }
    pressure    = _bmp.readPressure() / 100.0f;
    temperature = _bmp.readTemperature();
    altitude    = _bmp.readAltitude(_sea_level_pressure);
}

void BMP280Driver::setSeaLevelPressure(float pressure) {
    _sea_level_pressure = pressure;
}

bool BMP280Driver::isInitialized() {
    return _initialized;
}