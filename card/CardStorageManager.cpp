#include "Arduino.h"
#include "CardStorageManager.h"


CardStorageManager::CardStorageManager()
{
}

void CardStorageManager::update()
{
  for (int i = 0; i < _cardStorageCount; i++) {
    _cardStorage[i].update();
  }
}

void CardStorageManager::addCardStorage(CardStorage cardStorage)
{
  _cardStorage[_cardStorageCount] = cardStorage;
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
            _cardStorage[i].tiltRightWhenCard();
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