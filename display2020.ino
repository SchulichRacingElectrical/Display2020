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

#define SWTICH_BUTTON_PIN 2
#define LAPTIME_BUTTON_PIN 3

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// SoftSPI - note that on some processors this might be *faster* than hardware SPI!
//Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST, MISO);

uint16_t width = tft.height();
uint16_t height = tft.width();

int buttonState = 0;
int prevButtonState = 0;
int screenMode = 0;

long lapMillis = 0;

unsigned int previousMillis = 0;
const int interval = 100;

// Display numbers 
int TIME_MINUTE = 88;
int prev_TIME_MINUTE = 88;

int TIME_SECOND = 88;
int prev_TIME_SECOND = 88;

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

int MAP = 120;
int prev_MAP = 120;

int AFR = 120;
int prev_AFR = 120;

int EXH = 120;
int prev_EXH = 120;

int WATER_PRES_1 = 50;
int prev_WATER_PRES_1 = 50;

int WATER_PRES_2 = 50;
int prev_WATER_PRES_2 = 50;

int FAN_1 = 50;
int prev_FAN_1 = 50;

int FAN_2 = 50;
int prev_FAN_2 = 50;

int RR = 50;
int prev_RR = 50;

int RL = 50;
int prev_RL = 50;

int FR = 50;
int prev_FR = 50;

int FL = 50;
int prev_FL = 50;

int FB = 50;
int prev_FB = 50;

int RB = 50;
int prev_RB = 50;

int FBP = 50;
int prev_FBP = 50;

int STR_ANG = 30;
int prev_STR_ANG = 30;

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

  pinMode(SWTICH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LAPTIME_BUTTON_PIN, INPUT_PULLUP);

  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);

  Serial.print("Width: 0x"); Serial.println(width, DEC);
  Serial.print("Height: 0x"); Serial.println(height, DEC);

  drawDriverScreen();
}

void loop(void) {
  buttonState = digitalRead(SWTICH_BUTTON_PIN);
  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState == LOW) {
    Serial.println(buttonState);
    if (prevButtonState == 0) {
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
    test_change_all_curr();
    switch (screenMode) {
      case 0:
        updateDriverScreen();
        break;
      case 1:
        updatePowertrainScreen();
        break;
      case 2:
        updateSuspensionScreen();
        break;
      case 3:
        break;
      default:
        break;
    }
    test_change_all_prev();
  }
  buttonState = digitalRead(SWTICH_BUTTON_PIN);
  if (digitalRead(SWTICH_BUTTON_PIN) == LOW) {
    
  }
}

void updateDriverScreen() {
  replaceNum(RPM       , prev_RPM       , 120, 200, 6, true);
  replaceNum(WATER_TEMP, prev_WATER_TEMP, 0  , 30 , 3, true);
  replaceNum(SPEED     , prev_SPEED     , 0  , 110, 3, true);
  replaceNum(OIL_TEMP  , prev_OIL_TEMP  , 330, 30 , 3, true);
  replaceNum(OIL_PRES  , prev_OIL_PRES  , 330, 110, 3, true);

  updateTime();
  
  replaceNum(TIME_SECOND, prev_TIME_SECOND, 180, 295, 3, true);
  replaceNum(TIME_MINUTE, prev_TIME_MINUTE, 140, 295, 3, true);
}

void updateTime() {
  long timeNow = millis();
  
  TIME_SECOND = (timeNow) % 60000 / 1000;
  TIME_MINUTE = (timeNow) % 3600000 / 60000;

  Serial.print("Second:");Serial.println(TIME_SECOND, DEC);
  Serial.print("Minute:");Serial.println(TIME_MINUTE, DEC);
}

void updatePowertrainScreen() {
  replaceNum(WATER_TEMP  , prev_WATER_TEMP  , 300, 5,   2, false);
  replaceNum(OIL_TEMP    , prev_OIL_TEMP    , 300, 35,  2, true);
  replaceNum(OIL_PRES    , prev_OIL_PRES    , 300, 65,  2, false);
  replaceNum(MAP         , prev_MAP         , 300, 95,  2, true);
  replaceNum(AFR         , prev_AFR         , 300, 125, 2, false);
  replaceNum(EXH         , prev_EXH         , 300, 155, 2, true);
  replaceNum(RPM         , prev_RPM         , 300, 185, 2, false);
  replaceNum(WATER_PRES_1, prev_WATER_PRES_1, 300, 215, 2, true);
  replaceNum(WATER_PRES_2, prev_WATER_PRES_2, 300, 245, 2, false);
  replaceNum(FAN_1       , prev_FAN_1       , 300, 275, 2, true);
  replaceNum(FAN_2       , prev_FAN_2       , 300, 305, 2, false);
}

