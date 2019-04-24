import serial, time, arrow
import serial.tools.list_ports
from functools import partial
from signal import pause
from guizero import App, Waffle, Text, TextBox, PushButton, Box, Picture, yesno, Window, ListBox, Slider
from PyMata.pymata import PyMata
from CardCaptures import CardCaptures
from CardStorageManager import CardStorageManager

LOADER_IMG = "ui/loader.gif"
CAPTURE_FOLDER = "captures/"
CAPTURES = CAPTURE_FOLDER + arrow.now().format('YYYY-MM-DD HH:mm:ss') + '/'

SERVO_CARD_PIN = 12
SERVO_CARD_CLOSE = 60
SERVO_CARD_OPEN = 140
SERVO_OPEN_TIME = 0.3 # sec

COLORS = {
  "unknown": 0,
  "red": 1,
  "green": 2,
  "white": 3,
  "black": 4,
  "blue": 5,
  "multi": 6,
  "land": 7,
}
storages = [
  {
    "id": "A",
    "sensor_pin": 0,
    "driving_pin": 3,
    "direction_pin": 4,
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
    "driving_pin": 5,
    "direction_pin": 6,
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
    "driving_pin": 7,
    "direction_pin": 8,
    "calibration": {
      "MOVE_STOP": 90,
      "TILT_STOP": 87,
      "TILT_LEFT": 110,
      "TILT_RIGHT": 50,
    }
  },
]


board = None
card_storage_manager = CardStorageManager()
card_captures = CardCaptures(CAPTURES)

# Ask the user if they really want to close the window
def on_close():
  app.destroy()
  # if yesno("Close", "Do you want to quit?"):
  #   app.destroy()

def toggleServo():
    board.analog_write(SERVO_CARD_PIN, SERVO_CARD_OPEN)
    time.sleep(SERVO_OPEN_TIME)
    board.analog_write(SERVO_CARD_PIN, SERVO_CARD_CLOSE)

def initDashboard():
  global board, card_storage_manager

  # TODO: delete those lines when ready
  time.sleep(0.5)
  loading_box.hide()
  main.show()
  return

  board = PyMata("/dev/ttyUSB0")
  card_storage_manager.setBoard(board)

  board.servo_config(SERVO_CARD_PIN)
  board.analog_write(SERVO_CARD_PIN, SERVO_CARD_CLOSE)

  for i, storage in enumerate(storages):
    card_storage = card_storage_manager.addCardStorage(storage)

    button = PushButton(card_storage_controls, grid=[i,0], enabled=False, text="")
    storages[i]['button'] = button

    move_forward = PushButton(card_storage_controls, command=card_storage.moveForward, grid=[i,1], text="^")
    stop_button = PushButton(card_storage_controls, grid=[i,2], command=card_storage.stop, text="STOP")
    move_backward = PushButton(card_storage_controls, command=card_storage.moveBackward, grid=[i,3], text="v")
    tilt_left_button = PushButton(card_storage_controls, command=card_storage.tiltLeft, grid=[i,4], text="LEFT")
    tilt_stop_button = PushButton(card_storage_controls, command=card_storage.noTilt, grid=[i,5], text="NO TILT")
    tilt_right_button = PushButton(card_storage_controls, command=card_storage.tiltRight, grid=[i,6], text="RIGHT")

  # Hide loader and display the dashboard
  loading_box.hide()
  main.show()

def update():
  global card_storage_manager
  if not card_storage_manager:
    return

  card_storage_manager.update()

  for i, storage in enumerate(card_storage_manager.getCardStorages()):
    # print("update {} {}".format(i, storage.isCardDetected()))
    if storage.isCardDetected():
      storages[i]['button'].bg = 'green'
    else:
      storages[i]['button'].bg = None

def sendCardTo(x, y):
  print('Send card to {} {}'.format(x, y))
  if (x,y) == (1,4):
    card_storage_manager.sendCardTo(0)
  if (x,y) == (2,3):
    card_storage_manager.sendCardTo(1)
  if (x,y) == (0,3):
    card_storage_manager.sendCardTo(2)
  if (x,y) == (2,2):
    card_storage_manager.sendCardTo(3)
  if (x,y) == (0,2):
    card_storage_manager.sendCardTo(4)
  if (x,y) == (2,1):
    card_storage_manager.sendCardTo(5)
  if (x,y) == (0,1):
    card_storage_manager.sendCardTo(6)
  if (x,y) == (1,0):
    card_storage_manager.sendCardTo(7)

def sendCardToCapture():
  pass

def sendCardToSort():
  pass

