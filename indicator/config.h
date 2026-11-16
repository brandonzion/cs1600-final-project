#pragma once
#include <stdint.h>

// Keep consistent with our sim's configs
constexpr uint16_t REDLINE_RPM = 7000;
constexpr uint8_t NUM_GEARS = 6;
constexpr float GEAR_RATIOS[NUM_GEARS] = {3.50f, 2.10f, 1.40f, 1.00f, 0.80f, 0.65f};
