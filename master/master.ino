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

// CONST
const int SEND_CARD = 1;
const int OPEN_SERVO = 2;

// SERIAL
const long SERIAL_SPEED = 921600; //921600
const int ARDUINO_ADDRESS = 8;
const int SLAVE_ADDRESS = 8;
const uint8_t LCD_ADDRESS = 0x3F;

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

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  intro();
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
unsigned long startcaptureTime;

bool modeAuto = false;
char cardColor = CARD_NONE;
void loop(void)
{
  buttonB();
  buttonC();
  buttonD();

  if (digitalRead(BUTTON_A) == HIGH) {
    modeAuto = false;
    state = Send;
    digitalWrite(LED_GREEN, LOW);
    printLcdLine("Mode: manual", 0);
  } else {
    modeAuto = true;
    digitalWrite(LED_GREEN, HIGH);
    printLcdLine("Mode: auto", 0);
  }

  if (modeAuto) {
    switch (state) {
      case Send:
        {
          cardColor = CARD_NONE;
          sendCard();
          state = Capture;
        }
        break;
      case Capture:
        {
          if (!captureCard()) {
            cardColor = CARD_UNKNOWN;
            state = Sort;
          } else {
            state = Analyze;
          }
        }
        break;
      case Analyze:
        {
          analyzeCard();
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
  delay(1000);
  clearLcdLine(1);
  digitalWrite(LED_WHITE, LOW);
}

bool captureCard(void)
{
  digitalWrite(LED_YELLOW, HIGH);
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
  printLcdLine("Capturing card...", 1);
  bool status = cardCam.singleCapture();
  //bool status = true;
  //delay(1000);
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
void analyzeCard(void)
{
  if (!analyzing) {
    digitalWrite(LED_YELLOW, HIGH);
    analyzing = true;
    clearLcdLine(1);
    clearLcdLine(2);
    clearLcdLine(3);
    printLcdLine("Analyzing card...", 1);
  }
  delay(1000);
  //if (analyzing && Serial.available()) {
    //String color = Serial.readString();
  if (analyzing) {
    char color = "R";
    
    state = Sort;
    
    char cardMsg[21];
    sprintf(cardMsg, "Card is: %c", color);
    printLcdLine(cardMsg, 3);
    clearLcdLine(1);
    analyzing = false;
    digitalWrite(LED_YELLOW, LOW);
  }
}

void sortCard(void)
{
  digitalWrite(LED_BLUE, HIGH);
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
  printLcdLine("Sorting card...", 1);
  sendToSlave(OPEN_SERVO);
  delay(500);
  clearLcdLine(1);
  digitalWrite(LED_BLUE, LOW);
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

void buttonC()
{
  if (digitalRead(BUTTON_C) == LOW) {
    captureCard();
  }
}

void buttonD(void) {
  if (digitalRead(BUTTON_D) == LOW) {
    sortCard();
  }
}
