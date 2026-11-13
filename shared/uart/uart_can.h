#ifndef UART_CAN_H
#define UART_CAN_H

#include <Arduino.h>
#include "can_frame.h"

const uint8_t UART_START_BYTE = 0xAA; // tells us new packet started
// start byte + CAN ID (4 bytes) + flags (1 byte) + DLC (1 byte) + data (8 bytes) + checksum (1 byte)
const uint8_t UART_MAX_PACKET_SIZE = 16; 

// goal of this class is to implement a UART to CAN bridge that can send and receive CAN frames over a serial connection. 
// handles serialization and deserialization of CAN frames + calculation of checksums for data integrity.
// UART = transport layer, CAN = protocol layer. 

// NEED TO TEST: DLC = 0, DLC = 8, invalid standard CAN IDs, invalid extended CAN IDs, and corrupted checksums
class UARTCAN {

private:
    Stream &serial; // byte flow
    // receive parser state
    uint8_t buf[UART_MAX_PACKET_SIZE]; // buffer holds incoming bytes til full packet received
    uint8_t index = 0;
    uint8_t expectedLength = 0;

    // XOR checksum
    uint8_t calculateChecksum(const uint8_t *data, uint8_t length) {
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < length; i++) {
            checksum ^= data[i];
        }
        return checksum;
    }
    
    // reset the receiver state
    void resetReceiver() {
        index = 0;
        expectedLength = 0;
    }

// includes send, recieve, and constructor
public:
    UARTCAN(Stream &port) : serial(port) {}

    // send and serialize
    bool send(const CANFrame &frame) {
        if (frame.dlc > CAN_MAX_DATA_LENGTH || frame.id > 0x1FFFFFFF // checks if packet is too big or if extended frame has too big ID
            || (!frame.extended && frame.id > 0x7FF)) { // checks if standard frame has too big ID 
            return false;
        }

        uint8_t packet[UART_MAX_PACKET_SIZE];
        
        // start byte + CAN ID (4 bytes) + flags (1 byte) + DLC (1 byte) + data (8 bytes) + checksum (1 byte)

        packet[0] = UART_START_BYTE;
        packet[1] = (frame.id >> 24) & 0xFF;
        packet[2] = (frame.id >> 16) & 0xFF;
        packet[3] = (frame.id >> 8) & 0xFF;
        packet[4] = frame.id & 0xFF;
        packet[5] = 0;
        if (frame.extended) {
            packet[5] = 0x01;
        } 
        else {
            packet[5] = 0x00;
        }
        packet[6] = frame.dlc;
        for (uint8_t i = 0; i < frame.dlc; i++) {
            packet[7 + i] = frame.data[i];
        }
        packet[7 + frame.dlc] =
            calculateChecksum(&packet[1], 6 + frame.dlc);

        // boom transmit packet
        return serial.write(packet, 8 + frame.dlc) == 8 + frame.dlc;
    }

    // receive and deserialize
    bool receive(CANFrame &frame) {
        while (serial.available()) {
            uint8_t incoming = serial.read();
            // wait for start byte
            if (index == 0) {
                if (incoming == UART_START_BYTE) {
                    buf[index++] = incoming;
                }
                continue;
            }
            if (index >= UART_MAX_PACKET_SIZE) {
                resetReceiver();
                continue;
            }

            buf[index++] = incoming;
            // DLC at byte 7, index 6 @ buffer, wait til index is past 6 to ensure its fully received 
            // determines how big payload is, so use this to calc expected length of packet
            if (index == 7) {
                uint8_t dlc = buf[6];
                if (dlc > CAN_MAX_DATA_LENGTH) {
                    resetReceiver();
                    continue;
                }
                expectedLength = 8 + dlc;
            }
            if (expectedLength != 0 && index == expectedLength) {
                uint8_t dlc = buf[6];
                uint8_t checksum = calculateChecksum(&buf[1], 6 + dlc);
                if (checksum != buf[7 + dlc]) {
                    resetReceiver();
                    continue;
                }
                // reconstruct CAN id by shifting and combining the bytes from the buffer
                uint32_t id =
                    ((uint32_t)buf[1] << 24) |
                    ((uint32_t)buf[2] << 16) |
                    ((uint32_t)buf[3] << 8) |
                    buf[4];
                uint8_t flags = buf[5];

                if (flags > 1 || id > 0x1FFFFFFF || ((flags & 0x01) == 0 && id > 0x7FF)) {
                    resetReceiver();
                    continue;
                }
                frame.id = id;
                frame.extended = flags & 0x01;
                frame.dlc = dlc;

                for (uint8_t i = 0; i < dlc; i++) {
                    frame.data[i] = buf[7 + i];
                }
                resetReceiver();
                return true;
            }
        }
        return false;
    }
};

#endif
