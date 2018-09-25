#include <Wire.h>
#include <BreezyArduCAM.h>
#include <SPI.h>

const int CS = 10;

Serial_ArduCAM_FrameGrabber fg;
ArduCAM_Mini_2MP cardCam(CS, &fg);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(921600);
  cardCam.beginJpeg800x600();
}

void loop(void)
{
  cardCam.capture();
}
