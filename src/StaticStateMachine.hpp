/*
 * Statically constructed StateMachine implementation.
 *
 * Copyright (c) 2021, Kupto
 *
 * Allows to implement a Finite State Machine (FSM). An instance of FSM must
 * inherit from the StaticStateMachine<> class.  Each state is declared by a
 * member [update_]function and a nested templated class (whose type defines
 * the state). The FSM runs by calling its update() member (f.e. from loop()
 * method).  The update() in turn calls the current state [update_]function.
 * Single state [update_]function is evaluated every iteration. Make sure to
 * keep state implementation concise, without much sleeping.
 *
 * Use the changeState<>() template member method to switch states. Template
 * argument is the appropriate nested templated class, unique to each state.
 * Even though it is possible to alter FSM state fom outside, this should be
 * avoided, due to possible race conditions. Only use changeState<>() inside
 * a state [update_]function, perhaps after checking flags, set from outside.
 *
 * Think of each nested state class as of a node in FSM directed graph. Edge
 * in this graph represents a state [update_]function. Most nodes have self-
 * assigning edge, called an [update_]function.  Some nodes have in addition
 * an incoming edges (entry points). State entry states may be defined.
 *
 * This is free software; published under the MIT license.
 * See the LICENSE file.
 */

#include <inttypes.h>

#pragma once

// TODO: Allow for multiple attributes at once
template <typename States, typename Attribute = void>
class StaticStateMachine; // Forward declaration

/**
 * StaticStateMachine<> - Base FSM implementation
 *
 * This template class should be inherited by a State Machine implementation
 * logic. Pass in child class for template argument.  This follows curiously
 * recurring template pattern, such as:
 *
 *  class MyFSMImpl : public StaticStateMachine< MyFSMImpl >
 *  { ... }; // Implementation logic, defining and using states
 */
template <typename States>
class StaticStateMachine<States> {
protected:
  /**
   * Updater - Pointer to a member method, implementing single FSM state
   */
  typedef void (States::*Updater)();

  /**
   * LogicImpl - An alias for the child class / user of StaticStateMachine<>
   */
  using LogicImpl = States;

  volatile Updater _currentState;

public:
  const volatile Updater& state() const {
    return _currentState;
  }

  template<typename S>
  bool isState() const {
    return _currentState == S::state();
  }

  template<typename S>
  void changeState() {
    _currentState = S::state();
  }

  // TODO: Change state defined by lambda or functor

  /**
   * update() - evaluate current FSM state
   */
  void update() {
    // C++17 std::invoke(_currentState);
    auto &t = static_cast<LogicImpl&>(*this);
    (t.*t._currentState)();
  }
};


/**
 * TimedAttr<> - Attribute extending StaticStateMachine<>
 *
 * Used to pass TimeStamp function (F) and its return type (R) into the
 * StaticStateMachine<> class.
 */
template <typename R, R (*F)()>
struct TimedAttr;

/**
 * StaticStateMachine<> - Extends StaticStateMachine<> with time keeping
 *
 * In addition, this State Machine keeps the amount of time spent in current
 * FSM state. This is useful for timed state transitions. This template gets
 * specialized with TimedAttr<>, passing in TimeStamp function pointer,  and
 * its return type. The return type must be an integral time-stamp for which
 * the following holds `TimeType elapsed = TimeStamp() - PreviousTimeStamp;`
 */
template <typename States, typename TimeType, TimeType (*TimeStamp)()>
class StaticStateMachine<States, TimedAttr<TimeType, TimeStamp>>
    : public StaticStateMachine<States> {
protected:
  volatile TimeType _changeTS;

public:
  TimeType stateElapsed() const {
      return TimeStamp() - _changeTS;
  }

  template<typename S>
  void changeState() {
    _changeTS = TimeStamp();
    StaticStateMachine<States>::template changeState<S>();
  }
};


/**
 * DECLARE_STATE_MEMBER() - Create a new FSM state
 * @name: Name of the new state, must be a valid C identifier
 *
 * Use this to declare a state member of implementation logic, e.g.
 *
 *  class MyFSMImpl : public StaticStateMachine< MyFSMImpl >
 *  {
 *    DECLARE_STATE_MEMBER(first_state); // Forward declared 1st state
 *    DECLARE_STATE_MEMBER(second_state) {
 *      // 2nd state code
 *    }
 *    ...
 *  };
 */
#define DECLARE_STATE_MEMBER(name) \
  public: struct name { \
    static Updater state() \
    { return &LogicImpl::update_ ## name; } \
  }; \
  protected: void update_ ## name()

/**
 * DEFINE_STATE_MEMBER() - Define the logic of an existing state
 * @implementation: State Machine implementation logic child class
 * @name: Name of an existing (declared but not yet defined) state
 *
 * Use this to implement a state outside of the child class definition, e.g.
 *
 *  DEFINE_STATE_MEMBER(MyFSMImpl, first_state) {
 *    // 1st state code
 *  }
 */
#define DEFINE_STATE_MEMBER(implementation, name) \
  void implementation::update_ ## name()

/**
 * DECLARE_STATE_ENTRY_MEMBER() - Create a new FSM state entry state
 * @to: Name of the state to enter (the @to FSM state must exist)
 * @name: Name of the new state entry, must be a valid C identifier
 *
 * The state entry method gets called once, before which a transition to @to
 * FSM state occurs. Transition to @to happens first so that the state entry
 * method may transfer to yet another state. As a consequence, the FSM state
 * entry state should not be tested against via the `isState()` method. Note
 * that more state entry points may be defined for single FSM state.
 */
#define DECLARE_STATE_ENTRY_MEMBER(to, name) \
  public: struct to ## _ ## name { \
    static Updater state() \
    { return &LogicImpl::update_ ## to ## _ ## name ; } \
  }; \
  protected: void update_ ## to ## _ ## name() { \
    changeState<to>(); \
    enter_ ## to ## _ ## name(); \
  } \
  protected: void enter_ ## to ## _ ## name()

/**
 * DEFINE_STATE_ENTRY_MEMBER() - Define the logic of an existing state entry
 * @implementation: State Machine implementation logic child class
 * @to: Name of the state to enter (the @to FSM state must exist)
 * @name: Name of an existing (declared but not yet defined) state entry
 *
 * Same as `DEFINE_STATE_MEMBER()` but for FSM state entry state.
 */
#define DEFINE_STATE_ENTRY_MEMBER(implementation, to, name) \
  void implementation::enter_ ## to ## _ ## name()
