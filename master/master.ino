#include <Wire.h>
#include <BreezyArduCAM.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Servo.h>
#include <LED.h>
#include <Button.h>

// CAMERA
const int CS_CAM = 23;

// BUTTONS
Button buttonA = Button(48, INTERNAL_PULLUP);
Button buttonB = Button(46, INTERNAL_PULLUP);
Button buttonC = Button(44, INTERNAL_PULLUP);
Button buttonD = Button(42, INTERNAL_PULLUP);

// LEDS
LED ledGreen = LED(49);
LED ledWhite = LED(47);
LED ledYellow = LED(45);
LED ledBlue = LED(43);

// SENSORS
const int SERVO_CARD_SENSOR = 8;

Servo cardServo;

// MOTORS
const int MOTOR_CARD_FORWARD = 8;
const int MOTOR_CARD_BACKWARD = 9;
const int MOTOR_CARD_PWM = 10;
const int MOTOR_CARD_FORWARD_TIME = 400; //ms
const int MOTOR_CARD_FORWARD_POWER = 110; //0 to 255
const int MOTOR_CARD_BACKWARD_TIME = 120; //ms
const int MOTOR_CARD_BACKWARD_POWER = 210; //0 to 255
const int SERVO_CARD = 2;
const int SERVO_CLOSE = 60;
const int SERVO_OPEN = 130;
const int SERVO_OPEN_TIME = 300; //ms

// CONST
const int SEND_CARD = 1;
const int OPEN_SERVO = 2;

// SERIAL
const long SERIAL_SPEED = 921600; //921600
const int ARDUINO_ADDRESS = 8;
// const int SLAVE_ADDRESS = 8;
const uint8_t LCD_ADDRESS = 0x3F;
char *SERIAL_DELIMITER = '|';

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

LiquidCrystal_I2C lcd(LCD_ADDRESS,20,4);

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(SERIAL_SPEED);
  cardCam.beginJpeg800x600();

  lcd.init();
  lcd.backlight();

  // Setup pinouts
  pinMode(SERVO_CARD_SENSOR, INPUT);

  pinMode(MOTOR_CARD_FORWARD, OUTPUT);
  pinMode(MOTOR_CARD_BACKWARD, OUTPUT);
  pinMode(MOTOR_CARD_PWM, OUTPUT);
  pinMode(SERVO_CARD, OUTPUT);

  // Setup motor initial position
  cardServo.attach(SERVO_CARD);
  cardServo.write(SERVO_CLOSE);

  // Play the colorful intro
  // TODO: find a non-blocking way to play it
  //       while setting up everything
  intro();

  lcd.clear();
  printLcdLine("Mode: manual", 0);
}

void intro(void){
  int i, j;
  LED leds[] = {ledGreen, ledWhite, ledYellow, ledBlue};
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      leds[i-1].off();
    }
    leds[i].on();
    delay(70);
  }
  for (i = 3; i >= 0 ; i--) {
    if (i < 4) {
      leds[i+1].off();
    }
    leds[i].on();
    delay(70);
  }
  for (i = 0; i < 4; i++) {
    leds[i].on();
    delay(70);
  }

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      leds[j].on();
    }
    delay(100);
    for (j = 0; j < 4; j++) {
      leds[j].off();
    }
    delay(100);
  }
}

enum states {
  None,
  Send,
  Capture,
  Analyze,
  Sort
};
states state = Send;

void printLcdLine(char message[], int line)
{
  char str[21];
  sprintf(str, "%-20s", message);
  lcd.setCursor(0, line);
  lcd.print(str);
}

void clearLcdLine(int line)
{
  lcd.setCursor(0, line);
  lcd.print("                    ");
}

void sendCard(void)
{
  ledWhite.on();
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
  printLcdLine("Sending card...", 1);
  sendCardtoAnalyse();
  delay(500);
  clearLcdLine(1);
  ledWhite.off();
}

bool isCardReady(void) {
  return digitalRead(SERVO_CARD_SENSOR) == LOW;
}

bool captureCard(void)
{
  ledYellow.on();
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
  printLcdLine("Capturing card...", 1);
  bool status = cardCam.singleCapture();
  if (!status) {
    printLcdLine("Capture error!", 2);
  } else {
    printLcdLine("Capture success", 2);
  }
  clearLcdLine(1);
  ledYellow.off();

  return status;
}

