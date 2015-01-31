#include "Seat.h"
#include "Button.h"
#include "application.h"

const int STOPPED = 0;
const int moveBuffer = 500;

void Seat::setButtonPins(int downPin, int upPin)
{
  _downButton.setPin(downPin);
  _upButton.setPin(upPin);
}

void Seat::setRelayPins(int upRelayPin, int downRelayPin)
{
  _upRelayPin = upRelayPin;
  _downRelayPin = downRelayPin;
  pinMode(_upRelayPin, OUTPUT);
  pinMode(_downRelayPin, OUTPUT);
}

void Seat::setPositions(int feetUpPosition, int flatPosition)
{
  _feetUpPosition = feetUpPosition;
  _flatPosition = flatPosition;
}

bool Seat::run() // return value == "Did you do any work?"
{
  unsigned long currentMillis = millis();

  if (_sofaOffTime > 0 && currentMillis >= _sofaOffTime)
    stopMoving();

  int upState = _upButton.checkState();
  int downState = _downButton.checkState();

  if (upState == 0 && downState == 0)
    return false;

  if (_sofaDirection == STOPPED)
  {
    if (downState == PRESSED)
      startMoving(DOWN);
    else if (upState == PRESSED)
      startMoving(UP);
  }
  else if (_sofaDirection != STOPPED)
  {
    int currentPosition = getCurrentPosition();
    if (currentPosition > 0 && currentPosition < _flatPosition)
    {
      if (downState == PRESSED || upState == PRESSED)
        stopMoving();
      else if (downState == SINGLECLICK || upState == SINGLECLICK)
        executeShortPress();
      else if (downState == DOUBLECLICK || upState == DOUBLECLICK)
        executeDoublePress();
      else if (downState == LONG_PRESS_RELEASED || upState == LONG_PRESS_RELEASED)
        stopMoving();
    }
    else
      stopMoving();
  }

    return true;
}

void Seat::startMoving(int direction) {
  if (direction == UP)
  {
    digitalWrite(_upRelayPin, HIGH);
    _sofaDirection = UP;
  }
  else if (direction == DOWN)
  {
    digitalWrite(_downRelayPin, HIGH);
    _sofaDirection = DOWN;
  }

  _sofaMoveStartTime = millis();
}

void Seat::stopMoving() {
  if (_sofaDirection == UP)
    digitalWrite(_upRelayPin, LOW);
  else if (_sofaDirection == DOWN)
    digitalWrite(_downRelayPin, LOW);

  _sofaPosition = getCurrentPosition();
  _sofaDirection = STOPPED;
  _sofaOffTime = 0;
}

int Seat::getCurrentPosition() {
  int position = 0;
  if (_sofaDirection == UP)
    position =  (_sofaPosition - (millis() - _sofaMoveStartTime));
  else if (_sofaDirection == DOWN)
    position =  (_sofaPosition + (millis() - _sofaMoveStartTime));
  else
    position = _sofaPosition;

  if (position <= 0)
    return 0;
  else if (position >= _feetUpPosition)
    return _feetUpPosition;
  else
    return position;
}

bool Seat::isMoving() {
  return _sofaDirection != STOPPED;
}

void Seat::moveToTarget(int targetPosition) {
  int currentPosition = getCurrentPosition();
  if (targetPosition < (currentPosition - 20) || targetPosition > (currentPosition + 20))
  {
    if (targetPosition > currentPosition) // GOING DOWN
    {
      if (_sofaDirection == UP)
      {
        stopMoving();
        // Without this delay the Spark Core reboots
        // Could be caused by relay feedback or power draw
        // if all 6 relays are on at the same time.
        delay(50);
        startMoving(DOWN);
      }
      else if (_sofaDirection == STOPPED)
        startMoving(DOWN);

      _sofaOffTime = millis() + (targetPosition - currentPosition);
    }
    else // GOING UP
    {
      if (_sofaDirection == DOWN)
      {
        stopMoving();
        // Without this delay the Spark Core reboots
        // Could be caused by relay feedback or power draw
        // if all 6 relays are on at the same time.
        delay(50);
        startMoving(UP);
      }
      else if (_sofaDirection == STOPPED)
        startMoving(UP);

      _sofaOffTime = millis() + (currentPosition - targetPosition);
    }

    if (targetPosition == 0 || targetPosition == _flatPosition)
      _sofaOffTime += moveBuffer;
  }
}

void Seat::executeShortPress() {
  int currentPosition = getCurrentPosition();
  if (_sofaDirection == UP)
  {
    if (currentPosition < (_feetUpPosition-20)) // TO UPRIGHT
      moveToTarget(0);
    else // TO FEET
      moveToTarget(_feetUpPosition);
  }
  else if (_sofaDirection == DOWN)
  {
    if (currentPosition < (_feetUpPosition+20)) // TO FEET UP
      moveToTarget(_feetUpPosition);
    else // TO FLAT
      moveToTarget(_flatPosition);
  }
}

void Seat::executeDoublePress() {
  Serial.println("DOUBLE PRESS");
  if (_sofaDirection == UP) // TO UPRIGHT
    moveToTarget(0);
  else if (_sofaDirection == DOWN) // TO FLAT
    moveToTarget(_flatPosition);
}

void Seat::moveToUpright() {
  moveToTarget(0);
}

void Seat::moveToFeet() {
  moveToTarget(_feetUpPosition);
}

void Seat::moveToFlat() {
  moveToTarget(_flatPosition);
}
