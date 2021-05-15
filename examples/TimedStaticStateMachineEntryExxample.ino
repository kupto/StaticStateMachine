/*
 * Example usage of Timed StaticStateMachine with entry states.
 *
 * Copyright (c) 2021, Kupto
 *
 * Blinks the built-in LED.
 *
 * This is free software; published under the MIT license.
 * See the LICENSE file.
 */

#include <StaticStateMachine.hpp>
#include <Arduino.h>

class BlinkerImpl : public StaticStateMachine<BlinkerImpl,
    TimedAttr<unsigned long, millis>>
{
  const uint8_t builtin_led_pin = LED_BUILTIN;

  const unsigned long on_time = 300;
  const unsigned long off_time = 700;
public:
  BlinkerImpl()
  {
    // Start in the init state
    changeState<on_init>();
  }

  DECLARE_STATE_MEMBER(on)
  { // Definition of the on state
    if (stateElapsed() > on_time)
    { // Check length of time spent in the on state
      Serial.print("Time elapsed in on state: ");
      Serial.println(stateElapsed());
      changeState<off_>(); // Use the off state default entry point
    }
  }
  DECLARE_STATE_MEMBER(off); // Just declare the off state
  // Note that the protected: access specifier is active here

  DECLARE_STATE_ENTRY_MEMBER(on, init)
  { // The on_init state
    Serial.println("on_init state");
    pinMode(builtin_led_pin, OUTPUT);
    changeState<on_>();
  }
  DECLARE_STATE_ENTRY_MEMBER(on, )
  { // The default entry point, on_ state
    Serial.println("on_ state");
    digitalWrite(builtin_led_pin, HIGH);
  }
  // Just declare the off_ state default entry point
  DECLARE_STATE_ENTRY_MEMBER(off, );
};

DEFINE_STATE_MEMBER(BlinkerImpl, off)
{ // Out of class definition of the off state
  if (stateElapsed() > off_time)
  { // Check length of time spent in the off state
    Serial.print("Time elapsed in off state: ");
    Serial.println(stateElapsed());
    changeState<on_>(); // Use the on state default entry point
  }
}

DEFINE_STATE_ENTRY_MEMBER(BlinkerImpl, off, )
{ // The default entry point, off_ state,  defined out of class
  Serial.println("off_ state");
  digitalWrite(builtin_led_pin, LOW);
}

BlinkerImpl blinker; // Instance of a state machine

void setup() {
  Serial.begin(9600);
}

void loop() {
  blinker.update(); // Call update in every loop
  /* loop iteration - state
   * 1 - on_init
   * 2 - on_
   * 3 - on
   * ...
   * X+1 - off_
   * X+2 - off
   * ...
   * Y+1 - on_
   * Y+2 - on
   * ...
   * Z+1 - off_
   * Z+2 - off
   * ...
   */
}
