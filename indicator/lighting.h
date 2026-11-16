#pragma once
#include "state_machine.h"

constexpr uint16_t RPM_PER_LED = 100;
constexpr uint16_t LED_COUNT = REDLINE_RPM / RPM_PER_LED;
constexpr uint32_t FLASH_HALF_PERIOD_MS = 250; // 0.5 s period

struct LedColor { uint8_t r, g, b; };

uint16_t optimalShiftRpm(uint8_t gear);
bool shouldUpshift(uint32_t rpm, uint8_t gear);

void determineLedColors(const full_state& state, uint32_t clock,
                        LedColor (&colors)[LED_COUNT]);
