#include "Particle.h"
#include "Button.h"
#include "Settings.h"

#ifndef Seat_h
#define Seat_h

enum {
  STOPPED = 0,
  UP = 1,
  DOWN = 2
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
    void setMeasuringMode(bool enabled);
    void savePosition(int address);
    void loop();
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
    void reportPosition();
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
    int _seatPositionPct;
    int _feetUpPosition;
    int _flatPosition;
    bool _measuringMode;
    Settings settings;
};
#endif
