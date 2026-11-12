#include "types.h"

// Converts pot value to throttle value
float potToThrottle(int raw, int adcMax);

// Advance the engine one time step
float updateRpm(float rpm, float throttle, uint8_t gear, float dtSec);

// RPM after a shift
// newRpm = rpm * ratio[to] / ratio[from], clamped to [0, REDLINE].
float rpmAfterShift(float rpm, uint8_t fromGear, uint8_t toGear);

uint8_t applyShift(uint8_t gear, GearShift s);

// Turns raw button levels into at most one shift per press.
class ShiftInput {
public:
  GearShift update(bool upPressed, bool downPressed, unsigned long now);

private:
  bool          stableUp_      = false;  // debounced levels
  bool          stableDown_    = false;
  bool          rawUp_         = false;  // last raw levels seen
  bool          rawDown_       = false;
  unsigned long lastRawChange_ = 0;      // when either raw level last changed
};
