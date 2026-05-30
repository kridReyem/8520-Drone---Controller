#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "shared_types.h"   // ControllerData + TelemetryData
// config.h wird NICHT hier inkludiert – kommt über die .cpp

class NRF24Driver {
public:
    NRF24Driver(uint8_t ce_pin, uint8_t csn_pin);

        /**
     * [TX] Wartet bis Drohne READY meldet
     * Sendet periodisch PING-Pakete und wertet ACK-Status aus
     * @param timeout_ms  Maximale Wartezeit in ms (0 = unbegrenzt)
     * @return true wenn Drohne bereit
     */
    bool waitForDrone(uint32_t timeout_ms = 0);

    /**
     * Initialisiert NRF24 als Sender (Controller-Seite)
     * Öffnet WritingPipe auf CONTROLLER_ADDRESS
     */
    bool beginTX();

    /**
     * Initialisiert NRF24 als Empfänger (Drohnen-Seite)
     * Öffnet ReadingPipe 1 auf CONTROLLER_ADDRESS, startet Listening
     */
    bool beginRX();

    /**
     * [TX] Sendet ControllerData, liest optional ACK-Telemetrie zurück
     * @param data         Thumbstick + Button Daten
     * @param telemetry_out Zeiger auf TelemetryData-Puffer, oder nullptr
     * @return true wenn Übertragung erfolgreich
     */
    bool sendMessage(const ControllerData& data,
                     TelemetryData* telemetry_out = nullptr);

    /**
     * [RX] Prüft ob ein Paket verfügbar ist
     * @return true wenn Daten bereit zum Lesen
     */
    bool available();

    /**
     * [RX] Liest empfangenes ControllerData-Paket
     * Prüft Checksum – gibt false zurück wenn Checksum fehlschlägt
     * @param data_out Ziel-Struct
     * @return true wenn Paket valide
     */
    bool receiveMessage(ControllerData& data_out);

    /**
     * [RX] Registriert Telemetrie-Daten als nächsten ACK-Payload
     * Muss nach jedem IMU-Read aufgerufen werden
     * @param telemetry Aktuelle Sensor-Daten der Drohne
     */
    void setAckPayload(const TelemetryData& telemetry);

    /**
     * Zeitstempel des letzten erfolgreich empfangenen Pakets
     * Für Failsafe-Logik: wenn (millis() - lastReceiveTime()) > 500 → Motoren aus
     */
    uint32_t lastReceiveTime() const { return _last_receive_ms; }

    bool isReady()    const { return _initialized; }
    void powerDown();
    void powerUp();
    void printDetails();

private:
    RF24     _radio;
    bool     _initialized;
    uint32_t _last_receive_ms;

    uint8_t calculateChecksum(const ControllerData& data);
};