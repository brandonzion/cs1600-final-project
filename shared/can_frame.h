#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <Arduino.h>
// #include <stdint.h>
#include "../simulator/types/types.h"

// max CAN payload size
const uint8_t CAN_MAX_PAYLOAD = 8;
// max engine state payload size
const uint8_t ENGINE_STATE_MAX_PAYLOAD = 3;
// every CAN packet has IDs that sort them into categories
const uint32_t CAN_ID_ENGINE_STATE = 0x100;


// logical CAN frame
// shared with both UART and custom CAN capstone 
struct CANFrame {
    uint32_t id; // 11-bit or 29-bit identifier
    bool extended; // boolean that defines if CANFrame uses extended identifier
    // bool remoteRequest; // RTR bit, dont think i need it but may need later
    uint8_t dlc; // 4-bit data length code from 0-8, representing how many (0-8) bytes of payload data
    uint8_t data[CAN_MAX_PAYLOAD]; // payload
};

// encode engine state to CAN frame
inline CANFrame encodeEngineState(const EngineState &engine) {
    CANFrame frame = {};
    frame.id = CAN_ID_ENGINE_STATE;
    frame.extended = false;
    frame.dlc = ENGINE_STATE_MAX_PAYLOAD;

    // RPM, big-endian 2 bytes
    frame.data[0] = (engine.rpm >> 8);
    frame.data[1] = engine.rpm & 0xFF;
    // gear, 1 byte
    frame.data[2] = engine.gear;
    return frame;
}

// decode CAN frame to engine state
inline bool decodeEngineState(const CANFrame &frame, EngineState &engine) {
    if (frame.id != CAN_ID_ENGINE_STATE || frame.dlc != ENGINE_STATE_MAX_PAYLOAD) {
        return false;
    }
    engine.rpm = ((uint16_t)frame.data[0] << 8) | frame.data[1];
    engine.gear = frame.data[2];
    return true;
}

#endif

