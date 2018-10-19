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

//LiquidCrystal_I2C lcd(LCD_ADDRESS,20,4);

char line0[21]; 
char line1[21];
char line2[21];
char line3[21];
 

void setup(void)
{
  Wire.begin();
  SPI.begin();

  Serial.begin(SERIAL_SPEED);
  cardCam.beginJpeg800x600();

  //lcd.init();
  //lcd.backlight();

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
  Motor1,
  Motor2,
  Waiting,
  Capture
};
states state = Motor1;

void sendToSlave(byte value)
{
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(value);
  Wire.endTransmission();
}
unsigned long startcaptureTime;

bool modeAuto = false;
void loop(void)
{
  buttonA();
  buttonB();
  buttonC();
  buttonD();

  if (digitalRead(SWITCH) == HIGH) {
    modeAuto = false;
    state = Motor1;
    digitalWrite(LED_GREEN, LOW);
     sprintf(line0, "Mode: %-14s", "manual"); 
  } else {
    modeAuto = true;
    digitalWrite(LED_GREEN, HIGH);
     sprintf(line0, "Mode: %-14s", "auto"); 
  }

  if (modeAuto) {
    switch (state) {
      case Motor1:
        {
     			sprintf(line1, "%-20s", "Sending card..."); 
          sendToSlave(SEND_CARD);
          delay(1000);
          state = Capture;
        }
        break;
      case Capture:
        {
     			sprintf(line1, "%-20s", "Capturing image..."); 
          digitalWrite(LED_YELLOW, HIGH);
          cardCam.singleCapture();
          digitalWrite(LED_YELLOW, LOW);
          state = Motor2;
        }
        break;
      case Waiting:
        {
     			sprintf(line1, "%-20s", "Waiting data..."); 
          state = Motor2;
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
      case Motor2:
        {
     			sprintf(line1, "%-20s", "Sorting data..."); 
          sendToSlave(OPEN_SERVO);
          state = Motor1;
          delay(500);
        }
        break;
    }
  }

	//updateLcd();
 
}

void updateLcd(void)
{
  //lcd.setCursor(0,0);
   //lcd.print(line0);
  //lcd.setCursor(0,1);
   //lcd.print(line1);
 // lcd.setCursor(0,2);
  // lcd.print(line2);
 // lcd.setCursor(0,3);
  // lcd.print(line3);
}

void clearLcdLine(char line)
{
    sprintf(line, "%-20s", " ");
}

// SEND CARD
void buttonA()
{
  if (digitalRead(BUTTON_A) == LOW) {
    sendToSlave(SEND_CARD);
  }
}

// SEND CARD TO SORTING
void buttonB()
{
  if (digitalRead(BUTTON_B) == LOW) {
    sendToSlave(OPEN_SERVO);
  }
}

// TAKE CAMERA PICTURE
bool capturing = false;
void buttonC(void) {
  if (!capturing && digitalRead(BUTTON_C) == LOW) {
    capturing = true;
    digitalWrite(LED_YELLOW, HIGH);
    cardCam.singleCapture();
    digitalWrite(LED_YELLOW, LOW);
    capturing = false;
  }
}

void buttonD(void) {
}