
#ifndef UART_CAN_H
#define UART_CAN_H

#include <Arduino.h>
#include "can_frame.h"

const uint8_t UART_START_MARKER = 0xAA;
const uint8_t UART_MAX_PACKET_SIZE = 16;

class UARTCAN {
private:

    Stream &serial;

    // receive parser state
    uint8_t rxBuffer[UART_MAX_PACKET_SIZE];
    uint8_t rxIndex = 0;
    uint8_t expectedLength = 0;

    // XOR checksum
    uint8_t calculateChecksum(
        const uint8_t *data,
        uint8_t length
    ) {
        uint8_t checksum = 0;

        for (uint8_t i = 0; i < length; i++) {
            checksum ^= data[i];
        }

        return checksum;
    }

public:

    // Constructor
    UARTCAN(Stream &port) : serial(port) {}

    // Serialize and transmit a CAN frame
    bool send(const CANFrame &frame) {

        if (frame.dlc > CAN_MAX_DATA_LENGTH ||
            frame.id > 0x1FFFFFFF) {
            return false;
        }

        if (!frame.extended && frame.id > 0x7FF) {
            return false;
        }

        uint8_t packet[UART_MAX_PACKET_SIZE];

        packet[0] = UART_START_MARKER;

        // CAN ID (4 bytes, big-endian)
        packet[1] = (frame.id >> 24) & 0xFF;
        packet[2] = (frame.id >> 16) & 0xFF;
        packet[3] = (frame.id >> 8) & 0xFF;
        packet[4] = frame.id & 0xFF;

        // Flags
        packet[5] = 0;

        if (frame.extended) {
            packet[5] |= 0x01;
        }

        if (frame.remoteRequest) {
            packet[5] |= 0x02;
        }

        // Data length
        packet[6] = frame.dlc;

        // Payload
        for (uint8_t i = 0; i < frame.dlc; i++) {
            packet[7 + i] = frame.data[i];
        }

        // Checksum over ID, flags, DLC, and data
        uint8_t checksumLength = 6 + frame.dlc;

        packet[7 + frame.dlc] =
            calculateChecksum(&packet[1], checksumLength);

        // Transmit complete packet
        uint8_t packetLength = 8 + frame.dlc;

        return serial.write(packet, packetLength) == packetLength;
    }

    // Receive and deserialize a CAN frame
    bool receive(CANFrame &frame) {

        while (serial.available()) {

            uint8_t incoming = serial.read();

            // Wait for start marker
            if (rxIndex == 0) {

                if (incoming == UART_START_MARKER) {
                    rxBuffer[rxIndex++] = incoming;
                }

                continue;
            }

            rxBuffer[rxIndex++] = incoming;

            // Once DLC is received, determine packet length
            if (rxIndex == 7) {

                uint8_t dlc = rxBuffer[6];

                if (dlc > CAN_MAX_DATA_LENGTH) {
                    rxIndex = 0;
                    expectedLength = 0;
                    continue;
                }

                expectedLength = 8 + dlc;
            }

            // Wait until complete packet is received
            if (expectedLength != 0 &&
                rxIndex == expectedLength) {

                uint8_t dlc = rxBuffer[6];

                uint8_t checksum =
                    calculateChecksum(&rxBuffer[1], 6 + dlc);

                uint8_t receivedChecksum =
                    rxBuffer[7 + dlc];

                bool valid = (checksum == receivedChecksum);

                if (valid) {

                    uint32_t id =
                        ((uint32_t)rxBuffer[1] << 24) |
                        ((uint32_t)rxBuffer[2] << 16) |
                        ((uint32_t)rxBuffer[3] << 8) |
                        rxBuffer[4];

                    uint8_t flags = rxBuffer[5];

                    // Validate identifier and flags
                    if ((flags & 0xFC) == 0 &&
                        id <= 0x1FFFFFFF &&
                        ((flags & 0x01) || id <= 0x7FF)) {

                        frame.id = id;
                        frame.extended = flags & 0x01;
                        frame.remoteRequest = flags & 0x02;
                        frame.dlc = dlc;

                        for (uint8_t i = 0; i < dlc; i++) {
                            frame.data[i] = rxBuffer[7 + i];
                        }

                        rxIndex = 0;
                        expectedLength = 0;

                        return true;
                    }
                }

                // Reset parser after packet completion
                rxIndex = 0;
                expectedLength = 0;
            }
        }

        return false;
    }
};

#endif
