#include "state_machine.h"

enum fsm_state {
    s_INIT_IDLE,
    s_NORMAL_MODE,
    s_RACE_MODE,
    s_END
};

full_state updateSM(full_state currState, bool newCanFrame, uint32_t receivedRPM,
                    uint8_t receivedGear, bool button, uint32_t clock) {
    full_state ret = currState;
    const bool buttonPressed = button && !currState.previousButton;
    ret.previousButton = button;

    const bool validFrame = newCanFrame && receivedRPM <= REDLINE_RPM &&
                            receivedGear >= 1 && receivedGear <= NUM_GEARS;

    fsm_state state;
    if (currState.ended) {
        state = s_END;
    } else if (!currState.activated) {
        state = s_INIT_IDLE;
    } else if (currState.mode == NORMAL) {
        state = s_NORMAL_MODE;
    } else {
        state = s_RACE_MODE;
    }

    switch (state) {
        case s_INIT_IDLE:
            if (validFrame && (receivedRPM != currState.rpm ||
                               receivedGear != currState.gear)) {
                ret.activated = true;
            }

            // Mode switching
            if (buttonPressed) {
                ret.mode = currState.mode == NORMAL ? RACE : NORMAL;
            }
            break;

        case s_NORMAL_MODE:
            if (buttonPressed) {
                ret.mode = RACE;
            }
            break;

        case s_RACE_MODE:
            if (buttonPressed) {
                ret.mode = NORMAL;
            }
            break;

        case s_END:
            // Since we're having END transition out when we receive
            // a valid frame, I just had it transition right to
            // race or normal mode since if we had it transition
            // back to idle, it would just transition out immediately again
            if (validFrame) {
                ret.ended = false;
                ret.activated = true;
            }

            if (buttonPressed) {
                ret.mode = currState.mode == NORMAL ? RACE : NORMAL;
            }
            break;
    }

    if (validFrame) {
        ret.rpm = receivedRPM;
        ret.gear = receivedGear;
        ret.savedClock = clock;
    } else if (uint32_t(clock - currState.savedClock) >= DATA_TIMEOUT_MS) {
        ret.ended = true;
    }

    return ret;
}
