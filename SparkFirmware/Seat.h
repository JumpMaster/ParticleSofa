#include "Button.h"

#ifndef Seat_h
#define Seat_h

static const int UP = 1;
static const int DOWN = 2;

class Seat
{
  public:
    // FUNCTIONS
    void setButtonPins(int upButton, int downButton);
    void setRelayPins(int upRelay, int downRelay);
    void setPositions(int feetUpPosition, int flatPosition);
    bool run();
    void startMoving(int direction);
    void stopMoving();
    void moveToUpright();
    void moveToFeet();
    void moveToFlat();
    int getCurrentPosition();
    bool isMoving();
  private:
    // FUNCTIONS
    void executeShortPress();
    void executeDoublePress();
    void moveToTarget(int targetPosition);
    // VARIABLES
    Button _upButton;
    Button _downButton;
    int _upRelayPin;
    int _downRelayPin;
    unsigned long _sofaOffTime;
    unsigned long _sofaMoveStartTime;
    int _sofaDirection;
    int _sofaPosition;
    int _feetUpPosition;
    int _flatPosition;
};
#endif
