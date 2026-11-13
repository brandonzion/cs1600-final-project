#include <stdint.h>

// Vehicle parameters
const uint16_t REDLINE_RPM = 7000;
const uint8_t  NUM_GEARS   = 6;
const float GEAR_RATIOS[NUM_GEARS] = {3.50f, 2.10f, 1.40f, 1.00f, 0.80f, 0.65f};

// Engine model 
const float MAX_ACCEL_RPM_PER_S  = 3000.0f;
// RPM drop rate at no throttle
const float ENGINE_BRAKE_RPM_PER_S = 1500.0f;
const float THROTTLE_DEADZONE = 0.02f;

// Timing 
const unsigned long CAN_TX_INTERVAL = 20;  // ms
const unsigned long DEBOUNCE_MS = 30;

// Potentiometer 
const int ADC_MAX = 1023; 
