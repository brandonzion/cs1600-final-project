// c++ -std=c++11 -Wall -Wextra indicator/state_machine.cpp indicator/lighting.cpp \
//   indicator/test/test_indicator.cpp -o /tmp/test_indicator && /tmp/test_indicator
#include "../lighting.h"
#include <cassert>
#include <cstdio>

static bool black(LedColor c) { return c.r == 0 && c.g == 0 && c.b == 0; }

static void assertAllOff(const LedColor (&colors)[LED_COUNT]) {
    for (auto c : colors) assert(black(c));
}

static void assertAllRed(const LedColor (&colors)[LED_COUNT]) {
    for (auto c : colors) assert(c.r == 255 && c.g == 0 && c.b == 0);
}

static full_state activeState(indicator_mode mode = NORMAL) {
    full_state state;
    state.mode = mode;
    return updateSM(state, true, 1000, 1, false, 0);
}

static void test_idle() {
    full_state state;
    LedColor colors[LED_COUNT];
    assert(state.mode == NORMAL && state.rpm == 0 && state.gear == 1);
    assert(!state.activated && !state.ended);
    determineLedColors(state, 0, colors);
    assertAllOff(colors);

    state = updateSM(state, true, 0, 1, false, 10);
    assert(!state.activated && state.savedClock == 10);
    state = updateSM(state, false, 0, 1, true, 11);
    assert(state.mode == RACE);
    determineLedColors(state, 11, colors);
    assertAllOff(colors); // switching modes doesn't move us out of idle
}

static void test_activation() {
    full_state rpmChanged = updateSM(full_state{}, true, 500, 1, false, 20);
    assert(rpmChanged.activated && rpmChanged.rpm == 500);
    full_state gearChanged = updateSM(full_state{}, true, 0, 2, false, 20);
    assert(gearChanged.activated && gearChanged.gear == 2);

    rpmChanged = updateSM(rpmChanged, true, 0, 1, false, 30);
    assert(rpmChanged.activated && rpmChanged.rpm == 0);
}

static void test_mode_button() {
    full_state state = activeState();
    state = updateSM(state, false, 0, 1, true, 1);
    assert(state.mode == RACE);
    state = updateSM(state, false, 0, 1, true, 2);
    assert(state.mode == RACE); // check if we hold button, only one mode switch
    state = updateSM(state, false, 0, 1, false, 3);
    state = updateSM(state, false, 0, 1, true, 4);
    assert(state.mode == NORMAL);

    state = updateSM(state, false, 0, 1, false, DATA_TIMEOUT_MS);
    assert(state.ended);
    state = updateSM(state, false, 0, 1, true, DATA_TIMEOUT_MS + 1);
    assert(state.ended && state.mode == RACE);
}



int main() {
    test_idle();
    test_activation();
    test_mode_button();
    std::puts("tests passed.");
}
