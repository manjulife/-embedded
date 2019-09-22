#include <Arduino.h>
// PROJECT: Morze
// MAINTAINER: @manjulife
//
class MultiButton {
  public:

    MultiButton(int pin) {
      _pin = pin;
      _lastTransition = millis();
      _state = StateIdle;
      _new = false;
      pinMode(_pin, INPUT);
    }

    void update() {
      bool pressed = digitalRead(_pin);
      _new = false;

      if (!pressed && _state == StateIdle) {
        return;
      }

      unsigned int now = millis();
      int diff = now - _lastTransition;

      State next = StateIdle;
      switch (_state) {
        case StateIdle:                next = _checkIdle(pressed, diff);                break;
        case StateDebounce:            next = _checkDebounce(pressed, diff);            break;
        case StatePressed:             next = _checkPressed(pressed, diff);             break;
        case StateClickUp:             next = _checkClickUp(pressed, diff);             break;
        case StateClickIdle:           next = _checkClickIdle(pressed, diff);           break;
        case StateSingleClick:         next = _checkSingleClick(pressed, diff);         break;
        case StateDoubleClickDebounce: next = _checkDoubleClickDebounce(pressed, diff); break;
        case StateDoubleClick:         next = _checkDoubleClick(pressed, diff);         break;
        case StateLongClick:           next = _checkLongClick(pressed, diff);           break;
        case StateOtherUp:             next = _checkOtherUp(pressed, diff);             break;
      }
      //Serial.print(diff);
      //Serial.print("next:");
      //Serial.println(next);
      //Serial.print("_state:");
      //Serial.println(_state);
      if (next != _state) {

        _lastTransition = now;
        // Enter next state
        _state = next;

        _new = true;
      }
    } // END UPDATE


    bool isClick() const {
      return _new && (_state == StatePressed || _state == StateDoubleClick);
    }


    bool isSingleClick() {
      return _new && _state == StateSingleClick;
    }


    bool isDoubleClick() {
      return _new && _state == StateDoubleClick;
    }

    bool isLongClick() {
      return _new && _state == StateLongClick;
    }

    bool isReleased() {
      return _new && (_state == StateClickUp || _state == StateOtherUp);
    }

  private:
    static const int DEBOUNCE_DELAY    =  65; // ms
    static const int SINGLECLICK_DELAY = 100; // ms
    static const int LONGCLICK_DELAY   = 300; // ms
    int _pin;

    enum State {
      StateIdle,
      StateDebounce,
      StatePressed,
      StateClickUp,
      StateClickIdle,
      StateSingleClick,
      StateDoubleClickDebounce,
      StateDoubleClick,
      StateLongClick,
      StateOtherUp,
    };

    unsigned int _lastTransition;
    State _state;
    bool _new;

    State _checkIdle(bool pressed, int diff) {
      (void)diff;
      // Wait for a key press
      return pressed ? StateDebounce : StateIdle;
    }

    State _checkDebounce(bool pressed, int diff) {

      if (!pressed) {
        return StateIdle;
      }
      if (diff >= DEBOUNCE_DELAY) {

        return StatePressed;
      }
      return StateDebounce;
    }

    State _checkPressed(bool pressed, int diff) {
      //Serial.println("--- Pressed");

      if (!pressed) {
        return StateClickUp;
      }

      if (diff >= LONGCLICK_DELAY) {
        Serial.println(diff);
        return StateLongClick;
      }
      return StatePressed;
    }

    State _checkClickUp(bool pressed, int diff) {
      (void)pressed;
      (void)diff;
      return StateClickIdle;
    }

    State _checkClickIdle(bool pressed, int diff) {
      if (pressed) {
        return StateDoubleClickDebounce;
      }
      if (diff >= SINGLECLICK_DELAY) {
        Serial.println(diff);
        return StateSingleClick;
      }
      return StateClickIdle;
    }

    State _checkSingleClick(bool pressed, int diff) {
      (void)pressed;
      (void)diff;
      return StateIdle;
    }

    State _checkDoubleClickDebounce(bool pressed, int diff) {
      if (!pressed) {
        return StateClickIdle;
      }
      if (diff >= DEBOUNCE_DELAY) {
        return StateDoubleClick;
      }
      return StateDoubleClickDebounce;
    }

    State _checkDoubleClick(bool pressed, int diff) {
      (void)diff;
      if (!pressed) {
        return StateOtherUp;
      }
      return StateDoubleClick;
    }

    State _checkLongClick(bool pressed, int diff) {
      (void)diff;
      if (!pressed) {
        return StateOtherUp;
      }
      return StateLongClick;
    }

    State _checkOtherUp(bool pressed, int diff) {
      (void)pressed;
      (void)diff;
      return StateIdle;
    }
}; // END CLASS
//
class MorzeKeyBtn : public MultiButton {
public:
  MorzeKeyBtn(int pin):
    MultiButton(pin){
      _num = 0;
      _unit = "";
      _isSend =false;
    }

  void update(){
    MultiButton::update();
    now = millis();
    if (MultiButton::isLongClick()) {
        _unit = _unit +"-";
        //Serial.println(_unit);
        _lastTransition = now;
        _isSend = true;
    }

    if (MultiButton::isSingleClick()) {
        _unit = _unit + ".";
        //Serial.println(_unit);
        _lastTransition = now;
        _isSend = true;
    }

    if (!_isSend) {
      return;
    }

    int diff = now - _lastTransition;

    if ((diff >= UNIT_PAUSE)&&(diff <= ELEMENT_PAUSE)) {
        Serial.println("-unit");
    }

    if (diff >= ELEMENT_PAUSE) {
        Serial.print("-elemnt: ");
        Serial.println(_unit);
        _unit = "";
        _isSend = false;
    }
    //Serial.println(_unit);
    //Serial.println("*****");
  } // END UPDATE
private:
  String _unit;
  int _num;
  unsigned int _lastTransition;
  unsigned int now;
  bool _isSend;
  static const int UNIT_PAUSE    =  300; // ms
  static const int ELEMENT_PAUSE = 1000; // ms
  //State _state;
};

MorzeKeyBtn morze(2);

void setup() {
  Serial.begin(9600);
}

void loop() {
  morze.update();
} // END loop
