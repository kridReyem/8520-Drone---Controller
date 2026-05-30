#include "nrf24_driver.h"
#include "../config.h"

NRF24Driver::NRF24Driver(uint8_t ce_pin, uint8_t csn_pin)
    : _radio(ce_pin, csn_pin)
    , _initialized(false)
    , _last_receive_ms(0)
{
}

// ─── gemeinsame Basis-Konfiguration ────────────────────────────────────────
static bool configureRadio(RF24& radio) {
    radio.setChannel(NRF24_CHANNEL);
    radio.setAutoAck(true);
    radio.enableDynamicPayloads();
    radio.enableAckPayload();
    radio.setPALevel(NRF24_PA_LEVEL);
    radio.setDataRate(NRF24_DATA_RATE);
    radio.setCRCLength(RF24_CRC_16);
    radio.setAddressWidth(5);

    // Auto-Retransmit: 3 Versuche, 500µs Wartezeit zwischen Versuchen
    // Gesamt-Timeout bei Fehler: 3 × 500µs = 1.5ms << 20ms Sendeintervall
    radio.setRetries(2, 3);  // (delay_250us_steps, count)
    //                  ^  ^
    //                  |  └─ 3 Wiederholungen
    //                  └──── 2 × 250µs = 500µs Wartezeit

    return true;
}

static bool initSPI(RF24& radio) {
    SPI.setMOSI(NRF24_SPI_MOSI);
    SPI.setMISO(NRF24_SPI_MISO);
    SPI.setSCLK(NRF24_SPI_SCK);
    SPI.begin();

    if (!radio.begin(&SPI)) {
        DEBUG_UART.println(F("[NRF24] ERROR: Hardware not responding!"));
        return false;
    }
    if (!radio.isChipConnected()) {
        DEBUG_UART.println(F("[NRF24] ERROR: Chip not detected!"));
        return false;
    }
    return true;
}

// ─── TX-Seite (Controller) ──────────────────────────────────────────────────
bool NRF24Driver::beginTX() {
    DEBUG_UART.println(F("[NRF24] Initializing TX (Controller)..."));

    if (!initSPI(_radio)) return false;
    configureRadio(_radio);

    _radio.openWritingPipe(CONTROLLER_ADDRESS);
    _radio.openReadingPipe(1, RECEIVER_ADDRESS);
    _radio.stopListening();   // TX-Modus

    DEBUG_UART.print(F("[NRF24] TX ready. Channel: "));
    DEBUG_UART.println(NRF24_CHANNEL);

    _initialized = true;
    return true;
}

// ─── RX-Seite (Drohne) ──────────────────────────────────────────────────────
bool NRF24Driver::beginRX() {
    DEBUG_UART.println(F("[NRF24] Initializing RX (Drone)..."));

    if (!initSPI(_radio)) return false;
    configureRadio(_radio);

    _radio.openReadingPipe(1, CONTROLLER_ADDRESS);
    _radio.startListening();  // RX-Modus

    DEBUG_UART.print(F("[NRF24] RX ready. Channel: "));
    DEBUG_UART.println(NRF24_CHANNEL);

    _initialized = true;
    return true;
}

// ─── TX: Senden + ACK-Payload lesen ─────────────────────────────────────────
bool NRF24Driver::sendMessage(const ControllerData& data,
                               TelemetryData* telemetry_out) {
    if (!_initialized) return false;

    ControllerData tx_data = data;
    tx_data.checksum = calculateChecksum(tx_data);

    bool success = _radio.write(&tx_data, sizeof(ControllerData));
    if (!success) {
        DEBUG_UART.println(F("[NRF24] TX failed!"));
        return false;
    }

    // ACK-Payload auslesen falls Puffer übergeben
    if (telemetry_out != nullptr && _radio.isAckPayloadAvailable()) {
        _radio.read(telemetry_out, sizeof(TelemetryData));

        // Checksum prüfen
        uint8_t cs = 0;
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(telemetry_out);
        for (size_t i = 0; i < offsetof(TelemetryData, checksum); i++) {
            cs ^= ptr[i];
        }
        if (cs != telemetry_out->checksum) {
            DEBUG_UART.println(F("[NRF24] ACK checksum failed!"));
            // TX war erfolgreich, nur ACK verworfen – kein false zurück
        }
    }

    return true;
}

// ─── RX: Paket verfügbar? ────────────────────────────────────────────────────
bool NRF24Driver::available() {
    if (!_initialized) return false;
    return _radio.available();
}

// ─── RX: Paket lesen + Checksum prüfen ──────────────────────────────────────
bool NRF24Driver::receiveMessage(ControllerData& data_out) {
    if (!_initialized) return false;

    _radio.read(&data_out, sizeof(ControllerData));

    // Checksum prüfen
    uint8_t cs = 0;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&data_out);
    for (size_t i = 0; i < offsetof(ControllerData, checksum); i++) {
        cs ^= ptr[i];
    }
    if (cs != data_out.checksum) {
        DEBUG_UART.println(F("[NRF24] RX checksum failed!"));
        return false;
    }

    _last_receive_ms = millis();   // Failsafe-Timestamp aktualisieren
    return true;
}

bool NRF24Driver::waitForDrone(uint32_t timeout_ms) {
    if (!_initialized) return false;

    DEBUG_UART.print(F("[NRF24] Waiting for drone"));

    ControllerData ping = {};   // leeres Paket als PING
    TelemetryData  response;
    uint32_t start = millis();

    while (true) {
        // Timeout prüfen
        if (timeout_ms > 0 && (millis() - start) > timeout_ms) {
            DEBUG_UART.println(F(" TIMEOUT!"));
            return false;
        }

        // PING senden
        if (_radio.write(&ping, sizeof(ControllerData))) {
            // ACK empfangen – Status prüfen
            if (_radio.isAckPayloadAvailable()) {
                _radio.read(&response, sizeof(TelemetryData));
                if (response.status == 1) {
                    DEBUG_UART.println(F(" OK"));
                    DEBUG_UART.print(F("[NRF24] Drone ready after "));
                    DEBUG_UART.print(millis() - start);
                    DEBUG_UART.println(F("ms"));
                    return true;
                }
            }
        }

        DEBUG_UART.print(F("."));   // Fortschrittsanzeige
        delay(200);                 // 5Hz PING-Rate
    }
}

// ─── RX: ACK-Payload für nächstes eingehendes Paket setzen ──────────────────
void NRF24Driver::setAckPayload(const TelemetryData& telemetry) {
    if (!_initialized) return;
    // Pipe 1 = die Pipe auf der wir hören
    _radio.writeAckPayload(1, &telemetry, sizeof(TelemetryData));
}

// ─── Power Management ────────────────────────────────────────────────────────
void NRF24Driver::powerDown() {
    if (_initialized) {
        _radio.powerDown();
        DEBUG_UART.println(F("[NRF24] Power down"));
    }
}

void NRF24Driver::powerUp() {
    if (_initialized) {
        _radio.powerUp();
        DEBUG_UART.println(F("[NRF24] Power up"));
    }
}

void NRF24Driver::printDetails() {
    if (_initialized) _radio.printDetails();
}

// ─── Checksum ────────────────────────────────────────────────────────────────
uint8_t NRF24Driver::calculateChecksum(const ControllerData& data) {
    uint8_t checksum = 0;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&data);
    for (size_t i = 0; i < offsetof(ControllerData, checksum); i++) {
        checksum ^= ptr[i];
    }
    return checksum;
}