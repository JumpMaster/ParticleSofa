#include "Button.h"
#include "application.h"

// CONSTANTS
const int longPressInterval = 300;
const int multiplePressInterval = 750;
const int debounceInterval = 50;
const int maxPresses = 3; // tripple click

void Button::setPin(int pin)
{
  _pin = pin;
  pinMode(_pin, INPUT);
}

int Button::checkState()
{
    unsigned long currentMillis = millis();
    int buttonState;
    int returnValue = 0;
    bool buttonStateChanged = false;
    
    if ((currentMillis - _stateChangedMillis) < debounceInterval)
        buttonState = _previousButtonState;
    else
        buttonState = digitalRead(_pin);
        
    if (buttonState != _previousButtonState) {
        _stateChangedMillis = currentMillis;
        _previousButtonState = buttonState;
        buttonStateChanged = true;
    }

    if (buttonState == LOW) // PRESSED
    {
        if (buttonStateChanged)
        {
            if (_pressedMillis == 0)
            {
                _pressedMillis = currentMillis;
                _pressCount = 1;
                returnValue = PRESSED;
            } else {
                _pressCount++;
            }
        } else if (!_longPressExecuted && ((currentMillis - _stateChangedMillis) > longPressInterval)) {
            _longPressExecuted = true;
        }
    } else if (buttonState == HIGH && _pressedMillis > 0) { // RELEASED
        if (!_longPressExecuted && ((_pressCount >= maxPresses) || ((currentMillis - _pressedMillis) > multiplePressInterval))) {
            switch (_pressCount) {
                case 1:
                    returnValue = SINGLECLICK; // Single Click
                    break;
                case 2:
                    returnValue = DOUBLECLICK; // Double Click
                    break;
                case 3:
                    returnValue = TRIPPLECLICK; // Tripple Click
                    break;
            }
            _pressedMillis = 0;
        } else if (_longPressExecuted == true) {
            _longPressExecuted = false;
            _pressedMillis = 0;
            returnValue = LONG_PRESS_RELEASED; // LONG PRESS RELEASED
        }
    }
    
    return returnValue;
}
