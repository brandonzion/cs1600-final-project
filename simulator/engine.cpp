#include "engine.h"
#include "config.h"
#include "types.h"

static float clampRpm(float rpm) {
  if (rpm < 0.0f) return 0.0f;
  if (rpm > (float)REDLINE_RPM) {
     return (float)REDLINE_RPM;
  }
  return rpm;
}

static float ratioOf(uint8_t gear) {
  if (gear < 1) gear = 1;
  if (gear > NUM_GEARS) gear = NUM_GEARS;
  return GEAR_RATIOS[gear - 1];
}

float potToThrottle(int raw, int adcMax) {
  if (raw < 0) raw = 0;
  if (raw > adcMax) raw = adcMax;
  float t = (float)raw / (float)adcMax;
  return t < THROTTLE_DEADZONE ? 0.0f : t;
}

// How hard a gear pulls compared to 1st: lower gears multiply engine torque more, so RPM climbs faster.
static float gearPull(uint8_t gear) {
  return ratioOf(gear) / ratioOf(1);
}

float updateRpm(float rpm, float throttle, uint8_t gear, float dtSec) {
  float climbRate = MAX_ACCEL_RPM_PER_S * gearPull(gear);  // RPM/s at full throttle
  float dropRate  = ENGINE_BRAKE_RPM_PER_S;                // RPM/s with foot off

  float gained = throttle          * climbRate * dtSec;
  float lost   = (1.0f - throttle) * dropRate  * dtSec;

  return clampRpm(rpm + gained - lost);
}

float rpmAfterShift(float rpm, uint8_t fromGear, uint8_t toGear) {
  return clampRpm(rpm * ratioOf(toGear) / ratioOf(fromGear));
}

uint8_t applyShift(uint8_t gear, GearShift s) {
  if (s == GearShift::UPSHIFT && gear < NUM_GEARS) return gear + 1;
  if (s == GearShift::DOWNSHIFT && gear > 1)       return gear - 1;
  return gear;  // NONE, or shifting past a limit
}

GearShift ShiftInput::update(bool upPressed, bool downPressed, unsigned long now) {
  if (upPressed != rawUp_ || downPressed != rawDown_) {
    rawUp_         = upPressed;
    rawDown_       = downPressed;
    lastRawChange_ = now;
    return GearShift::NONE;
  }
  if (now - lastRawChange_ < DEBOUNCE_MS) return GearShift::NONE;  // still settling
  if (upPressed == stableUp_ && downPressed == stableDown_) return GearShift::NONE;

  bool wasIdle = !stableUp_ && !stableDown_;
  stableUp_    = upPressed;
  stableDown_  = downPressed;
 
  if (!wasIdle) return GearShift::NONE;
  if (upPressed && !downPressed) return GearShift::UPSHIFT;
  if (downPressed && !upPressed) return GearShift::DOWNSHIFT;
  return GearShift::NONE;
}
