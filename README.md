# STM32F411CEU6 Wireless Controller with IMU

A handheld wireless controller using dual thumbsticks and NRF24L01 for remote control, with a receiver that includes LSM6DS3 accelerometer/gyroscope for motion sensing.

## System Overview

**SENDER (Controller)**:
- Reads input from two analog thumbsticks (4 axes + 2 buttons)
- Transmits data via NRF24L01+ at 50Hz
- Includes button debouncing and data checksums

**EMPFÄNGER (Receiver)**:
- Receives thumbstick data via NRF24L01+
- Reads LSM6DS3 IMU (accelerometer, gyroscope, temperature)
- Outputs all data via UART at 115200 baud

## Hardware

### Sender (Controller)
- **Main Controller**: STM32F411CEU6 "Black Pill" (100MHz, 512KB Flash, 128KB SRAM)
- **Wireless Module**: NRF24L01+ 2.4GHz Transceiver
- **Input Devices**: 2× KY-023 Thumbstick Joystick Modules
- **Debug Interface**: FT232RL USB-UART Adapter (optional)
- **Programming**: ST-Link V2 Programmer

### Empfänger (Receiver)
- **Main Controller**: STM32F411CEU6 "Black Pill"
- **Wireless Module**: NRF24L01+ 2.4GHz Transceiver
- **IMU Sensor**: LSM6DS3 Accelerometer/Gyroscope (I2C)
- **Debug Interface**: FT232RL USB-UART Adapter
- **Programming**: ST-Link V2 Programmer

## Wiring

### Sender - NRF24L01+ to STM32F411CEU6
- **VCC** → 3.3V (⚠️ **NOT 5V**)
- **GND** → GND
- **MOSI** → PA7 (SPI1_MOSI)
- **MISO** → PA6 (SPI1_MISO)
- **SCK** → PA5 (SPI1_SCK)
- **CE** → PA8
- **CSN** → PB12

### Sender - Left Thumbstick (KY-023)
- **GND** → GND
- **+5V** → 3.3V
- **VRX (X-axis)** → PA0 (ADC1_IN0)
- **VRY (Y-axis)** → PA1 (ADC1_IN1)
- **SW (Button)** → PA4 (Digital input with internal pull-up)

### Sender - Right Thumbstick (KY-023)
- **GND** → GND
- **+5V** → 3.3V
- **VRX (X-axis)** → PA2 (ADC1_IN2)
- **VRY (Y-axis)** → PA3 (ADC1_IN3)
- **SW (Button)** → PB3 (Digital input with internal pull-up)

### Empfänger - NRF24L01+ to STM32F411CEU6
- Identical to Sender wiring

### Empfänger - LSM6DS3 IMU (I2C)
- **VCC** → 3.3V (⚠️ **NOT 5V**)
- **GND** → GND
- **SDA** → PB7 (I2C1_SDA)
- **SCL** → PB6 (I2C1_SCL)
- **Optional**: 100nF decoupling capacitor between VCC and GND

### FT232RL UART Debug (Sender or Empfänger)
- **TX (FT232RL)** → PA3 (USART2_RX on STM32)
- **RX (FT232RL)** → PA2 (USART2_TX on STM32)
- **GND** → GND

### Onboard LED (Both Boards)
- **PC13** → Built-in LED (active LOW)

## How to Use

### 1. Flash the Sender
```
pio run -e sender -t upload
```

### 2. Flash the Empfänger
Use the code from `src/receiver_stub.h` in a separate PlatformIO project or copy it to `src/main.cpp`.

### 3. Monitor Output
Open a serial monitor at 115200 baud. The receiver outputs:
- `[T]ms] THUMB: L(x,y) R(x,y) [BUTTONS] | IMU: A(x,y,z)g G(x,y,z)dps T:temp°C`

## UART Output Format

The Empfänger outputs all data in a single line:

```
[12345ms] L(2048,2047) R(1980,2090) [L-CLICK] | IMU: A(0.02,-0.01,0.98)g G(0.1,0.2,-0.1)dps T:26.5°C
```

- `L(left_x,left_y)`: Left thumbstick position (0-4095)
- `R(right_x,right_y)`: Right thumbstick position
- `[L-CLICK]`/`[R-CLICK]`: Button press events
- `IMU: A(ax,ay,az)g`: Acceleration in g (gravitational units)
- `G(gx,gy,gz)dps`: Angular velocity in degrees per second
- `T:temp°C`: Temperature in Celsius

## Data Transmission

- **Rate**: 50Hz (every 20ms)
- **Channel**: 76 (2.476GHz)
- **Address**: `CTR01`
- **Checksum**: XOR checksum for data integrity

## Files

### Sender
- **`src/main.cpp`**: Main application with setup/loop and data handling
- **`src/config.h`**: Pin definitions, radio settings, and timing constants
- **`src/drivers/nrf24_driver.h/cpp`**: NRF24L01+ driver with checksum calculation
- **`src/drivers/thumbstick.h/cpp`**: KY-023 thumbstick driver with deadband filtering

### Empfänger
- **`src/receiver_stub.h`**: Complete receiver firmware with LSM6DS3 integration
- **`src/drivers/lsm6ds3_driver.h/cpp`**: LSM6DS3 IMU driver (optional, for use with separate driver)

### Documentation
- **`platformio.ini`**: PlatformIO project configuration
- **`docs/steps.json`**: Step-by-step assembly instructions (German)
- **`specs/bom.json`**: Bill of materials with component specifications