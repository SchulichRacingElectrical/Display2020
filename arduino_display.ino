/***************************************************
  This is our library for the Adafruit HX8357D Breakout
  ----> http://www.adafruit.com/products/2050

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

// These are 'flexible' lines that can be changed
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// SoftSPI - note that on some processors this might be *faster* than hardware SPI!
//Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST, MISO);

uint16_t width = tft.width();
uint16_t height = tft.height();

int buttonState = 0;
int prevButtonState = 0;
const int buttonPin = 2;
int screenMode = 0;

unsigned int previousMillis = 0;
const int interval = 100;

int RPM = 5000;
int prev_RPM = 5000;

int WATER_TEMP = 50;
int prev_WATER_TEMP = 50;

int SPEED = 120;
int prev_SPEED = 120;

int OIL_TEMP = 50;
int prev_OIL_TEMP = 50;

int OIL_PRES = 50;
int prev_OIL_PRES = 50;

void setup() {
  Serial.begin(9600);
  Serial.println("HX8357D Test!");

  tft.begin();

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(HX8357_RDPOWMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDCOLMOD);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDIM);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDSDR);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  Serial.println(F("Benchmark                Time (microseconds)"));

  pinMode(buttonPin, INPUT);

  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);

  Serial.print("Width: 0x"); Serial.println(width, DEC);
  Serial.print("Height: 0x"); Serial.println(height, DEC);

  drawDriverScreen();
}

void loop(void) {
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    if (prevButtonState == 0) {
      tft.fillScreen(HX8357_BLACK);
      switch (screenMode) {
        case 0:
          screenMode = 1;
          drawPowertrainScreen();
          break;
        case 1:
          screenMode = 2;
          drawSuspensionScreen();
          break;
        case 2:
          screenMode = 3;
          drawAlert("holy craaappp");
          break;
        case 3:
          screenMode = 0;
          drawDriverScreen();
          break;
        default:
          break;
      }
    }
    prevButtonState = 1;
  } else {
    prevButtonState = 0;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    switch (screenMode) {
      case 0:
        RPM = abs(RPM + random(-20, 21));
        WATER_TEMP = abs(WATER_TEMP + random(-3, 4));
        SPEED = abs(SPEED + random(-10, 11));
        OIL_TEMP = abs(OIL_TEMP + random(-5, 6));
        OIL_PRES = abs(OIL_PRES + random(-5, 6));
        updateDriverScreen();
        prev_RPM = RPM;
        prev_WATER_TEMP = WATER_TEMP;
        prev_SPEED = SPEED;
        prev_OIL_TEMP = OIL_TEMP;
        prev_OIL_PRES = OIL_PRES;
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
      default:
        break;
    }
  }
}

void replaceNum(int curr, int prev, int x, int y, int numSize) {
  int digitCurr[6];
  int digitPrev[6];
  int og_curr = curr;
  int og_prev = prev;
  int currNumOffset = 0;
  int prevNumOffset = 0;
  int numWidth;
  switch (numSize) {
    case 3:
      numWidth = 18;
      break;
    case 6:
      numWidth = 36;
      break;
    default:
      break;
  }

  tft.setTextSize(numSize);

  //Convert curr num to array
  for (int i = 5; i >= 0; i--) {
    digitCurr[i] = curr % 10;
    curr /= 10;
  }

  //Convert prev num to array
  for (int i = 5; i >= 0; i--) {
    digitPrev[i] = prev % 10;
    prev /= 10;
  }

  bool currDoing = true;
  bool prevDoing = true;
  for(int i = 0;(i < 6); i++) {
    if(digitCurr[i] == 0 && currDoing)
      currNumOffset++;
    else 
      currDoing = false;
    if(digitPrev[i] == 0 && prevDoing)
      prevNumOffset++;
    else 
      prevDoing = false;
  }
//  int currNumOffsetWidth = (currNumOffset) * numWidth;
//  int prevNumOffsetWidth = (prevNumOffset) * numWidth;

  //Proper clearing
  tft.setTextColor(HX8357_BLACK);
  int diff = prevNumOffset - currNumOffset;
  for(int i = 0; i < prevNumOffset; i++) {
    tft.setCursor(x + (i * numWidth), y);
    tft.setTextColor(HX8357_BLACK);
    tft.print(0, DEC);
  }
  
  //Printing
  for(int i = 0; i < 6; i++) {
    Serial.print(digitPrev[i]);
  }
  Serial.print(" -> ");
  for(int i = 0; i < 6; i++) {
    Serial.print(digitCurr[i]);
  }
  Serial.print(" -> ");
  Serial.print(currNumOffset);  
  Serial.print(" -> ");
  Serial.print(prevNumOffset);  
  Serial.println();
  
  //Code to only clear changed number
  for (int i = 0; i < 6; i++) {
    if (digitCurr[i] != digitPrev[i]) {
      tft.setCursor(x + (i * numWidth), y);
      tft.setTextColor(HX8357_BLACK);
      tft.print(digitPrev[i], DEC);
      tft.setCursor(x + (i * numWidth), y);
      tft.setTextColor(HX8357_WHITE);
      tft.print(digitCurr[i], DEC);
    }
  }
}

void updateDriverScreen() {
  replaceNum(RPM, prev_RPM, 120, 200, 6);
  replaceNum(WATER_TEMP, prev_WATER_TEMP, 0, 30, 3);
  replaceNum(SPEED, prev_SPEED, 0, 110, 3);
  replaceNum(OIL_TEMP, prev_OIL_TEMP, 330, 30, 3);
  replaceNum(OIL_PRES, prev_OIL_PRES, 330, 110, 3);

  // textSize(3) => width = 18
  // textSize(6) => width = 36
}

void drawAlert(String message) {
  tft.fillScreen(HX8357_BLACK);
  tft.fillRect(30, 30, 420, 260, HX8357_RED);
  tft.setTextSize(5);
  tft.setCursor(50, 140);
  tft.println(message);
}

void drawDriverScreen() {
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("WATER TEMP");

  tft.setCursor(0, 80);
  tft.println("SPEED");

  tft.setCursor(330, 0);
  tft.println("OIL TEMP");

  tft.setCursor(330, 80);
  tft.println("OIL PRES");

  tft.setTextSize(4);
  tft.setCursor(200, 150);
  tft.println("RPM");
}

void drawPowertrainScreen() {
  tft.setTextSize(2);

  tft.setCursor(0, 0);
  tft.println("WATER TEMP:");

  tft.setCursor(0, 30);
  tft.println("OIL TEMP:");

  tft.setCursor(0, 60);
  tft.println("OIL PRES:");

  tft.setCursor(0, 90);
  tft.println("MAP:");

  tft.setCursor(0, 120);
  tft.println("AFR:");

  tft.setCursor(0, 150);
  tft.println("EXH:");

  tft.setCursor(0, 180);
  tft.println("RPM:");

  tft.setCursor(0, 210);
  tft.println("WATER PRES 1:");

  tft.setCursor(0, 240);
  tft.println("WATER PRES 2:");

  tft.setCursor(0, 270);
  tft.println("FAN 1:");

  tft.setCursor(0, 300);
  tft.println("FAN 2:");

}

void drawSuspensionScreen() {
  tft.setTextSize(3);

  tft.setCursor(0, 0);
  tft.println("RR:");

  tft.setCursor(0, 40);
  tft.println("RL:");

  tft.setCursor(0, 80);
  tft.println("FR:");

  tft.setCursor(0, 120);
  tft.println("FL:");

  tft.setCursor(0, 160);
  tft.println("FB:");

  tft.setCursor(0, 200);
  tft.println("RB:");

  tft.setCursor(0, 240);
  tft.println("FBP:");

  tft.setCursor(0, 280);
  tft.println("STR ANG:");
}
