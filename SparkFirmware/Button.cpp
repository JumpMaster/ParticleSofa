#include "Button.h"
#include "application.h"

// CONSTANTS
const int longPressInterval = 300;
const int multiplePressInterval = 500;
const int debounceInterval = 20;
const int maxPresses = 2; // double click

void Button::setPin(int pin)
{
  _pin = pin;
  pinMode(_pin, INPUT);
}

int Button::checkState()
{
  unsigned long currentMillis = millis();
  int buttonState = digitalRead(_pin);
  int returnValue = 0;

  // check for button bounce and ignore it
  if (buttonState != _previousButtonState)
  {
    if ((currentMillis - _stateChangedMillis) < debounceInterval)
    {
      _stateChangedMillis = currentMillis;
      return false;
    }
    else
      _stateChangedMillis = currentMillis;
  }

  if (buttonState == LOW) // PRESSED
  {
    if (buttonState != _previousButtonState)
    {
      if (_pressedMillis == 0)
        {
          _pressedMillis = currentMillis;
          _pressCount = 0;
          returnValue = PRESSED;
        }

        _pressCount++;
      }
      else if (!_longPressExecuted && ((currentMillis - _pressedMillis) > longPressInterval))
        _longPressExecuted = true;
  }
  else if ((buttonState == HIGH) && (_pressedMillis > 0)) // RELEASED
  {
    if (!_longPressExecuted && ((_pressCount >= maxPresses) || ((currentMillis - _pressedMillis) > multiplePressInterval)))
    {
      switch (_pressCount)
      {
        case 1:
          returnValue = SINGLECLICK; // Single Click
          break;
        case 2:
          returnValue = DOUBLECLICK; // Double Click
      }
      _pressedMillis = 0;
    }
    else if (_longPressExecuted == true)
    {
      _longPressExecuted = false;
      _pressedMillis = 0;
      returnValue = LONG_PRESS_RELEASED; // LONG PRESS RELEASED
    }
    else if (buttonState != _previousButtonState)
      returnValue = RELEASED; // NOT LONG PRESS BUT RELEASED
  }
  _previousButtonState = buttonState;
  return returnValue;
}
