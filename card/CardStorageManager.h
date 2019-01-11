#ifndef CardStorageManager_h
#define CardStorageManager_h

#include "Arduino.h"
#include "CardStorage.h"

class CardStorageManager
{
  public:
    CardStorageManager();
    void addCardStorage(CardStorage cardStorage);
    void update();
    void sendCardTo(int storage);
    void stop();
  private:
    CardStorage _cardStorage[10];
    int _cardStorageCount = 0;
    int _sendCardTo;
};

#endif