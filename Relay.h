// Relay.h
#ifndef RELAY_h
#define RELAY_h

#include <Arduino.h>

enum RelayState {
    ON = LOW,
    OFF = HIGH   
};

class Relay {
  private:
    int pin;
    RelayState state;

  public:
    Relay(int pin);
    void turnOn();
    void turnOff();
    void toggle(RelayState state);
    RelayState getState();
};

#endif
