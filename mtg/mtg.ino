#include <Servo.h>

Servo cardServo;

// MOTORS
const int MOTOR_CARD = 5;
const int MOTOR_CARD_TIME = 250; //ms
const int SERVO_CARD = 2;
const int SERVO_CLOSE = 60;
const int SERVO_OPEN = 130;
const int SERVO_OPEN_TIME = 300; //ms

// BUTTONS
const int BUTTON_A = 7;
const int BUTTON_B = 8;

// SENSORS
const int CARD_HOLDER_SENSOR = 6;

// LEDS
const int LED_RED = 4;
const int LED_GREEN = 3;

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

// SERIAL CONSTS
const long SERIAL_SPEED = 921600; //921600
const int CARD_DETECTED = 42;
const int CARD_ANALYSED = 43;


void setup(void)
{
  Serial.begin(SERIAL_SPEED);

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
int getCardColor(bool run)
{
  if (!run && !analyzing_card) {
    return CARD_NONE;
  }

  if (!analyzing_card) {
    digitalWrite(LED_GREEN, HIGH);
    analyzing_card = true;
    Serial.write(CARD_DETECTED);
    analyzing_card_start = millis();
  }

  if (analyzing_card && Serial.available()) {
    card_color = Serial.read();
    digitalWrite(LED_GREEN, LOW);
    analyzing_card = false;
    return card_color;
  }

  return CARD_NONE;
}

bool card_ready = false;
byte prevbyte;
byte currbyte;
void loop(void)
{
  //  sendCardtoAnalyse();
  //  card_ready = isCardReadyToAnalyze();
  //  card_color = getCardColor(card_ready);
  //  if (card_ready) {
  //    digitalWrite(LED_RED, HIGH);
  //    if (card_color) {
  //      sendCardToSorting(true);
  //    }
  //  } else {
  //    digitalWrite(LED_RED, LOW);
  //    sendCardToSorting(false);
  //    //    card_color = CARD_NONE;
  //  }
  //  debugSendCardToSorting();

  if (Serial.available()) {
    currbyte = Serial.peek();

    if (prevbyte == 43) {
      prevbyte = Serial.read();
      currbyte = Serial.read();
      Serial.print("I received: ");
      Serial.println(currbyte, DEC);
      Serial.print("prev:");
      Serial.println(prevbyte, DEC);
      Serial.read(); // clear prevbyte
    }
  }
  prevbyte = currbyte;

  if (!card_ready && digitalRead(BUTTON_B) == LOW) {
    Serial.write(42);
    card_ready = true;
  }
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