void updateSuspensionScreen(){
  replaceNum(RR     , prev_RR     , 300, 10 , 3, false);
  replaceNum(RL     , prev_RL     , 300, 50 , 3, true);
  replaceNum(FL     , prev_FL     , 300, 90 , 3, false);
  replaceNum(FR     , prev_FR     , 300, 130, 3, true);
  replaceNum(RB     , prev_RB     , 300, 170, 3, false);
  replaceNum(FB     , prev_FB     , 300, 210, 3, true);
  replaceNum(FBP    , prev_FBP    , 300, 250, 3, false);
  replaceNum(STR_ANG, prev_STR_ANG, 300, 290, 3, true);
}

void drawAlert(String message) {
  tft.fillScreen(HX8357_BLACK);
  tft.fillScreen(HX8357_BLACK);
  tft.fillRect(30, 30, 420, 260, HX8357_RED);
  tft.setTextSize(5);
  tft.setCursor(50, 140);
  tft.println(message);
}

void drawDriverScreen() {
  tft.fillScreen(HX8357_BLACK);
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

  tft.setCursor(0, 295);
  tft.println("LAPTIME:");

  tft.setTextSize(4);
  tft.setCursor(200, 150);
  tft.println("RPM");
}

void drawPowertrainScreen() {
  tft.setTextSize(2);
  bool isBlack = true;
  for (int i = 0; i < height; i += 30) {
    if (isBlack)
      tft.fillRect(0, i, width, 30, HX8357_WHITE);
    else
      tft.fillRect(0, i, width, 30, HX8357_BLACK);

    isBlack = !isBlack;
  }

  tft.setTextColor(HX8357_BLACK);

  tft.setCursor(5, 5);
  tft.println("WATER TEMP:");

  tft.setCursor(5, 65);
  tft.println("OIL PRES:");

  tft.setCursor(5, 125);
  tft.println("AFR:");

  tft.setCursor(5, 185);
  tft.println("RPM:");

  tft.setCursor(5, 245);
  tft.println("WATER PRES 2:");

  tft.setCursor(5, 302);
  tft.println("FAN 2:");

  tft.setTextColor(HX8357_WHITE);

  tft.setCursor(5, 35);
  tft.println("OIL TEMP:");

  tft.setCursor(5, 95);
  tft.println("MAP:");

  tft.setCursor(5, 155);
  tft.println("EXH:");

  tft.setCursor(5, 215);
  tft.println("WATER PRES 1:");

  tft.setCursor(5, 275);
  tft.println("FAN 1:");

}

void drawSuspensionScreen() {
  tft.setTextSize(3);
  bool isBlack = true;
  for (int i = 0; i < height; i += 40) {
    if (isBlack)
      tft.fillRect(0, i, width, 40, HX8357_WHITE);
    else
      tft.fillRect(0, i, width, 40, HX8357_BLACK);

    isBlack = !isBlack;
  }

  tft.setTextColor(HX8357_BLACK);

  tft.setCursor(5, 10);
  tft.println("RR:");

  tft.setCursor(5, 90);
  tft.println("FR:");

  tft.setCursor(5, 170);
  tft.println("FB:");

  tft.setCursor(5, 250);
  tft.println("FBP:");

  tft.setTextColor(HX8357_WHITE);

  tft.setCursor(5, 50);
  tft.println("RL:");

  tft.setCursor(5, 130);
  tft.println("FL:");

  tft.setCursor(5, 210);
  tft.println("RB:");

  tft.setCursor(5, 290);
  tft.println("STR ANG:");

}


