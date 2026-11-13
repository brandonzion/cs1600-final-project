#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include "can_frame.h"

// shared functions in UART CAN and Capstone CAN 
class CANInterface {
public:
    virtual bool send(const CANFrame &frame) = 0;
    virtual bool receive(CANFrame &frame) = 0;
    virtual void update() = 0;
    virtual ~CANInterface() {} 
    // virtual destructor, so deleting any derived CAN driver classes goes through a CANInterface pointer
    virtual ~CANInterface() {} 
};
#endif
