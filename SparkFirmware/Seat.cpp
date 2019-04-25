#include "Seat.h"
#include "Button.h"
#include "application.h"
#include "Settings.h"

const int moveBuffer = 250;

Seat::Seat(int seatNumber, int upButtonPin, int downButtonPin, int upRelayPin, int downRelayPin, Seat *sofa, void (*callback)(int,int)) {
    _seatNumber = seatNumber;
    _sofa = sofa;
    
    _upButton.setPin(upButtonPin);
    _downButton.setPin(downButtonPin);
    
    _upRelayPin = upRelayPin;
    _downRelayPin = downRelayPin;
    pinMode(_upRelayPin, OUTPUT);
    pinMode(_downRelayPin, OUTPUT);
    
    this->callback = callback;
    
    loadPositions();
}

/*
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
*/

void Seat::setMeasuringMode(bool enabled)
{
  _measuringMode = enabled;
}

void Seat::loadPositions()
{
  int memoryLocation = _seatNumber*10;
  int feetupValue = settings.readInt(memoryLocation);
  int flatValue = settings.readInt(memoryLocation+2);

  if (feetupValue > 0 && flatValue > 0)
    setPositions(feetupValue, flatValue);
  else
    setPositions(9999, 10000);
}

void Seat::savePosition(int address)
{
  if (address == 0) // Feetup
    _feetUpPosition = _seatPosition;
  else if (address == 2) // Flat
    _flatPosition = _seatPosition;
  else
    return;

  Particle.publish("LOG", "Writing "+String(_seatPosition)+" to address "+String(address+(_seatNumber*10)), PRIVATE);
  settings.writeInt(address+(_seatNumber*10), _seatPosition);
  return;
}

void Seat::setPositions(int feetUpPosition, int flatPosition)
{
  _feetUpPosition = feetUpPosition;
  _flatPosition = flatPosition;
}

bool Seat::run() { // return value == "Did you do any work?"

    unsigned long currentMillis = millis();

    if (_seatOffTime > 0 && currentMillis >= _seatOffTime)
        stopMoving();

    int upState = _upButton.checkState();
    int downState = _downButton.checkState();
    
    if (upState == 0 && downState == 0)
        return false;
    
    if (upState == PRESSED) {
        if (_seatDirection == STOPPED)
            startMoving(UP);
        else
            stopMoving();
    } else if (downState == PRESSED) {
        if (_seatDirection == STOPPED)
            startMoving(DOWN);
        else
            stopMoving();
    } else if (downState == SINGLECLICK || upState == SINGLECLICK)
        executeShortPress();
    else if (downState == DOUBLECLICK || upState == DOUBLECLICK)
        executeDoublePress();
    else if (downState == TRIPPLECLICK || upState == TRIPPLECLICK)
        executeTripplePress();
    else if (downState == LONG_PRESS_RELEASED || upState == LONG_PRESS_RELEASED)// if (downState == LONG_PRESS_RELEASED || upState == LONG_PRESS_RELEASED)
        stopMoving();
    else
        return false;
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
  
  
  int position;
  
  if (_seatPosition < (_feetUpPosition+40)) {
      position = round(((float) _seatPosition / _feetUpPosition)*5)*10;
  } else {
      position = (round(((float) (_seatPosition-_feetUpPosition) / (_flatPosition-_feetUpPosition))*5)*10)+50;
  }
  
  callback(_seatNumber, position);
}

int Seat::getCurrentPosition() {
  int position = 0;

  if (_seatDirection == UP)
    position =  _seatPosition - (millis() - _seatMoveStartTime);
  else if (_seatDirection == DOWN)
    position =  _seatPosition + (millis() - _seatMoveStartTime);
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
        if (targetPosition > currentPosition) { // GOING DOWN
            if (_seatDirection == UP) {
                stopMoving();
                // Without this delay the Spark Core reboots
                // Could be caused by relay feedback or power draw
                // if all 6 relays are on at the same time.
                delay(50);
                startMoving(DOWN);
            } else if (_seatDirection == STOPPED)
                startMoving(DOWN);

            _seatOffTime = millis() + (targetPosition - currentPosition);
        } else { // GOING UP
            if (_seatDirection == DOWN) {
                stopMoving();
                // Without this delay the Spark Core reboots
                // Could be caused by relay feedback or power draw
                // if all 6 relays are on at the same time.
                delay(50);
                startMoving(UP);
            } else if (_seatDirection == STOPPED)
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
  if (_seatDirection == UP) // TO UPRIGHT
    moveToTarget(0);
  else if (_seatDirection == DOWN) // TO FLAT
    moveToTarget(_flatPosition);
}

void Seat::executeTripplePress() {

    int currentPosition = getCurrentPosition();

    if (_seatDirection == UP) {
        if (currentPosition < (_feetUpPosition-20)) { // TO UPRIGHT
            for (int i = 0; i < 3; i++)
                (_sofa + i)->moveToUpright();
        } else { // TO FEET
            for (int i = 0; i < 3; i++)
                (_sofa + i)->moveToFeet();
        }
    } else if (_seatDirection == DOWN) {
        if (currentPosition < (_feetUpPosition+20)) { // TO FEET UP
            for (int i = 0; i < 3; i++)
                (_sofa + i)->moveToFeet();
        } else { // TO FLAT
            for (int i = 0; i < 3; i++)
                (_sofa + i)->moveToFlat();
        }
    }
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

void Seat::moveUp() {
    int currentPosition = getCurrentPosition();

    if (currentPosition < (_feetUpPosition-20)) // TO FEET UP
        moveToTarget(_feetUpPosition);
    else // TO FLAT
        moveToTarget(_flatPosition);
}

void Seat::moveDown() {
    int currentPosition = getCurrentPosition();

    if (currentPosition < (_feetUpPosition+20)) // TO UPRIGHT
        moveToTarget(0);
    else // TO FEET
        moveToTarget(_feetUpPosition);
}
