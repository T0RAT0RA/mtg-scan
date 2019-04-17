class CardStorage:

  def __init__(self, id, driving_pin, direction_pin, sensor_pin, calibration=None, inverted_driving_rotation=False, board=None):
    self.id = id
    self.board = board
    self.driving_pin = driving_pin
    self.direction_pin = direction_pin
    self.sensor_pin = sensor_pin
    self.calibration = calibration

    self.MOVE_FORWARD = 180
    self.MOVE_STOP = calibration['MOVE_STOP']
    self.MOVE_BACKWARD = 0
    self.TILT_STOP = calibration['TILT_STOP']
    self.TILT_LEFT = calibration['TILT_LEFT']
    self.TILT_RIGHT = calibration['TILT_RIGHT']

    self.inverted_driving_rotation = inverted_driving_rotation

    self.board.servo_config(self.driving_pin)
    self.stop()
    self.board.servo_config(self.direction_pin)
    self.noTilt()

    self.board.enable_analog_reporting(self.sensor_pin)

    # pinMode(drivingPin, OUTPUT);
    # pinMode(directionPin, OUTPUT);
    # pinMode(sensorPin, INPUT_PULLUP);

    # _servoDriving.attach(drivingPin);
    # _servoDirection.attach(directionPin);
    # _sensorPin = sensorPin;

  def cardDetected(self):
    sensor_value = self.board.analog_read(self.sensor_pin)
    print("_cardStorage " + self.id + ": cardDetected - {}".format(sensor_value))
    return sensor_value <= 900

  def invertDrivingRotation(self):
    self.inverted_driving_rotation = not self.inverted_driving_rotation


  def moveForward(self):
    print("_cardStorage " + self.id + ": moveForward")
    if not self.inverted_driving_rotation:
      self.board.analog_write(self.driving_pin, self.MOVE_FORWARD)
    else:
      self.board.analog_write(self.driving_pin, self.MOVE_BACKWARD)

  def moveBackward(self):
    print("_cardStorage " + self.id + ": moveBackward")
    if not self.inverted_driving_rotation:
      self.board.analog_write(self.driving_pin, self.MOVE_BACKWARD)
    else:
      self.board.analog_write(self.driving_pin, self.MOVE_FORWARD)

  def stop(self):
    print("_cardStorage " + self.id + ": stop")
    self.board.analog_write(self.driving_pin, self.MOVE_STOP)

  # void CardStorage::tiltLeftWhenCard()
  # {
  #   _tiltState = CardStorageTiltState::left;
  # }

  def tiltLeft(self):
    print("_cardStorage " + self.id + ": tiltLeft")
    self.board.analog_write(self.direction_pin, self.TILT_LEFT)

  # void CardStorage::tiltRightWhenCard()
  # {
  #   _tiltState = CardStorageTiltState::right;
  # }

  def tiltRight(self):
    print("_cardStorage " + self.id + ": tiltRight")
    self.board.analog_write(self.direction_pin, self.TILT_RIGHT)

  def noTilt(self):
    print("_cardStorage " + self.id + ": noTilt")
    self.board.analog_write(self.direction_pin, self.TILT_STOP)

  # CardStorageTiltState CardStorage::getTiltState()
  # {
  #   return _tiltState;
  # }