// Host-side unit tests for the simulator's logic (no Arduino needed).
// From the repo root:
//   g++ -std=c++11 -Wall simulator/engine.cpp simulator/test/test_engine.cpp \
//       -o test_engine && ./test_engine
// (Arduino only compiles a sketch's top folder and src/, so test/ is ignored.)
#include <cstdio>
#include "../engine.h"
#include "../config.h"

static int failures = 0;
#define CHECK(cond)                                                          \
  do {                                                                       \
    if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); failures++; } \
  } while (0)

// ---- R2: potentiometer as throttle ----------------------------------------
static void test_pot_to_throttle() {
  CHECK(potToThrottle(0, ADC_MAX) == 0.0f);
  CHECK(potToThrottle(ADC_MAX, ADC_MAX) == 1.0f);
  CHECK(potToThrottle(10, ADC_MAX) == 0.0f);             // inside deadzone
  CHECK(potToThrottle(-5, ADC_MAX) == 0.0f);             // clamped low
  CHECK(potToThrottle(ADC_MAX + 50, ADC_MAX) == 1.0f);   // clamped high
}

// Run the model for `ms` milliseconds at fixed throttle.
static float run(float rpm, float throttle, uint8_t gear, unsigned long ms) {
  for (unsigned long t = 0; t < ms; t += CAN_TX_INTERVAL)
    rpm = updateRpm(rpm, throttle, gear, CAN_TX_INTERVAL / 1000.0f);
  return rpm;
}

static void test_engine_model() {
  CHECK(run(0, 1.0f, 1, 1000) > 0);                         // throttle raises RPM
  CHECK(run(4000, 0.0f, 1, 1000) < 4000);                   // release lowers it
  CHECK(run(0, 1.0f, 1, 10000) == (float)REDLINE_RPM);      // capped at redline
  CHECK(run(3000, 0.0f, 1, 10000) == 0.0f);                 // floored at 0
  CHECK(run(2000, 1.0f, 1, 500) > run(2000, 1.0f, NUM_GEARS, 500));  // low gear pulls harder
}

// ---- Indicator R7: RPM on shift -------------------------------------------
static void test_rpm_after_shift() {
  float up = rpmAfterShift(6000, 1, 2);
  CHECK(up < 6000);                                          // upshift drops RPM
  CHECK(up > 6000 * GEAR_RATIOS[1] / GEAR_RATIOS[0] - 1 &&
        up < 6000 * GEAR_RATIOS[1] / GEAR_RATIOS[0] + 1);
  CHECK(rpmAfterShift(3000, 3, 2) > 3000);                   // downshift raises it
  CHECK(rpmAfterShift(6500, 2, 1) == (float)REDLINE_RPM);    // over-rev clamped
}

// ---- R3 / R7a / R7b: gear bounds -----------------------------------------
static void test_apply_shift() {
  CHECK(applyShift(1, GearShift::UPSHIFT) == 2);
  CHECK(applyShift(3, GearShift::DOWNSHIFT) == 2);
  CHECK(applyShift(3, GearShift::NONE) == 3);
  CHECK(applyShift(NUM_GEARS, GearShift::UPSHIFT) == NUM_GEARS);  // R7b
  CHECK(applyShift(1, GearShift::DOWNSHIFT) == 1);                // R7b
}

// ---- R3 / R7c: buttons -> shifts (needs ShiftInput::update) --------------
// Simulate holding buttons from time t0 to t1 in 1 ms steps; count shifts.
static int countShifts(ShiftInput& in, bool up, bool down,
                       unsigned long t0, unsigned long t1, GearShift want) {
  int n = 0;
  for (unsigned long t = t0; t < t1; t++)
    if (in.update(up, down, t) == want) n++;
  return n;
}

static void test_single_press_one_shift() {
  ShiftInput in;
  countShifts(in, false, false, 0, 100, GearShift::NONE);
  CHECK(countShifts(in, true, false, 100, 600, GearShift::UPSHIFT) == 1);  // held 500 ms
  CHECK(countShifts(in, false, false, 600, 700, GearShift::UPSHIFT) == 0); // release
}

static void test_two_presses_two_shifts() {
  ShiftInput in;
  int n = 0;
  n += countShifts(in, false, true, 0, 200, GearShift::DOWNSHIFT);
  n += countShifts(in, false, false, 200, 400, GearShift::DOWNSHIFT);
  n += countShifts(in, false, true, 400, 600, GearShift::DOWNSHIFT);
  CHECK(n == 2);
}

static void test_bounce_is_one_press() {
  ShiftInput in;
  int n = 0;
  // 10 ms of chatter (toggle every 2 ms), then held steady.
  for (unsigned long t = 0; t < 10; t++)
    if (in.update((t / 2) % 2 == 0, false, t) == GearShift::UPSHIFT) n++;
  n += countShifts(in, true, false, 10, 300, GearShift::UPSHIFT);
  CHECK(n == 1);
}

static void test_simultaneous_is_none() {
  ShiftInput in;
  CHECK(countShifts(in, true, true, 0, 300, GearShift::UPSHIFT) == 0);
  ShiftInput in2;
  CHECK(countShifts(in2, true, true, 0, 300, GearShift::DOWNSHIFT) == 0);
}

int main() {
  test_pot_to_throttle();
  test_engine_model();
  test_rpm_after_shift();
  test_apply_shift();
  test_single_press_one_shift();
  test_two_presses_two_shifts();
  test_bounce_is_one_press();
  test_simultaneous_is_none();
  std::printf(failures ? "%d failure(s)\n" : "all passed\n", failures);
  return failures ? 1 : 0;
}
