#include "Arduino.h"
#include "CardStorageManager.h"


CardStorageManager::CardStorageManager()
{
}

void CardStorageManager::update()
{
  bool stopAll = false;
  for (int i = 0; i < _cardStorageCount; i++) {
    _cardStorage[i].update();
    if (_cardStorage[i].cardDetected() && _cardStorage[i].getTiltState() != CardStorageTiltState::none) {
      switch (_cardStorage[i].getTiltState()) {
        case CardStorageTiltState::left:
          delay(100);
          _cardStorage[i].stop();
          _cardStorage[i].tiltLeft();
          delay(500);
          stopAll = true;
        break;
        case CardStorageTiltState::right:
          delay(100);
          _cardStorage[i].stop();
          _cardStorage[i].tiltRight();
          delay(500);
          stopAll = true;
        break;
        default:
          _cardStorage[i].noTilt();
        break;
      }
    }
  }

  if (stopAll) {
    for (int i = 0; i < _cardStorageCount; i++) {
      _cardStorage[i].stop();
      _cardStorage[i].noTilt();
    }
  }
}

void CardStorageManager::addCardStorage(CardStorage cardStorage)
{
  _cardStorage[_cardStorageCount] = cardStorage;
  _cardStorage[_cardStorageCount].noTilt();
  _cardStorageCount++;
}

void CardStorageManager::sendCardTo(int storage)
{
  int targetStorage = round((storage + 0.5) / 2) - 1;

  for (int i = 0; i < _cardStorageCount; i++) {

      _cardStorage[i].stop();
      _cardStorage[i].noTilt();

      if (i <= targetStorage) {
      _cardStorage[i].moveForward();
        if (i == targetStorage) {
          if (storage % 2 == 0) {
            _cardStorage[i].tiltLeftWhenCard();
          } else {
            _cardStorage[i].tiltRightWhenCard();
          }
        }
      }
  }

  if (storage == 0) {
    _cardStorage[0].moveBackward();
  }
}

void CardStorageManager::stop()
{
  for (int i = 0; i < _cardStorageCount; i++) {
    _cardStorage[i].stop();
  }
}
void CardStorageManager::noTilt()
{
  for (int i = 0; i < _cardStorageCount; i++) {
    _cardStorage[i].noTilt();
  }
}