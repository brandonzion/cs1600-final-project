#include <WDT.h>
#include "state_machine.h"
#include "lighting.h"

full_state indicatorState;
LedColor LedColors[LED_COUNT];

void setup() {
    Serial.begin(115200);
    indicatorState.savedClock = millis();
    determineLedColors(indicatorState, millis(), LedColors);
    if (!WDT.begin(20)) Serial.println("Watchdog initialization failed");
}

void loop() {
    // still have to do the button logic
    const bool newCanFrame = false;
    const uint32_t receivedRPM = 0;
    const uint8_t receivedGear = 1;
    const bool buttonPressed = false;
    const uint32_t now = millis();

    indicatorState = updateSM(indicatorState, newCanFrame, receivedRPM,
                              receivedGear, buttonPressed, now);
    determineLedColors(indicatorState, now, LedColors);
    // Still need to update theLED driver with the new colors
    WDT.refresh();
}
