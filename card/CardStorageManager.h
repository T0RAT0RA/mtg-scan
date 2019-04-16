#ifndef CardStorageManager_h
#define CardStorageManager_h

#include "Arduino.h"
#include "CardStorage.h"

const int DELAY_BEFORE_TILT = 500; //ms
class CardStorageManager
{
  public:
    CardStorageManager();
    void addCardStorage(CardStorage cardStorage);
    void update();
    void sendCardTo(int storage);
    void stop();
    void noTilt();
  private:
    CardStorage _cardStorage[10];
    long _cardStorageLastDetection[10];
    int _cardStorageCount = 0;
    int _sendCardTo;
};

#endif