#include <Servo.h>

Servo cardServo;

// MOTORS
static const int MOTOR_CARD = 5;
static const int MOTOR_CARD_TIME = 250; //ms
static const int SERVO_CARD = 2;
static const int SERVO_CLOSE = 60;
static const int SERVO_OPEN = 130;
static const int SERVO_OPEN_TIME = 300; //ms

// BUTTONS
static const int BUTTON_A = 7;
static const int BUTTON_B = 8;

// SENSORS
static const int CARD_HOLDER_SENSOR = 6;

// LEDS
static const int LED_RED = 4;
static const int LED_GREEN = 3;

// CARD COLORS
static const int CARD_NONE = 0;
static const int CARD_UNKNOWN = -1;
static const int CARD_RED = 1;
static const int CARD_GREEN = 2;
static const int CARD_BLUE = 3;
static const int CARD_BLACK = 4;
static const int CARD_WHITE = 5;
static const int CARD_COLORLESS = 6;
static const int CARD_OTHER = 7;


void setup(void)
{
  pinMode(MOTOR_CARD, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SERVO_CARD, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(CARD_HOLDER_SENSOR, INPUT);


  cardServo.attach(SERVO_CARD);
  cardServo.write(SERVO_CLOSE);
}

bool serving_new_card = false;
unsigned long serving_card_start = 0;
void sendCardtoAnalyse(void)
{
  int button_a = digitalRead(BUTTON_A);
  if (!serving_new_card && button_a == LOW) {
    serving_new_card = true;
    serving_card_start = millis();
    analogWrite(MOTOR_CARD, 100);
  }

  if (serving_new_card && (millis() - serving_card_start) > MOTOR_CARD_TIME) {
    serving_new_card = false;
    analogWrite(MOTOR_CARD, 0);
  }
}

int card_detected = HIGH;
int previous_card_reading_value = HIGH;
unsigned int last_debounce_time_card = 0;
int debounce_delay = 50; //ms
bool isCardReadyToAnalyze(void)
{
  int card_reading = digitalRead(CARD_HOLDER_SENSOR);

  if (card_reading != previous_card_reading_value) {
    // reset the debouncing timer
    last_debounce_time_card = millis();
  }

  if ((millis() - last_debounce_time_card) > debounce_delay) {
    if (card_reading != card_detected) {
      card_detected = card_reading;
    }
  }
  previous_card_reading_value = card_reading;

  return card_detected == LOW;
}

bool sorting_card = false;
unsigned long sorting_card_start = 0;
void sendCardToSorting(bool send)
{
  if (!sorting_card && send) {
    sorting_card = true;
    sorting_card_start = millis();
    cardServo.write(SERVO_OPEN);
  }

  if (sorting_card && !send && (millis() - sorting_card_start) > SERVO_OPEN_TIME) {
    sorting_card = false;
    cardServo.write(SERVO_CLOSE);
  }
}

bool analyzing_card = false;
bool tmp = false;
unsigned long analyzing_card_start = 0;
int card_color = CARD_NONE;
int getCardColor()
{
  if (!tmp) {
    return CARD_NONE;
  }

  if (!analyzing_card) {
    digitalWrite(LED_GREEN, HIGH);
    analyzing_card = true;
    analyzing_card_start = millis();
  }

  if (analyzing_card && (millis() - analyzing_card_start) > 2000) {
    digitalWrite(LED_GREEN, LOW);
    analyzing_card = false;
    tmp = false;
    return CARD_RED;
  }

  return CARD_NONE;
}


void loop(void)
{
  sendCardtoAnalyse();
  card_color = getCardColor();
  if (isCardReadyToAnalyze()) {
    digitalWrite(LED_RED, HIGH);
    tmp = true;
    if (card_color) {
      sendCardToSorting(true);
    }
  } else {
    digitalWrite(LED_RED, LOW);
    sendCardToSorting(false);
    //    card_color = CARD_NONE;
  }
  debugSendCardToSorting();
}


//-------------------------------------
// DEBUG FUNCTIONS
//-------------------------------------
void debugSendCardToSorting(void)
{
  if (digitalRead(BUTTON_B) == LOW) {
    sendCardToSorting(true);
  }
}
