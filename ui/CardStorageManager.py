from CardStorage import CardStorage

class CardStorageManager:
  DELAY_BEFORE_TILT = 500 # ms

  def __init__(self, board=None):
    self.board = board
    self.card_storages = []

  def addCardStorage(self, card_storage_config):
    card_storage = CardStorage(**card_storage_config, board=self.board)
    self.card_storages.append(card_storage)
    card_storage.noTilt()
    return card_storage

  def getCardStorages(self):
    return self.card_storages

  def update(self):
    # bool stopAll = false;
    # for (int i = 0; i < _cardStorageCount; i++) {
    #   _cardStorage[i].update();
    #   if (_cardStorage[i].cardDetected() && _cardStorage[i].getTiltState() != CardStorageTiltState::none) {
    #     switch (_cardStorage[i].getTiltState()) {
    #       case CardStorageTiltState::left:
    #         delay(100);
    #         _cardStorage[i].stop();
    #         _cardStorage[i].tiltLeft();
    #         delay(500);
    #         stopAll = true;
    #       break;
    #       case CardStorageTiltState::right:
    #         delay(100);
    #         _cardStorage[i].stop();
    #         _cardStorage[i].tiltRight();
    #         delay(500);
    #         stopAll = true;
    #       break;
    #       default:
    #         _cardStorage[i].noTilt();
    #       break;
    #     }
    #   }
    # }

    # if (stopAll) {
    #   for (int i = 0; i < _cardStorageCount; i++) {
    #     _cardStorage[i].stop();
    #     _cardStorage[i].noTilt();
    #   }
    # }
    pass

  def sendCardTo(self, storage):
    # int targetStorage = round((storage + 0.5) / 2) - 1;

    # for (int i = 0; i < _cardStorageCount; i++) {

    #     _cardStorage[i].stop();
    #     _cardStorage[i].noTilt();

    #     if (i <= targetStorage) {
    #     _cardStorage[i].moveForward();
    #       if (i == targetStorage) {
    #         if (storage % 2 == 0) {
    #           _cardStorage[i].tiltLeftWhenCard();
    #         } else {
    #           _cardStorage[i].tiltRightWhenCard();
    #         }
    #       }
    #     }
    # }

    # if (storage == 0) {
    #   _cardStorage[0].moveBackward();
    # }
    pass

  def stop(self):
    for storage in self.storages:
      storage.stop()

  def noTilt(self):
    for storage in self.storages:
      storage.noTilt()