#include "can_frame.h"


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
}

void loop() {
  CANFrame incomingFrame;
}