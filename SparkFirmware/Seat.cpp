#include "Seat.h"
#include "Button.h"
#include "application.h"
#include "Settings.h"

const int STOPPED = 0;
// const int moveBuffer = 500;
const int moveBuffer = 250;

void Seat::setSeatNumber(int number)
{
  _seatNumber = number;
}

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

void Seat::setMeasuringMode(bool enabled)
{
  _measuringMode = enabled;
}

void Seat::loadPositions()
{
  int memoryLocation = _seatNumber*10;
  int feetupValue = settings.readInt(memoryLocation);
  int flatValue = settings.readInt(memoryLocation+2);
  Spark.publish("LOG", "SEAT="+String(_seatNumber)+" FEET="+String(feetupValue)+" FLAT="+String(flatValue));
  if (feetupValue > 0 && flatValue > 0)
    setPositions(feetupValue, flatValue);
  else
    setPositions(9999, 10000);
    // setPositions(4900, 8000);
}

void Seat::savePosition(int address)
{
  if (address == 0) // Feetup
    _feetUpPosition = _seatPosition;
  else if (address == 2) // Flat
    _flatPosition = _seatPosition;
  else
    return;

  Spark.publish("LOG", "Writing "+String(_seatPosition)+" to address "+String(address+(_seatNumber*10)));
  settings.writeInt(address+(_seatNumber*10), _seatPosition);
  return;
}

void Seat::setPositions(int feetUpPosition, int flatPosition)
{
  _feetUpPosition = feetUpPosition;
  _flatPosition = flatPosition;
}

bool Seat::run() // return value == "Did you do any work?"
{
  unsigned long currentMillis = millis();

  if (_seatOffTime > 0 && currentMillis >= _seatOffTime)
    stopMoving();

  int upState = _upButton.checkState();
  int downState = _downButton.checkState();

  if (upState == 0 && downState == 0)
    return false;

  if (_seatDirection == STOPPED)
  {
    if (downState == PRESSED)
      startMoving(DOWN);
    else if (upState == PRESSED)
      startMoving(UP);
  }
  else if (_seatDirection != STOPPED)
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
    _seatDirection = UP;
  }
  else if (direction == DOWN)
  {
    digitalWrite(_downRelayPin, HIGH);
    _seatDirection = DOWN;
  }

  _seatMoveStartTime = millis();
}

void Seat::stopMoving() {
  if (_seatDirection == UP)
    digitalWrite(_upRelayPin, LOW);
  else if (_seatDirection == DOWN)
    digitalWrite(_downRelayPin, LOW);

  _seatPosition = getCurrentPosition();
  _seatDirection = STOPPED;
  _seatOffTime = 0;
}

int Seat::getCurrentPosition() {
  int position = 0;
  if (_seatDirection == UP)
    position =  (_seatPosition - (millis() - _seatMoveStartTime));
  else if (_seatDirection == DOWN)
    position =  (_seatPosition + (millis() - _seatMoveStartTime));
  else
    position = _seatPosition;

  if (position <= 0)
    return 0;
  else if (position > _flatPosition && !_measuringMode)
    return _flatPosition;
  else
    return position;
}

bool Seat::isMoving() {
  return _seatDirection != STOPPED;
}

void Seat::moveToTarget(int targetPosition) {
  int currentPosition = getCurrentPosition();
  if (targetPosition < (currentPosition - 20) || targetPosition > (currentPosition + 20))
  {
    if (targetPosition > currentPosition) // GOING DOWN
    {
      if (_seatDirection == UP)
      {
        stopMoving();
        // Without this delay the Spark Core reboots
        // Could be caused by relay feedback or power draw
        // if all 6 relays are on at the same time.
        delay(50);
        startMoving(DOWN);
      }
      else if (_seatDirection == STOPPED)
        startMoving(DOWN);

      _seatOffTime = millis() + (targetPosition - currentPosition);
    }
    else // GOING UP
    {
      if (_seatDirection == DOWN)
      {
        stopMoving();
        // Without this delay the Spark Core reboots
        // Could be caused by relay feedback or power draw
        // if all 6 relays are on at the same time.
        delay(50);
        startMoving(UP);
      }
      else if (_seatDirection == STOPPED)
        startMoving(UP);

      _seatOffTime = millis() + (currentPosition - targetPosition);
    }

    if (targetPosition == 0 || targetPosition == _flatPosition)
      _seatOffTime += moveBuffer;
  }
}

void Seat::executeShortPress() {
  int currentPosition = getCurrentPosition();
  if (_seatDirection == UP)
  {
    if (currentPosition < (_feetUpPosition-20)) // TO UPRIGHT
      moveToTarget(0);
    else // TO FEET
      moveToTarget(_feetUpPosition);
  }
  else if (_seatDirection == DOWN)
  {
    if (currentPosition < (_feetUpPosition+20)) // TO FEET UP
      moveToTarget(_feetUpPosition);
    else // TO FLAT
      moveToTarget(_flatPosition);
  }
}

void Seat::executeDoublePress() {
  Serial.println("DOUBLE PRESS");
  if (_seatDirection == UP) // TO UPRIGHT
    moveToTarget(0);
  else if (_seatDirection == DOWN) // TO FLAT
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
