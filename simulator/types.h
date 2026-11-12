#pragma once
#include <stdint.h>

enum class GearShift : uint8_t { NONE, UPSHIFT, DOWNSHIFT };

struct EngineState {
  uint16_t rpm;
  uint8_t  gear;
};
