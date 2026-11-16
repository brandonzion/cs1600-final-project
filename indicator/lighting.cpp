#include "lighting.h"

uint16_t optimalShiftRpm(uint8_t gear) {
    if (gear < 1 || gear >= NUM_GEARS) return 0;
    const float required = 2300.0f * GEAR_RATIOS[gear - 1] / GEAR_RATIOS[gear];
    uint32_t threshold = uint32_t(required + 0.999f);
    if (threshold > REDLINE_RPM) threshold = REDLINE_RPM;
    return uint16_t(threshold);
}

bool shouldUpshift(uint32_t rpm, uint8_t gear) {
    const uint16_t threshold = optimalShiftRpm(gear);
    return threshold != 0 && rpm >= threshold;
}

// blue -> cyan -> green -> yellow -> red.
static LedColor ledColor(uint16_t index) {
    const uint16_t position = uint32_t(index) * 1020 / (LED_COUNT - 1);
    if (position < 255) return {0, uint8_t(position), 255};
    if (position < 510) return {0, 255, uint8_t(510 - position)};
    if (position < 765) return {uint8_t(position - 510), 255, 0};
    return {255, uint8_t(1020 - position), 0};
}

void determineLedColors(const full_state& state, uint32_t clock,
                        LedColor (&colors)[LED_COUNT]) {
    for (auto& color : colors) color = {0, 0, 0};
    if (state.ended || !state.activated) return;

    const bool flash = state.rpm >= REDLINE_RPM;
    if (flash) {
        if ((clock / FLASH_HALF_PERIOD_MS) % 2 == 0) {
            for (auto& color : colors) color = {255, 0, 0};
        }
        return;
    }

    uint32_t lit = state.rpm / RPM_PER_LED;
    if (lit > LED_COUNT) lit = LED_COUNT;
    for (uint16_t i = 0; i < lit; ++i) colors[i] = ledColor(i);

    const uint16_t threshold = optimalShiftRpm(state.gear);
    if (state.mode == NORMAL && threshold != 0) {
        const uint16_t marker = (uint32_t(threshold) * LED_COUNT - 1) / REDLINE_RPM;
        colors[marker] = {0, 255, 0};
    }
}