bool analyzing = false;
char card[21];
char colors[21];
String serialIn;
unsigned long startAnalyzeTime;
const int ANALYZE_TIMEOUT = 4500;
void analyzeCard(void)
{
  if (!analyzing) {
    ledYellow.on();
    analyzing = true;
    clearLcdLine(1);
    printLcdLine("Analyzing card.", 1);
    card[0] = '\0';
    colors[0] = '\0';
    startAnalyzeTime = millis();
  }

  while (analyzing) {
    printLcdLine("Analyzing card..", 1);
    if (card[0] == '\0' && Serial.available()) {
      printLcdLine("Analyzing card...", 1);
      serialIn = Serial.readStringUntil(SERIAL_DELIMITER);
      serialIn.replace(SERIAL_DELIMITER, "");
      serialIn.toCharArray(card, 21);
      printLcdLine(card, 2);
      clearLcdLine(3);
    } else if (card[0] != '\0' && colors[0] == '\0' && Serial.available()) {
      printLcdLine("Analyzing card....", 1);
      serialIn = Serial.readStringUntil(SERIAL_DELIMITER);
      serialIn.replace(SERIAL_DELIMITER, "");
      serialIn.toCharArray(colors, 21);
      printLcdLine(colors, 3);
      analyzing = false;
    }

    if (millis() - startAnalyzeTime >= ANALYZE_TIMEOUT){
      analyzing = false;
      printLcdLine("Analyze timeout", 2);
    }
  }

  clearLcdLine(1);
  card[0] = '\0';
  colors[0] = '\0';
  ledYellow.off();
}

void sortCard(void)
{
  ledBlue.on();
  clearLcdLine(1);
  printLcdLine("Sorting card...", 1);
  sendCardToSorting();
  delay(500);
  clearLcdLine(1);
  ledBlue.off();
}

bool serving_new_card = false;
bool prepare_new_card = false;
unsigned long serving_card_start = 0;
void sendCardtoAnalyse(void)
{
  if (!serving_new_card && !prepare_new_card) {
    serving_new_card = true;
    serving_card_start = millis();
    analogWrite(MOTOR_CARD_FORWARD, 255);
    analogWrite(MOTOR_CARD_BACKWARD, 0);
    analogWrite(MOTOR_CARD_PWM, MOTOR_CARD_FORWARD_POWER);
  }

  // TODO: remove this delay
  delay(MOTOR_CARD_FORWARD_TIME);

  if (serving_new_card && (millis() - serving_card_start) >= MOTOR_CARD_FORWARD_TIME) {
    serving_new_card = false;
    prepare_new_card = true;
    analogWrite(MOTOR_CARD_FORWARD, 0);
    analogWrite(MOTOR_CARD_BACKWARD, 255);
    analogWrite(MOTOR_CARD_PWM, MOTOR_CARD_BACKWARD_POWER);
  }

  // TODO: remove this delay
  delay(MOTOR_CARD_BACKWARD_TIME);

  if (prepare_new_card && (millis() - serving_card_start) >= MOTOR_CARD_FORWARD_TIME + MOTOR_CARD_BACKWARD_TIME) {
    prepare_new_card = false;
    analogWrite(MOTOR_CARD_FORWARD, 0);
    analogWrite(MOTOR_CARD_BACKWARD, 0);
  }
}

bool sorting_card = false;
unsigned long sorting_card_start = 0;
void sendCardToSorting()
{
  if (!sorting_card) {
    sorting_card = true;
    sorting_card_start = millis();
    cardServo.write(SERVO_OPEN);
  }

  // TODO: remove this delay
  delay(SERVO_OPEN_TIME);

  if (sorting_card && (millis() - sorting_card_start) >= SERVO_OPEN_TIME) {
    sorting_card = false;
    cardServo.write(SERVO_CLOSE);
  }
}


bool modeAuto = false;
char cardColor = CARD_NONE;
void loop(void)
{

  if (modeAuto && !buttonA.isPressed()) {
    modeAuto = false;
    state = Send;
    ledGreen.off();
    lcd.clear();
    printLcdLine("Mode: manual", 0);
  } else if (!modeAuto && buttonA.isPressed()) {
    modeAuto = true;
    ledGreen.on();
    printLcdLine("Mode: auto", 0);
  }

  if (buttonB.isPressed()) {
    ledWhite.on();
    sendCard();
  } else {
    ledWhite.off();
  }

  if (buttonC.isPressed()) {
    captureCard();
    analyzeCard();
  }

  if (buttonD.isPressed()) {
    sortCard();
  }

  if (modeAuto) {
    switch (state) {
      case Send:
        {
          cardColor = CARD_NONE;
          sendCard();
          state = Capture;
          printLcdLine("Waiting card...", 2);
        }
        break;
      case Capture:
        {
          // Start capture only when card is detected
          if (isCardReady()) {
            if (!captureCard()) {
              cardColor = CARD_UNKNOWN;
              state = Sort;
            } else {
              state = Analyze;
            }
          }
        }
        break;
      case Analyze:
        {
            analyzeCard();
            state = Sort;
        }
        break;
      case Sort:
        {
          sortCard();
          state = Send;
        }
        break;
    }
  }

}