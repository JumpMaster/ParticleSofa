#include "Button.h"
#include "Settings.h"

#ifndef Seat_h
#define Seat_h

static const int UP = 1;
static const int DOWN = 2;

class Seat
{
  public:
    // FUNCTIONS
    void setSeatNumber(int number);
    void setButtonPins(int upButton, int downButton);
    void setRelayPins(int upRelay, int downRelay);
    void setMeasuringMode(bool enabled);
    void loadPositions();
    void savePosition(int address);
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
    void setPositions(int feetUpPosition, int flatPosition);
    // VARIABLES
    Button _upButton;
    Button _downButton;
    int _upRelayPin;
    int _downRelayPin;
    unsigned long _seatOffTime;
    unsigned long _seatMoveStartTime;
    int _seatNumber;
    int _seatDirection;
    int _seatPosition;
    int _feetUpPosition;
    int _flatPosition;
    bool _measuringMode;
    Settings settings;
};
#endif
