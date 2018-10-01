#include <Wire.h>
#include <BreezyArduCAM.h>
#include <SPI.h>

// CAMERA
const int CS = 10;

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


Serial_ArduCAM_FrameGrabber fg;
ArduCAM_Mini_2MP cardCam(CS, &fg);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(SERIAL_SPEED);
  cardCam.beginJpeg800x600();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
}

enum states {
  None,
  Motor1,
  Motor2,
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
    digitalWrite(LED_GREEN, LOW);
  } else {
    modeAuto = true;
    digitalWrite(LED_GREEN, HIGH);
  }

  if (modeAuto) {
    switch (state) {
      case Motor1:
        {
          Serial.println("Motor1");
          sendToSlave(SEND_CARD);
          delay(1500);
          state = Capture;
          startcaptureTime = millis();
          cardCam.singleCapture(true);
        }
        break;
      case Capture:
        {
          Serial.println("Capture");
          cardCam.singleCapture(false);

          if ( millis() - startcaptureTime > 5000)
          {
            state = Motor2;
          }
        }
        break;
      case Motor2:
        {
          Serial.println("Motor2");
          sendToSlave(OPEN_SERVO);
          state = Motor1;
        }
        break;
    }
  }
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
bool capture = false;
bool capturing = false;
void buttonC(void) {
  if (!capturing && !capture && digitalRead(BUTTON_C) == LOW) {
    capture = true;
    capturing = true;
    startcaptureTime = millis();
    digitalWrite(LED_YELLOW, HIGH);
    cardCam.singleCapture(true);
  }

  if (capturing) {
    cardCam.singleCapture(false);
    if (millis() - startcaptureTime > 1000) {
      capture = false;
      capturing = false;
    	digitalWrite(LED_YELLOW, LOW);
    }
  }
}

void buttonD(void) {
}