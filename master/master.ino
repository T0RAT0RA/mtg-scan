#include <Wire.h>
#include <BreezyArduCAM.h>
#include <ShiftedLCD.h>
#include <SPI.h>

// CAMERA
const int CS_CAM = 10;
// LCD
const int CS_LCD = 2;

// BUTTONS
const int BUTTON_A = 7;
const int BUTTON_B = 6;
const int BUTTON_C = 5;
const int BUTTON_D = 4;
const int SWITCH = 3;

// LEDS
const int LED_GREEN = 9;
const int LED_YELLOW = 8;

// CONST
const int SEND_CARD = 1;
const int OPEN_SERVO = 2;

// SERIAL
const long SERIAL_SPEED = 921600; //921600
const int ARDUINO_ADDRESS = 8;
const int SLAVE_ADDRESS = 8;

// CARD COLORS
const char CARD_NONE = 'N';
const char CARD_UNKNOWN = 'X';
const char CARD_RED = 'R';
const char CARD_GREEN = 'G';
const char CARD_BLUE = 'U';
const char CARD_BLACK = 'B';
const char CARD_WHITE = 'W';
const char CARD_COLORLESS = 'C';
const char CARD_OTHER = 'O';


Serial_ArduCAM_FrameGrabber fg;
ArduCAM_Mini_2MP cardCam(CS_CAM, &fg);

//LiquidCrystal lcd(CS_LCD);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(SERIAL_SPEED);
  //digitalWrite(CS_CAM, HIGH);
  //digitalWrite(CS_LCD, HIGH);
  cardCam.beginJpeg800x600();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  //selectLcd();
  //lcd.begin(16, 2);
  //printLcd("Ready to start!");
}

enum states {
  None,
  Motor1,
  Motor2,
  Waiting,
  Capture
};
states state = Motor1;

void sendToSlave(byte value)
{
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(value);
  Wire.endTransmission();
}
unsigned long startcaptureTime;

bool modeAuto = false;
void loop(void)
{
  buttonA();
  buttonB();
  buttonC();
  buttonD();

  if (digitalRead(SWITCH) == HIGH) {
    modeAuto = false;
    state = Motor1;
    digitalWrite(LED_GREEN, LOW);
    //selectLcd();
    //lcd.setCursor(0, 0);
    //lcd.print("Mode: manual");
  } else {
    modeAuto = true;
    digitalWrite(LED_GREEN, HIGH);
    //selectLcd();
    //lcd.setCursor(0, 0);
    //lcd.print("Mode: auto  ");
  }

  if (modeAuto) {
    switch (state) {
      case Motor1:
        {
          //printLcd("Sending card");
          sendToSlave(SEND_CARD);
          delay(1000);
          state = Capture;
        }
        break;
      case Capture:
        {
          //printLcd("Capturing image");
          digitalWrite(LED_YELLOW, HIGH);
          cardCam.singleCapture();
          digitalWrite(LED_YELLOW, LOW);
          state = Waiting;
        }
        break;
      case Waiting:
        {
          //printLcd("Waiting data");
          state = Motor2;
          //if (Serial.available()) {
          //  char color = Serial.read();
          //  if (color == 'G') {
          //    state = Motor2;
          //  } else {
          //    state = Motor1;
          //  }
          //}
        }
        break;
      case Motor2:
        {
          //printLcd("Sorting card");
          sendToSlave(OPEN_SERVO);
          state = Motor1;
          delay(500);
        }
        break;
    }
  }
}

void selectCam(void)
{
  //digitalWrite(CS_CAM, LOW);
  //digitalWrite(CS_LCD, HIGH);
}

void selectLcd(void)
{
  //digitalWrite(CS_CAM, HIGH);
  //digitalWrite(CS_LCD, LOW);
}

void printLcd(char message[])
{
  //selectLcd();
  //lcd.setCursor(0, 1);
  //lcd.print(message);
  //digitalWrite(CS_LCD, HIGH);
}

// SEND CARD
void buttonA()
{
  if (digitalRead(BUTTON_A) == LOW) {
    sendToSlave(SEND_CARD);
  }
}

// SEND CARD TO SORTING
void buttonB()
{
  if (digitalRead(BUTTON_B) == LOW) {
    sendToSlave(OPEN_SERVO);
  }
}

// TAKE CAMERA PICTURE
bool capturing = false;
void buttonC(void) {
  if (!capturing && digitalRead(BUTTON_C) == LOW) {
    //printLcd("Manual capture..");
    //selectCam();
    capturing = true;
    digitalWrite(LED_YELLOW, HIGH);
    cardCam.singleCapture();
    digitalWrite(LED_YELLOW, LOW);
    capturing = false;
    //printLcd("                ");
  }
}

void buttonD(void) {
}