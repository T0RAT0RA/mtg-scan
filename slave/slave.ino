#include <Servo.h>
#include <Wire.h>

Servo cardServo;

// MOTORS
const int MOTOR_CARD_FORWARD = 9;
const int MOTOR_CARD_BACKWARD = 10;
const int MOTOR_CARD_PWM = 11;
const int MOTOR_CARD_FORWARD_TIME = 600; //ms
const int MOTOR_CARD_BACKWARD_TIME = 100; //ms
const int SERVO_CARD = 2;
const int SERVO_CLOSE = 60;
const int SERVO_OPEN = 130;
const int SERVO_OPEN_TIME = 250; //ms

// SERIAL
const long SERIAL_SPEED = 921600; //921600
const int ARDUINO_ADDRESS = 8;

// CARD COLORS
const int CARD_NONE = 0;
const int CARD_UNKNOWN = -1;
const int CARD_RED = 1;
const int CARD_GREEN = 2;
const int CARD_BLUE = 3;
const int CARD_BLACK = 4;
const int CARD_WHITE = 5;
const int CARD_COLORLESS = 6;
const int CARD_OTHER = 7;

void setup(void)
{
  Serial.begin(SERIAL_SPEED);

  pinMode(MOTOR_CARD_FORWARD, OUTPUT);
  pinMode(MOTOR_CARD_BACKWARD, OUTPUT);
  pinMode(MOTOR_CARD_PWM, OUTPUT);
  pinMode(SERVO_CARD, OUTPUT);

  cardServo.attach(SERVO_CARD);
  cardServo.write(SERVO_CLOSE);

  Wire.begin(ARDUINO_ADDRESS);
  Wire.onReceive(receiveEvent);
}

bool send_card = false;
bool sort_card = false;
void receiveEvent(int howMany) {
  int x = Wire.read();
  if (x == 1) {
    send_card = true;
  } else if (x == 2) {
    sort_card = true;
  }
}

bool serving_new_card = false;
bool prepare_new_card = false;
unsigned long serving_card_start = 0;
void sendCardtoAnalyse(void)
{
  if (!serving_new_card && !prepare_new_card && send_card) {
    serving_new_card = true;
    serving_card_start = millis();
    analogWrite(MOTOR_CARD_FORWARD, 255);
    analogWrite(MOTOR_CARD_BACKWARD, 0);
    analogWrite(MOTOR_CARD_PWM, 90);
  }

  if (serving_new_card && (millis() - serving_card_start) > MOTOR_CARD_FORWARD_TIME) {
    serving_new_card = false;
    prepare_new_card = true;
    analogWrite(MOTOR_CARD_FORWARD, 0);
    analogWrite(MOTOR_CARD_BACKWARD, 255);
    analogWrite(MOTOR_CARD_PWM, 255);
  }

  if (prepare_new_card && (millis() - serving_card_start) > MOTOR_CARD_FORWARD_TIME + MOTOR_CARD_BACKWARD_TIME) {
    send_card = false;
    prepare_new_card = false;
    analogWrite(MOTOR_CARD_FORWARD, 0);
    analogWrite(MOTOR_CARD_BACKWARD, 0);
  }
}

bool sorting_card = false;
unsigned long sorting_card_start = 0;
void sendCardToSorting()
{
  if (!sorting_card && sort_card) {
    sorting_card = true;
    sort_card = false;
    sorting_card_start = millis();
    cardServo.write(SERVO_OPEN);
  }

  if (sorting_card && (millis() - sorting_card_start) > SERVO_OPEN_TIME) {
    sorting_card = false;
    cardServo.write(SERVO_CLOSE);
  }
}

void loop(void)
{
  sendCardtoAnalyse();
  sendCardToSorting();
}
