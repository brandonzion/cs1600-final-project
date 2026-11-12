#include "config.h"
#include "types.h"
#include "engine.h"

const uint8_t PIN_POT  = A0;
const uint8_t PIN_UP   = 2;
const uint8_t PIN_DOWN = 3;

#define LINK Serial1
const unsigned long LINK_BAUD = 115200;

EngineState   state       = {0, 1};  // 0 RPM, gear 1
float         rpmExact    = 0.0f;    // keeps fractional RPM between steps
unsigned long lastTick    = 0;       // start of the last engine tick
ShiftInput    shiftInput;

void transmitFrame(const EngineState& s, float throttle) {
  // TODO: replace with the shared CAN library's send call
  // For now, output in Serial Plotter format
  Serial.print("rpm:");       Serial.print(s.rpm);
  Serial.print(",gear:");     Serial.print(s.gear * 1000);
  Serial.print(",throttle:"); Serial.println(throttle * 1000);
}

void setup() {
  Serial.begin(115200);
  LINK.begin(LINK_BAUD);
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  transmitFrame(state, 0.0f);
  lastTick = millis();
}

void loop() {
  unsigned long now = millis();

  float throttle = potToThrottle(analogRead(PIN_POT), ADC_MAX);
  GearShift s = shiftInput.update(digitalRead(PIN_UP) == LOW,
                                  digitalRead(PIN_DOWN) == LOW, now);
  uint8_t newGear = applyShift(state.gear, s);
  if (newGear != state.gear) {
    rpmExact   = rpmAfterShift(rpmExact, state.gear, newGear);
    state.gear = newGear;
  }

  // Tick: advance the engine, then send CAN frame.
  if (now - lastTick >= CAN_TX_INTERVAL) {
    while (now - lastTick >= CAN_TX_INTERVAL) {
      rpmExact = updateRpm(rpmExact, throttle, state.gear, CAN_TX_INTERVAL / 1000.0f);
      lastTick += CAN_TX_INTERVAL;
    }
    state.rpm = (uint16_t)(rpmExact + 0.5f);
    transmitFrame(state, throttle);
  }
}
