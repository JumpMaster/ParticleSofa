#include "application.h"

#ifndef Button_h
#define Button_h

static const int PRESSED = 1;
// static const int RELEASED = 2;
static const int LONG_PRESS_RELEASED = 2;
static const int SINGLECLICK = 3;
static const int DOUBLECLICK = 4;
static const int TRIPPLECLICK = 5;

class Button
{
  public:
    void setPin(int pin);
    int checkState();
  private:
    int _pin;
    // BUTTON DETAILS
    int _previousButtonState = HIGH;
    unsigned long _stateChangedMillis;
    unsigned long _pressedMillis;
    int _pressCount;
    bool _longPressExecuted;   // to stop long presses executing over and over
};
#endif
