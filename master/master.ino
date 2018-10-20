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
const int SWITCH = 3;

// LEDS
const int LED_GREEN = 9;
const int LED_YELLOW = 8;

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
  pinMode(SWITCH, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
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
  buttonA();
  buttonB();
  buttonC();
  buttonD();

  if (digitalRead(SWITCH) == HIGH) {
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
          state = Sort;
          //if (Serial.available()) {
          //  char color = Serial.read();
          //  if (color == 'G') {
          //    state = Motor2;
          //  } else {
          //    state = Motor1;
          //  }
          //}
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
 
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
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
	printLcdLine("Sending card...", 1);
  sendToSlave(SEND_CARD);
	delay(1000);
  clearLcdLine(1);
}

bool captureCard(void)
{
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
	printLcdLine("Capturing card...", 1);
  digitalWrite(LED_YELLOW, HIGH);
  bool status = cardCam.singleCapture();
  digitalWrite(LED_YELLOW, LOW);
  if (!status) {
	  printLcdLine("Capture error!", 2);
  } else {
	  printLcdLine("Capture success", 2);
  }
  clearLcdLine(1);

  return status;
}

void analyzeCard(void)
{
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
	printLcdLine("Analyzing card...", 1);
	delay(2000);
  clearLcdLine(1);
}

void sortCard(void)
{
  clearLcdLine(1);
  clearLcdLine(2);
  clearLcdLine(3);
	printLcdLine("Sorting card...", 1);
  char cardMsg[21];
  sprintf(cardMsg, "Card is: %c", cardColor);
	printLcdLine(cardMsg, 3);
  sendToSlave(OPEN_SERVO);
  delay(500);
  clearLcdLine(1);
}


void buttonA()
{
  if (digitalRead(BUTTON_A) == LOW) {
    sendCard();
  }
}

void buttonB()
{
  if (digitalRead(BUTTON_B) == LOW) {
    captureCard();
  }
}

void buttonC(void) {
  if (digitalRead(BUTTON_C) == LOW) {
    sortCard();
  }
}

void buttonD(void) {
}