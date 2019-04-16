#include "Arduino.h"
#include "CardStorage.h"
#include <Servo.h>


CardStorage::CardStorage()
{
}

void CardStorage::init(int id, int drivingPin, int directionPin, int sensorPin)
{
  _id = id;

  pinMode(drivingPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);

  _servoDriving.attach(drivingPin);
  _servoDirection.attach(directionPin);
  _sensorPin = sensorPin;
}

void CardStorage::update()
{
}

bool CardStorage::cardDetected()
{
  return digitalRead(_sensorPin) != HIGH;
}

void CardStorage::invertDrivingRotation()
{
  _invertedDrivingRotation = !_invertedDrivingRotation;
}

void CardStorage::moveForward()
{
  Serial.println("_cardStorage " + String(_id) + ": moveForward");
  if (!_invertedDrivingRotation) {
    _servoDriving.write(MOVE_FORWARD);
  } else {
    _servoDriving.write(MOVE_BACKWARD);
  }
}

void CardStorage::moveBackward()
{
  Serial.println("_cardStorage " + String(_id) + ": moveBackward");
  if (!_invertedDrivingRotation) {
    _servoDriving.write(MOVE_BACKWARD);
  } else {
    _servoDriving.write(MOVE_FORWARD);
  }
}

void CardStorage::stop()
{
  Serial.println("_cardStorage " + String(_id) + ": stop");
  _servoDriving.write(STOP);
}

void CardStorage::tiltLeftWhenCard()
{
  _tiltState = CardStorageTiltState::left;
}

void CardStorage::tiltLeft()
{
  Serial.println("_cardStorage " + String(_id) + ": tiltLeft");
  _servoDirection.write(TILT_LEFT);
}

void CardStorage::tiltRightWhenCard()
{
  _tiltState = CardStorageTiltState::right;
}

void CardStorage::tiltRight()
{
  Serial.println("_cardStorage " + String(_id) + ": tiltRight");
  _servoDirection.write(TILT_RIGHT);
}

void CardStorage::noTilt()
{
  Serial.println("_cardStorage " + String(_id) + ": noTilt");
  _servoDirection.write(NO_TILT);
  _tiltState = CardStorageTiltState::none;
}

CardStorageTiltState CardStorage::getTiltState()
{
  return _tiltState;
}