def captureCard():
  new_card = card_captures.capture()
  card_captures.analyzeCard(new_card)

  card_capture_image_list.clear()
  for img in card_captures.getImages():
    card_capture_image_list.append(img)

  card_capture_image_list.value = new_card.getImgName()
  selectCardImage(new_card.getImgName())

  card_capture_image_box.show()


def selectCardImage(image):
  selected_card = card_captures.getCardByImgName(image)
  card_capture_image.image = selected_card.getImgPath()
  if selected_card.hasError():
    card_capture_data.value = '\n'.join([
      'Text Found: %s' % selected_card.text_found,
      'Error: %s' % selected_card.error,
    ])
  else:
    card_capture_data.value = '\n'.join([
      'Text Found: %s' % selected_card.text_found,
      'Card Name: %s' % selected_card.name,
      'Price (USD): %s' % selected_card.price,
      'Colors: %s' % selected_card.colors
    ])

def updateCardList():
  card_listtext.value = '\n'.join([
    '%i %s' % (card['quantity'], card['name']) for name, card in card_captures.getCardList().items()
  ])

app = App(title = 'MTG Scan', width = 1600, height = 800)
# app.tk.attributes("-fullscreen",True)


loading_box = Box(app)
loading_text = Text(loading_box, text="Connecting to Arduino...")
loading_img = Picture(loading_box, image=LOADER_IMG, width=60, height=60)

main = Box(app, width="fill", height="fill")
main.hide()

menu_box = Box(main, width="fill", align="top")
close = PushButton(menu_box, text="X", align="right", command=on_close)

card_dispenser_controls = Box(main, border=True, align="left", height="fill", width="fill")
Text(card_dispenser_controls, text="Dispenser")
card_dispenser_button = PushButton(card_dispenser_controls, text="Send card to capture", command=sendCardToCapture)

card_capture_controls = Box(main, border=True, align="left", height="fill", width="fill")
card_capture_title = Text(card_capture_controls, text="Capture")
card_capture_buttons_line1 = Box(card_capture_controls, align="top")
card_captures_button = PushButton(card_capture_buttons_line1, text="Capture", align="left", command=captureCard)
# card_capture_send_button = PushButton(card_capture_buttons_line1, text="Send to sort", align="left", command=sendCardToSort)

card_capture_buttons_line2 = Box(card_capture_controls, align="top")
PushButton(card_capture_buttons_line2, command=lambda: board.analog_write(SERVO_CARD_PIN, SERVO_CARD_OPEN), text="OPEN", align="left")
PushButton(card_capture_buttons_line2, command=lambda: board.analog_write(SERVO_CARD_PIN, SERVO_CARD_CLOSE), text="CLOSE", align="right")
PushButton(card_capture_buttons_line2, command=toggleServo, text="TOGGLE", align="bottom")

card_capture_image_box = Box(card_capture_controls, align="bottom", height="fill")
card_capture_image_box.hide()
card_capture_image_list = ListBox(
    card_capture_image_box,
    items=card_captures.getImages(),
    command=selectCardImage,
    scrollbar=True,
    height="100",
    width=int(800*1/3)
  )
card_capture_image = Picture(card_capture_image_box,
  width=int(800*1/3),
  height=int(600*1/3)
)
card_capture_data = Text(card_capture_image_box)

card_sorter_controls = Box(main, border=True, align="left", height="fill", width="fill")
card_sorter_title = Text(card_sorter_controls, text="Sorter")
card_sorter_buttons = Box(card_sorter_controls, align="top")

sort_buttons = {}
for i, (color, storage) in enumerate(COLORS.items()):
  button = PushButton(card_sorter_buttons, text=color, align="left", command=partial(card_storage_manager.sendCardTo, storage))
  sort_buttons["color"] = button

card_list_box = Box(card_sorter_controls, border=True, align="bottom", height=600, width="fill")
card_list_title = Text(card_list_box, text="Card list")
card_list_textbox = Box(card_list_box, width="fill")
card_list_textbox.bg = "white"
card_listtext = Text(
  card_list_textbox,
  align="left"
)
card_listtext.repeat(500, updateCardList)
# card_storage_controls = Box(main, layout="grid")
# waffle = Waffle(main, height=5, width=3, command=sendCardTo, dim=40)
# waffle.pixel(1, 0).color = 'green'
# waffle.pixel(0, 1).color = 'green'
# waffle.pixel(2, 1).color = 'green'
# waffle.pixel(0, 2).color = 'green'
# waffle.pixel(2, 2).color = 'green'
# waffle.pixel(0, 3).color = 'green'
# waffle.pixel(2, 3).color = 'green'
# waffle.pixel(1, 4).color = 'green'


# When the user tries to close the window, run the function do_this_on_close()
app.on_close(on_close)

app.repeat(100, update)
app.after(100, initDashboard)
app.display()