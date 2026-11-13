#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <Arduino.h>
#include <stdint.h>

// max CAN payload size
const uint8_t CAN_MAX_PAYLOAD = 8;
// max vehicle data payload size
const uint8_t VEHICLE_DATA_MAX_PAYLOAD = 3;
// every CAN packet has IDs that sort them into categories
const uint32_t CAN_ID_VEHICLE_DATA = 0x100;


// logical CAN frame
// shared with both UART and custom CAN capstone 
struct CANFrame {
    uint32_t id; // 11-bit or 29-bit identifier
    bool extended; // boolean that defines if CANFrame uses extended identifier
    // bool remoteRequest; // RTR bit, dont think i need it 
    uint8_t dlc; // 4-bit data length code from 0-8, representing 0-8 bytes of payload data
    uint8_t data[CAN_MAX_PAYLOAD]; // payload
};

// vehicle information to be ingested by indicator
struct VehicleData {
    uint16_t rpm;
    uint8_t gear;
};

// encode vehicle data to CAN frame
inline CANFrame encodeVehicleData(const VehicleData &vehicle) {
    CANFrame frame = {};
    frame.id = CAN_ID_VEHICLE_DATA;
    frame.extended = false;
    frame.dlc = VEHICLE_DATA_MAX_PAYLOAD;

    // RPM, big-endian 2 bytes
    frame.data[0] = (vehicle.rpm >> 8) & 0xFF;
    frame.data[1] = vehicle.rpm & 0xFF;
    // gear, 1 byte
    frame.data[2] = vehicle.gear;
    return frame;
}

// decode CAN frame to vehicle data
inline bool decodeVehicleData(const CANFrame &frame, VehicleData &vehicle) {
    if (frame.id != CAN_MAX_PAYLOAD || frame.dlc != VEHICLE_DATA_MAX_PAYLOAD) {
        return false;
    }
    vehicle.rpm = ((uint16_t)frame.data[0] << 8) | frame.data[1];
    vehicle.gear = frame.data[2];
    return true;
}

#endif

