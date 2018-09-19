#include <Wire.h>
#include <BreezyArduCAM.h>
#include <SPI.h>
#include <Servo.h>

Servo myservo;

static const int CS = 7;
static const int CARD_HOLDER_SERVO = 5;
static const int CARD_HOLDER_SENSOR = 6;
static const int RED = 2;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long last_debounce_time = 0;  // the last time the output pin was toggled
unsigned long debounce_delay = 50;    // the debounce time; increase if the output flickers

int sensor_value;
int previous_sensor_value = HIGH;

Serial_ArduCAM_FrameGrabber fg;

ArduCAM_Mini_2MP myCam(CS, &fg);

void setup(void)
{
  // ArduCAM Mini uses both I^2C and SPI buses
  Wire.begin();
  SPI.begin();

  // Fastest baud rate 921600 (change to 115200 for Due)
  Serial.begin(921600);

  // Begin capturing in JPEG
  myCam.beginJpeg640x480();

  myservo.attach(CARD_HOLDER_SERVO);
  myservo.write(90);

  pinMode(CARD_HOLDER_SENSOR, INPUT);
  pinMode(RED, OUTPUT);
}

void loop(void)
{
  int card_reading = digitalRead(CARD_HOLDER_SENSOR);

  if (card_reading != previous_sensor_value) {
    // reset the debouncing timer
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
    if (card_reading != sensor_value) {
      sensor_value = card_reading;

      if (sensor_value == LOW) {
        Serial.write(42);
      }
    }
  }
  previous_sensor_value = card_reading;
  myCam.capture();

}
