#include <Wire.h>
#include <BreezyArduCAM.h>
#include <SPI.h>

const int CS = 7;

Serial_ArduCAM_FrameGrabber fg;
ArduCAM_Mini_2MP cardCam(CS, &fg);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(921600);
  cardCam.beginJpeg800x600();
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
 
void loop(void)
{
   switch (state) {
        case Motor1:
        {
          sendToSlave(1);
          delay(1500);
          
          state = Capture;
          startcaptureTime = millis();
          cardCam.singleCapture(true);
        }
        break;
        case Capture:
        {
          cardCam.singleCapture(false);

          if ( millis() - startcaptureTime > 1000)
          {
              state = Motor2;
          }
        }
        break;
        case Motor2:
        {
          sendToSlave(2);
          delay(500);
          state = Motor1;
        }
        break;
   }
}
