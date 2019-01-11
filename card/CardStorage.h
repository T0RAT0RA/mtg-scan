#ifndef CardStorage_h
#define CardStorage_h

#include "Arduino.h"
#include <Servo.h>

enum CardStorageTiltState {
  left,
  right,
  none,
};

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
  private:
    int _id;
    Servo _servoDriving;
    Servo _servoDirection;
    int _sensorPin;
    CardStorageTiltState _tiltState;
    long _tiltStart;
};

#endif