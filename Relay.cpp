// Relay.cpp
#include "Relay.h"

Relay::Relay(int pin) {
  this->pin = pin;
  state     = OFF;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, OFF);
}

void Relay::turnOn() {
  if(state == ON)
    return;
  state = ON;
  digitalWrite(pin, state);
  Serial.print(F("turned ON pin "));
  Serial.println(pin);
}

void Relay::turnOff() {
  if(state == OFF)
    return;
  state = OFF;
  digitalWrite(pin, state);
  Serial.print(F("turned OFF pin "));
  Serial.println(pin);
}

void Relay::toggle(RelayState newState) {
  state = newState;
  digitalWrite(pin, newState);
}

RelayState Relay::getState() {
  return state;
}
