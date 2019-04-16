#ifndef CardStorage_h
#define CardStorage_h

#include "Arduino.h"
#include <Servo.h>

enum CardStorageTiltState {
  left,
  right,
  none,
};

const int TILT_LEFT = 55;
const int TILT_RIGHT = 120;
const int NO_TILT = 85;
const int MOVE_FORWARD = 180;
const int MOVE_BACKWARD = 0;
const int STOP = 89;
const int TILT_TIME = 300; //in millisecond

class CardStorage
{
  public:
    CardStorage();
    void init(int id, int drivingPin, int directionPin, int sensorPin);
    void update();
    void moveForward();
    void moveBackward();
    void stop();
    void tiltLeft();
    void tiltLeftWhenCard();
    void tiltRight();
    void tiltRightWhenCard();
    bool cardDetected();
    void noTilt();
    CardStorageTiltState getTiltState();
    void invertDrivingRotation();
  private:
    int _id;
    Servo _servoDriving;
    Servo _servoDirection;
    int _sensorPin;
    CardStorageTiltState _tiltState;
    bool _invertedDrivingRotation = false;
};

#endif