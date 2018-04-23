#ifndef BUTTON_H_
#define BUTTON_H_

#include <Arduino.h>

typedef void (*ButtonCallback)(uint8_t id,bool newState);


class Button {

private:
  uint8_t _id;
  uint8_t _pin;
  uint8_t _mode;
  ButtonCallback _listener;
  bool _currentState=false;

  unsigned long _lastDebounceTime=0;
public:
  Button (uint8_t id,uint8_t pin,uint8_t mode=INPUT);

  void update();
  void setListeners(ButtonCallback listener=NULL);
  bool isDown();
};

#endif