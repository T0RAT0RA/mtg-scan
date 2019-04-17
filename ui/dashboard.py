import serial, time, arrow
import serial.tools.list_ports
from functools import partial
from signal import pause
from guizero import App, Text,TextBox, PushButton, Box, yesno, Window, ListBox, Slider
from pymata_aio.pymata3 import PyMata3
from pymata_aio.constants import Constants
from CardStorageManager import CardStorageManager

board = None
card_storage_manager = None

storages = [
  {
    "id": "A",
    "sensor_pin": 0,
    "driving_pin": 2,
    "direction_pin": 3,
    "calibration": {
      "MOVE_STOP": 87,
      "TILT_STOP": 80,
      "TILT_LEFT": 110,
      "TILT_RIGHT": 50,
    }
  },
  {
    "id": "B",
    "sensor_pin": 1,
    "driving_pin": 4,
    "direction_pin": 5,
    "inverted_driving_rotation": True,
    "calibration": {
      "MOVE_STOP": 90,
      "TILT_STOP": 87,
      "TILT_LEFT": 110,
      "TILT_RIGHT": 50,
    }
  },
  {
    "id": "C",
    "sensor_pin": 2,
    "driving_pin": 6,
    "direction_pin": 7,
    "calibration": {
      "MOVE_STOP": 90,
      "TILT_STOP": 87,
      "TILT_LEFT": 110,
      "TILT_RIGHT": 50,
    }
  },
]

# Ask the user if they really want to close the window
def on_close():
  app.destroy()
  # if yesno("Close", "Do you want to quit?"):
  #   app.destroy()

def initDashboard():
  global board, card_storage_manager

  board = PyMata3(2)
  card_storage_manager = CardStorageManager(board)

  print (card_storage_manager.board)
  for i, storage in enumerate(storages):
    card_storage = card_storage_manager.addCardStorage(storage)
    # storage['sensor'] = LineSensor(storage['sensorPin'], threshold=0.1)
    # storage['drivingServo'] = Servo(storage['drivingPin'])
    # storage['drivingServo'].value = SERVO_STOP / 100
    # storage['directionServo'] = AngularServo(storage['directionPin'], initial_angle=TILT_STOP)

    button = PushButton(sliders, grid=[i,0], enabled=False, text="")
    storages[i]['button'] = button
    # storage['sensor'].when_line = partial(updateButtonBg, button, "green")
    # storage['sensor'].when_no_line = partial(updateButtonBg, button ,None)

    # drivingSlider = Slider(sliders, start=0, end=180, command=partial(updateServo, storage['drivingPin']), horizontal=False, width=50, height=200, grid=[i,1])
    move_forward = PushButton(sliders, command=card_storage.moveForward, grid=[i,1], text="^")
    stop_button = PushButton(sliders, grid=[i,2], command=card_storage.stop, text="STOP")
    move_backward = PushButton(sliders, command=card_storage.moveBackward, grid=[i,3], text="v")
    tilt_left_button = PushButton(sliders, command=card_storage.tiltLeft, grid=[i,4], text="LEFT")
    tilt_stop_button = PushButton(sliders, command=card_storage.noTilt, grid=[i,5], text="NO TILT")
    tilt_right_button = PushButton(sliders, command=card_storage.tiltRight, grid=[i,6], text="RIGHT")

  # Hide loader and display the dashboard
  loader.hide()
  sliders.show()

def update():
  print("update")
  for i, storage in enumerate(card_storage_manager.getCardStorages()):
    print("update" + storage.id)
    if storage.cardDetected():
      storages[i]['button'].bg = 'green'
    else:
      storages[i]['button'].bg = None


app = App(title = 'Door System', width = 1600, height = 900)
app.tk.attributes("-fullscreen",True)

menu_box = Box(app, width="fill", align="top")
sliders = Box(app, layout="grid")
sliders.hide()
loader = Text(app, text="Loading dashboard...")
close = PushButton(menu_box, text="X", align="right", command=on_close)

# When the user tries to close the window, run the function do_this_on_close()
app.on_close(on_close)

app.repeat(300, update)
app.after(100, initDashboard)
app.display()