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
  if (cardDetected() && _tiltState != CardStorageTiltState::none) {
    switch (_tiltState) {
      case CardStorageTiltState::left:
        tiltLeft();
      break;
      case CardStorageTiltState::right:
        tiltRight();
      break;
      default:
        noTilt();
      break;
    }
  } else {
    if (_tiltStart > 0 && millis() - _tiltStart >= 2000) {
      noTilt();
    }
  }
}

bool CardStorage::cardDetected()
{
  //TODO: Update condition
  return digitalRead(_sensorPin) == LOW;
}

void CardStorage::moveForward()
{
  Serial.println("_cardStorage " + String(_id) + ": moveForward");
  _servoDriving.write(180);
}

void CardStorage::moveBackward()
{
  Serial.println("_cardStorage " + String(_id) + ": moveBackward");
  _servoDriving.write(0);
}

void CardStorage::stop()
{
  Serial.println("_cardStorage " + String(_id) + ": stop");
  _servoDriving.write(90);
}

void CardStorage::tiltLeftWhenCard()
{
  _tiltState = CardStorageTiltState::left;
}

void CardStorage::tiltLeft()
{
  Serial.println("_cardStorage " + String(_id) + ": tiltLeft");
  _tiltStart = millis();
  _servoDirection.write(50);
}

void CardStorage::tiltRightWhenCard()
{
  _tiltState = CardStorageTiltState::right;
}

void CardStorage::tiltRight()
{
  Serial.println("_cardStorage " + String(_id) + ": tiltRight");
  _tiltStart = millis();
  _servoDirection.write(130);
}

void CardStorage::noTilt()
{
  Serial.println("_cardStorage " + String(_id) + ": noTilt");
  _tiltStart = 0;
  _servoDirection.write(90);
  _tiltState = CardStorageTiltState::none;
}