void replaceNum(int curr, int prev, int x, int y, int numSize, int isBlack) {
  int digitCurr[6];
  int digitPrev[6];
  int og_curr = curr;
  int og_prev = prev;
  int currNumOffset = 0;
  int prevNumOffset = 0;
  int numWidth;
  switch (numSize) {
    case 2:
      numWidth = 12;
      break;
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

  //Get proper digit offsets
  bool currDoing = true;
  bool prevDoing = true;
  for (int i = 0; (i < 6); i++) {
    if (digitCurr[i] == 0 && currDoing)
      currNumOffset++;
    else
      currDoing = false;
    if (digitPrev[i] == 0 && prevDoing)
      prevNumOffset++;
    else
      prevDoing = false;
  }

  //Proper clearing setup
  if (isBlack)
    tft.setTextColor(HX8357_BLACK);
  else
    tft.setTextColor(HX8357_WHITE);
  int diff = prevNumOffset - currNumOffset;
  for (int i = 0; i < prevNumOffset; i++) {
    tft.setCursor(x + (i * numWidth), y);
    tft.print(0, DEC);
  }

  //Printing
  //  for(int i = 0; i < 6; i++) {
  //    Serial.print(digitPrev[i]);
  //  }
  //  Serial.print(" -> ");
  //  for(int i = 0; i < 6; i++) {
  //    Serial.print(digitCurr[i]);
  //  }
  //  Serial.print(" -> ");
  //  Serial.print(currNumOffset);
  //  Serial.print(" -> ");
  //  Serial.print(prevNumOffset);
  //  Serial.println();

  //Code to only clear changed number
  for (int i = 0; i < 6; i++) {
    if (digitCurr[i] != digitPrev[i]) {
      tft.setCursor(x + (i * numWidth), y);
      if (isBlack)
        tft.setTextColor(HX8357_BLACK);
      else
        tft.setTextColor(HX8357_WHITE);
      tft.print(digitPrev[i], DEC);
      tft.setCursor(x + (i * numWidth), y);
      if (isBlack)
        tft.setTextColor(HX8357_WHITE);
      else
        tft.setTextColor(HX8357_BLACK);
      tft.print(digitCurr[i], DEC);
    }
  }
}

// Screen test functions
void test_change_all_curr() {
  RPM = abs(RPM + random(-20, 21));
  WATER_TEMP = abs(WATER_TEMP + random(-3, 4));
  SPEED = abs(SPEED + random(-10, 11));
  OIL_TEMP = abs(OIL_TEMP + random(-5, 6));
  OIL_PRES = abs(OIL_PRES + random(-5, 6));
  MAP = abs(MAP + random(-8, 9));
  AFR = abs(AFR + random(-8, 9));
  EXH = abs(EXH + random(-8, 9));
  WATER_PRES_1 = abs(WATER_PRES_1 + random(-3, 4));
  WATER_PRES_2 = abs(WATER_PRES_2 + random(-3, 4));
  FAN_1 = abs(FAN_1 + random(-3, 4));
  FAN_2 = abs(FAN_2 + random(-3, 4));
  RR = abs(RR + random(-3, 4));
  RL = abs(RL + random(-3, 4));
  FR = abs(FR + random(-3, 4));
  FL = abs(FL + random(-3, 4));
  FB = abs(FB + random(-3, 4));
  RB = abs(RB + random(-3, 4));
  FBP = abs(FBP + random(-5, 6));
  STR_ANG = abs(STR_ANG + random(-3, 4));
}

void test_change_all_prev() {
  prev_RPM = RPM;
  prev_WATER_TEMP = WATER_TEMP;
  prev_SPEED = SPEED;
  prev_OIL_TEMP = OIL_TEMP;
  prev_OIL_PRES = OIL_PRES;
  prev_MAP = MAP;
  prev_AFR = AFR;
  prev_EXH = EXH;
  prev_WATER_PRES_1 = WATER_PRES_1;
  prev_WATER_PRES_2 = WATER_PRES_2;
  prev_FAN_1 = FAN_1;
  prev_FAN_2 = FAN_2;
  prev_RR = RR;
  prev_RL = RL;
  prev_FR = FR;
  prev_FL = FL;
  prev_FB = FB;
  prev_RB = RB;
  prev_FBP = FBP;
  prev_STR_ANG = STR_ANG;
  prev_TIME_SECOND = TIME_SECOND;
  prev_TIME_MINUTE = TIME_MINUTE;
}
