import time
from CardStorage import CardStorage

class CardStorageManager:
  DELAY_BEFORE_TILT = 500 # ms

  def __init__(self, board=None):
    self.board = board
    self.card_storages = []

  def setBoard(self, board):
    self.board = board

  def addCardStorage(self, card_storage_config):
    card_storage = CardStorage(**card_storage_config, board=self.board)
    self.card_storages.append(card_storage)
    card_storage.noTilt()
    return card_storage

  def getCardStorages(self):
    return self.card_storages

  def update(self):
    stopAll = False

    for storage in self.getCardStorages():
      if storage.isCardDetected() and storage.getTiltState():
        if storage.getTiltState() == 1:
            time.sleep(0.1)
            storage.stop()
            storage.tiltLeft()
            time.sleep(0.5)
            stopAll = True
        elif storage.getTiltState() == 2:
            time.sleep(0.1)
            storage.stop()
            storage.tiltRight()
            time.sleep(0.5)
            stopAll = True
        elif storage.getTiltState() == 3:
            time.sleep(2)
            storage.stop()
            stopAll = True

    if stopAll:
      for storage in self.getCardStorages():
        storage.stop()
        storage.noTilt()


  def sendCardTo(self, targetStorage):
    targetStorageBox = round((targetStorage + 0.5) / 2) - 1;

    for i, storage in enumerate(self.getCardStorages()):
        storage.stop()
        storage.noTilt()

        if i <= targetStorageBox:
          storage.moveForward()

          if i == len(self.getCardStorages()) - 1:
            storage.noTiltWhenCard()

          if i == targetStorageBox:
            if targetStorage % 2 == 0:
              storage.tiltLeftWhenCard()
            else:
              storage.tiltRightWhenCard()


    if targetStorage == 0:
      self.getCardStorages()[0].moveBackward()
      self.getCardStorages()[0].noTiltWhenCard()

  def stop(self):
    for storage in self.getCardStorages():
      storage.stop()

  def noTilt(self):
    for storage in self.getCardStorages():
      storage.noTilt()