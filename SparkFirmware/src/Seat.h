#include "Button.h"
#include "Settings.h"

#ifndef Seat_h
#define Seat_h

enum {
  STOPPED,
  UP,
  DOWN
};

enum {
  UPRIGHT,
  FEETUP,
  FLAT
};

class Seat
{
  public:
    // FUNCTIONS
    Seat(int seatNumber, int upRelayPin, int downRelayPin, int upButtonPin, int downButtonPin, Seat *sofa, void (*callback)(int,int));
    void (*callback)(int, int);
    // void setSeatNumber(int number);
    // void setButtonPins(int upButton, int downButton);
    // void setRelayPins(int upRelay, int downRelay);
    void setMeasuringMode(bool enabled);
    void savePosition(int address);
    bool run();
    void startMoving(int direction);
    void stopMoving();
    void moveToUpright();
    void moveToFeet();
    void moveToFlat();
    void moveUp();
    void moveDown();
    int getCurrentPosition();
    bool isMoving();
  private:
    // FUNCTIONS
    void executeShortPress();
    void executeDoublePress();
    void executeTripplePress();
    void moveToTarget(int targetPosition);
    void setPositions(int feetUpPosition, int flatPosition);
    void loadPositions();
    // VARIABLES
    Seat *_sofa;
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
