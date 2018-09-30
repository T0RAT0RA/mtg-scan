#include <Wire.h>
#include <BreezyArduCAM.h>
#include <SPI.h>

// CAMERA
const int CS = 7;

// BUTTONS
const int BUTTON_C = 6;
const int BUTTON_D = 5;

// LEDS
const int LED_GREEN = 4;

Serial_ArduCAM_FrameGrabber fg;
ArduCAM_Mini_2MP cardCam(CS, &fg);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(921600);
  cardCam.beginJpeg800x600();
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
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
  Wire.beginTransmission(8);
  Wire.write(value);
  Wire.endTransmission();
}
unsigned long startcaptureTime;


bool modeAuto = true;
void loop(void)
{
  debugCamera();
  
  if (modeAuto) {
    switch (state) {
      case Motor1:
        {
          Serial.println("Motor1");
          sendToSlave(1);

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
          sendToSlave(2);
          state = Motor1;
        }
        break;
    }
  }
}

bool capture = false;
bool capturing = false;
void debugCamera(void) {
  if (!capturing && !capture && digitalRead(BUTTON_C) == LOW) {
    capture = true;
    capturing = true;
    startcaptureTime = millis();
    cardCam.singleCapture(true);
  }

  if (capturing) {
    cardCam.singleCapture(false);
    if (millis() - startcaptureTime > 1000) {
      capture = false;
      capturing = false;
    }
  }



}
