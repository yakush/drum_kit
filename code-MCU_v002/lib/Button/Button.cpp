#include "Button.h"
#include <Arduino.h>

Button::Button (uint8_t id,uint8_t pin,uint8_t mode){
  _id=id;
  _pin=pin;
  _mode=mode;
  pinMode(pin,INPUT);
}

void Button::update(){
  unsigned long now=millis();
  bool read = (digitalRead(_pin) == HIGH);

  // ignore same state
  if (read == _currentState){
    _lastDebounceTime = now;
    return;
  }

  // was stable for long - change state
  if (now > 50 + _lastDebounceTime){
    _currentState=read;
    if (_listener){
      _listener(_id,_currentState);
    }
  }
}

void Button::setListeners(ButtonCallback listener){
  _listener =listener;
}

bool Button::isDown(){
  return _currentState;
}