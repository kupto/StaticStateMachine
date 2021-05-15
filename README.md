# Statically constructed StateMachine Library for Arduino
Very simple and easy to use, see examples. This is an Arduino
library, but the implementation is platform independent.

Define a state machine as follows:
```C++
class MyStateMachine : public StaticStateMachine<MyStateMachine>
{
	// Declare states
}
```

to make a timed state machine on Arduino:
```C++
class MyTimedStateMachine : public StaticStateMachine<
    MyTimedStateMachine,
    TimedAttr<unsigned long, millis>>
{
	DECLARE_STATE_MEMBER(init) {
		Serial.println("Initialize");
		changeState<run>();
	}
	DECLARE_STATE_MEMBER(run); // Must be defined, use `DEFINE_STATE_MEMBER()`
}
```

Use the `DECLARE_STATE_MEMBER()` macro and friends to declare and
define state logic. Each state must have a unique name. Such name
must be a valid C identifier.

Use `changeState<>()` inside a state logic to change the current
state:
```C++
MyTimedStateMachine mtsm;
mtsm.changeState<MyTimedStateMachine::init>();
mtsm.update(); // Prints "Initialize" and changes state to "run"
mtsm.update(); // Evaluates the "run" state
```

This is still in beta, changes and/or comments are welcome.

# License
Copyright (c) 2021 Kupto - MIT License
