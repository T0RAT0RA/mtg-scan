#include <Wire.h>
#include <BreezyArduCAM.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

// CAMERA
const int CS_CAM = 10;

// BUTTONS
const int BUTTON_A = 7;
const int BUTTON_B = 6;
const int BUTTON_C = 5;
const int BUTTON_D = 4;

// LEDS
const int LED_GREEN = A0;
const int LED_WHITE = A1;
const int LED_YELLOW = A2;
const int LED_BLUE = A3;

// SENSORS
const int SERVO_CARD_SENSOR = 8;

// CONST
const int SEND_CARD = 1;
const int OPEN_SERVO = 2;

// SERIAL
const long SERIAL_SPEED = 921600; //921600
const int ARDUINO_ADDRESS = 8;
const int SLAVE_ADDRESS = 8;
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

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  
  pinMode(SERVO_CARD_SENSOR, INPUT);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  intro();
  
  lcd.clear();
  printLcdLine("Mode: manual", 0);
}

void intro(void){
  int i, j;
  int leds[] = {LED_GREEN, LED_WHITE, LED_YELLOW, LED_BLUE};
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      digitalWrite(leds[i-1], LOW);
    }
    digitalWrite(leds[i], HIGH);
    delay(70);
  }
  for (i = 3; i >= 0 ; i--) {
    if (i < 4) {
      digitalWrite(leds[i+1], LOW);
    }
    digitalWrite(leds[i], HIGH);
    delay(70);
  }
  for (i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(70);
  }
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      digitalWrite(leds[j], HIGH);
    }
    delay(100);
    for (j = 0; j < 4; j++) {
      digitalWrite(leds[j], LOW);
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

void sendToSlave(byte value)
{
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(value);
  Wire.endTransmission();
}

bool modeAuto = false;
char cardColor = CARD_NONE;
void loop(void)
{
  buttonA();
  buttonB();
  buttonC();
  buttonD();
    
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
  digitalWrite(LED_WHITE, HIGH);
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
  printLcdLine("Sending card...", 1);
  sendToSlave(SEND_CARD);
  delay(500);
  clearLcdLine(1);
  digitalWrite(LED_WHITE, LOW);
}

bool isCardReady(void) {
  return digitalRead(SERVO_CARD_SENSOR) == LOW;
}

bool captureCard(void)
{
  digitalWrite(LED_YELLOW, HIGH);
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
  digitalWrite(LED_YELLOW, LOW);

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
    digitalWrite(LED_YELLOW, HIGH);
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
  digitalWrite(LED_YELLOW, LOW);
}

void sortCard(void)
{
  digitalWrite(LED_BLUE, HIGH);
  clearLcdLine(1);
  printLcdLine("Sorting card...", 1);
  sendToSlave(OPEN_SERVO);
  delay(500);
  clearLcdLine(1);
  digitalWrite(LED_BLUE, LOW);
}


void buttonA()
{
  if (modeAuto && digitalRead(BUTTON_A) == HIGH) {
    modeAuto = false;
    state = Send;
    digitalWrite(LED_GREEN, LOW);
    lcd.clear();
    printLcdLine("Mode: manual", 0);
  } else if (!modeAuto && digitalRead(BUTTON_A) == LOW) {
    modeAuto = true;
    digitalWrite(LED_GREEN, HIGH);
    printLcdLine("Mode: auto", 0);
  }
}

void buttonB()
{
  if (digitalRead(BUTTON_B) == LOW) {
    digitalWrite(LED_WHITE, HIGH);
    sendCard();
  } else {
    digitalWrite(LED_WHITE, LOW);
  }
}

bool shouldAnalyse = false;
void buttonC()
{
  if (digitalRead(BUTTON_C) == LOW) {
    captureCard();
    analyzeCard();
  }
}

void buttonD(void) {
  if (digitalRead(BUTTON_D) == LOW) {
    sortCard();
  }
}
