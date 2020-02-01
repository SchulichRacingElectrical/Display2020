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

int buttonState = 0;
int prevButtonState = 0;
const int buttonPin = 2;
int screenMode = 0;

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

  //drawDriverScreen();
  //drawPowertrainScreen();
  //drawSuspensionScreen();
}


void loop(void) {
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    if (prevButtonState == 0) {
      switch (screenMode) {
        case 0:
          screenMode = 1;
          break;
        case 1:
          screenMode = 2;
          break;
        case 2:
          screenMode = 0;
          break;
        default:
          break;
      }
      tft.fillScreen(HX8357_BLACK);
    }
    prevButtonState = 1;
  } else {
    prevButtonState = 0;
  }

  Serial.print("Mode: 0x"); Serial.println(screenMode, HEX);

  switch (screenMode) {
    case 0:
      drawDriverScreen();
      break;
    case 1:
      drawPowertrainScreen();
      break;
    case 2:
      drawSuspensionScreen();
      break;
    default:
      break;
  }
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

  tft.setTextSize(6);
  tft.setCursor(160, 200);
  tft.println("12345");

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
