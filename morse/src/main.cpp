// PROJECT: Morze
// MAINTAINER: @manjulife

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// if LiquidCrystal_I2C lcd01(0x3f,20,4) else LiquidCrystal_I2C lcd02(0x26,20,4);
LiquidCrystal_I2C lcd01(0x26,20,4);

//---> CLASS MultiButton
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
}; //<--- END CLASS MultiButton

//---> CLASS ResetBtn
class ResetBtn : public MultiButton{
public:
  ResetBtn(int pin):
    MultiButton(pin){
  }

}; //<--- END CLASS RestBtn

//---> CLASS MorseBtn
class MorseBtn : public MultiButton{
public:
  MorseBtn(int pin):
    MultiButton(pin){
  }

}; //<--- END CLASS MorseBtn


//---> CLASS MorseTransmitter
//MorseBtn, RestBtn, Text, Relay, LedRelay, Sound
class MorseTransmitter : public ResetBtn , public MorseBtn {
public:
  MorseTransmitter(int Morsepin, int Resetpin, String text, int relay, int ledW, int Tonepin):
     ResetBtn(Resetpin), MorseBtn(Morsepin){
      _num = 0;
      _cursor = 0;
      _letter = 0;
      _unit = "";
      _isSend =false;
      _isOpen = false;
      _isSound = false;
      _isDot = false;
      _isDash = false;
      _text = text;
      _relay = relay;
      _tone = Tonepin;
      _ledW = ledW;
      pinMode(_relay, OUTPUT);
      pinMode(_ledW, OUTPUT);
      //pinMode(_tone, );
      // lcd.init();                      // initialize the lcd
      // lcd.backlight();
      // lcd.blink_on();
      // pinMode(_relay, OUTPUT);
    }

  void update(){
    MorseBtn::update();
    ResetBtn::update();

    now = millis();
    if (_isOpen){
      Serial.println("--- OPEN ---");
      lcd01.clear();
      digitalWrite(_ledW,HIGH);
      digitalWrite(_relay, HIGH);
      delay(10000);
      lcd01.noBacklight();
      lcd01.noDisplay();
      digitalWrite(_relay, LOW);
      digitalWrite(_ledW,LOW);
      delay(1000);
      _num = 0;
      _cursor = 0;
      _letter = 0;
      _unit = "";
      _outText = "";
      _isSend =false;
      _isOpen =false;
      _isSound = false;
      _isDot = false;
      _isDash = false;
    }

    if (MorseBtn::isLongClick()) {
        _unit = _unit +"-";
        //lcd01.print("-");
        lcd01.print("-");
        Serial.println(_unit);
        _lastTransition = now;
        _isSend = true;
        _isSound = true;
        _isDash = true;
        _cursor++;
    }

    if (MorseBtn::isSingleClick()) {
        _unit = _unit + ".";
        //lcd01.print(".");
        lcd01.print(".");
        Serial.println(_unit);
        _lastTransition = now;
        _isSend = true;
        _isSound = true;
        _isDot = true;
        _cursor++;
    }

    if ((ResetBtn::isClick())&&(_letter > 0)){
        _cursor--;
        _letter--;
        lcd01.setCursor(_letter, 0);
        lcd01.print(" ");
        lcd01.setCursor(_letter, 0);
        _outText.remove(_letter,1);
        Serial.println(_outText);
    }

    if (!_isSend) {
      return;
    }

    soundMorse();

    int diff = now - _lastTransition;
    if ((diff >= dotLen)&&(_isDot)) {_isSound = false; _isDot = false;}
    if ((diff >= UNIT_PAUSE)&&(diff <= ELEMENT_PAUSE)&&(_isDash)) {
       //  Serial.println("-unit");
        _isSound = false;
        _isDash = false;
    }

    if (diff >= ELEMENT_PAUSE) {
        Serial.print("unit: ");
        Serial.println(_unit);
        _line = findCharacter(_unit);
        //lcd.setCursor(_unit.length(),0);
        printCharacter(_unit.length(),_letter);
        //lcd01.setCursor(_letter, 0);
        lcd01.setCursor(_letter, 0);
        if (_line != " "){
          Serial.println("lcd>>");
          _outText =_outText + _line;
        //  lcd01.print(_line);
          lcd01.print(_line);
          _letter++;
        }
    //
    Serial.print("line: ");
    Serial.println(_line);
    Serial.print("outText: ");
    Serial.println(_outText);
    Serial.print("text: ");
    Serial.println(_text);
    //
    if (_text == _outText ) {
        _isOpen = true;
        Serial.println("WIN");
    }
    _unit = "";
    _isSend = false;
    }
    //Serial.println(_unit);
    //Serial.println("*****");
  } // END UPDATE
private:
  String _unit;
  String _line;
  String _outText;
  String _text;
  int _tone;
  int _num;
  int _cursor;
  int _letter;
  int _relay;
  int _ledW;
  int note = 1200;
  int dotLen = 100;
  int dashLen = dotLen * 3;
  const int NUM_LETTER = 40;
  unsigned int _lastTransition;
  unsigned int now;
  bool _isSend;
  bool _isOpen;
  bool _isSound;
  bool _isDot;
  bool _isDash;
  static const int UNIT_PAUSE    =  300; // ms
  static const int ELEMENT_PAUSE = 900; // ms
  // static const char* dictionary[NUM_LETTER] = {".-", "-...", "--."};

  String dictionaryEn = "abcdefghijklmnopqrstuvwxyz0123456789";

  String dictionaryMorse[40] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....",
  "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.",
  "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-",
  "-.--", "--..", "-----", ".----", "..---", "...--",
  "....-", ".....", "-....", "--...", "---..", "----."};
  //State _state;
  void printCharacter(int s, int countLetter){
    for (int item=countLetter; item<s+countLetter; item++){
        lcd01.setCursor(item,0);
        lcd01.print(" ");
    }

  }// end printCharacter

  void soundMorse(){
    if (_isSound){
      tone(_tone, note);
    } else {
      noTone(_tone);
      //Serial.println("sound-off");
    }
  }
  char findCharacter(String sequence){
    Serial.println();
    char character = ' ';
    for (int item=0; item<NUM_LETTER; item++){
      if (dictionaryMorse[item] == sequence) {
        character = dictionaryEn[item];
      break;
      } // endif
    } // endfor
    return character;
  } // END findSymbol
};
//<--- END CLASS MorseTransmitter

//______________________ INIT __________________________
//MorseBtn, RestBtn, Text, Relay, LedRelay, Sound, LCD
MorseTransmitter morseTransmitter01(2, 3, "wolf", 4, 6, 9);

void setup() {
  Serial.begin(9600);
  lcd01.init();
  lcd01.backlight();
  lcd01.display();
  lcd01.blink_on();
}

void loop() {
  morseTransmitter01.update();
} // END loop
