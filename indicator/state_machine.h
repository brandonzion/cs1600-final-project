#pragma once
#include <stdint.h>
#include "config.h"

enum indicator_mode { NORMAL, RACE };

struct full_state {
    indicator_mode mode = NORMAL;
    uint32_t rpm = 0;
    uint8_t gear = 1;
    uint32_t savedClock = 0;
    bool activated = false;
    bool ended = false;
    bool previousButton = false;
};

constexpr uint32_t DATA_TIMEOUT_MS = 30000;

full_state updateSM(full_state current, bool newCanFrame, uint32_t receivedRPM,
                    uint8_t receivedGear, bool button, uint32_t clock);
