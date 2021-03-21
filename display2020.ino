#include <SPI.h>
#include <mcp_can.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"

/* GENERAL COMMENTS
 * 
 * Using the 3.5" 320x480 display
 * 
 * TODO: Make the limits for warnings nice
 * 
 * Known Errors: Digits are on a blackground because when digits changed a black version is drawn over the previous
   *             Fixing this should also make the warning transistions smoother for some values
 * 
 */

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
//const int SPI_CS_PIN = 10;

//MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

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
#define YELLOW   0xF6A2
#define WHITE    0xFFFF

#define SWTICH_BUTTON_PIN 2
#define LAPTIME_BUTTON_PIN 3

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

//CAN bus parameters
unsigned char len = 0;
unsigned char buf[8];
unsigned long canId;

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

int BATT_VOLT = 50;       //Change to include 1 decimal place
int prev_BATT_VOLT = 0;  //Change to include 1 decimal place

bool WATER_FAN_L = false;
bool WATER_FAN_R = false;

bool FAN_L = false;
bool FAN_R = false;

bool FUEL_PUMP = false;

char GEAR = 'N';
char prev_GEAR = '0';

int RPM = 5000;
int prev_RPM = 0;

int WATER_TEMP = 90;
int prev_WATER_TEMP = 0;

int SPEED = 50;
int prev_SPEED = 0;

int OIL_TEMP = 90;
int prev_OIL_TEMP = 0;

int OIL_PRES = 50;
int prev_OIL_PRES = 0;

int MAP = 120;
int prev_MAP = 0;

int AFR = 120;
int prev_AFR = 0;

int EXH_TEMP = 120;
int prev_EXH_TEMP = 0;

int WATER_PRES_1 = 50;
int prev_WATER_PRES_1 = 0;

int WATER_PRES_2 = 50;
int prev_WATER_PRES_2 = 0;

int FAN_1 = 50;
int prev_FAN_1 = 0;

int FAN_2 = 50;
int prev_FAN_2 = 0;

int RR = 50;
int prev_RR = 0;

int RL = 50;
int prev_RL = 0;

int FR = 50;
int prev_FR = 0;

int FL = 50;
int prev_FL = 0;

int FB = 50;
int prev_FB = 0;

int RB = 50;
int prev_RB = 0;

int FBP = 50;
int prev_FBP = 0;

int STR_ANG = 30;
int prev_STR_ANG = 999;

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

  //Serial.println(F("Benchmark                Time (microseconds)"));

  pinMode(SWTICH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LAPTIME_BUTTON_PIN, INPUT_PULLUP);

  tft.setRotation(1);
  tft.fillScreen(HX8357_BLACK);

  Serial.print("Width: 0x"); Serial.println(width, DEC);
  Serial.print("Height: 0x"); Serial.println(height, DEC);

  drawDriverScreen();

  //CANinit();
}

void loop(void) {
  //CANrecieve();
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

/*
void CANinit() {
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}
*/

/*
void CANrecieve() {
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        canId = CAN.getCanId();
        
        Serial.println("-----------------------------");
        Serial.print("Get data from ID: 0x");
        Serial.println(canId, HEX);

        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }
        Serial.println();
    }
}
*/

void drawDivides(){
  tft.drawLine(160, 0, 160, 320, WHITE); //Vertical Line Left
  tft.drawLine(320, 0, 320, 320, WHITE); //Vertical Line Right

  /* Horizontal Lines Numbering
   *  1   1   1  
   *      4
   *  2       5
   *  3       6
   */
  tft.drawLine(0, 100, 480, 100, WHITE); //Horizontal Line 1
  tft.drawLine(0, 170, 160, 170, WHITE); //Horizontal Line 2
  tft.drawLine(0, 240, 160, 240, WHITE); //Horizontal Line 3
  tft.drawLine(160, 170, 320, 170, WHITE); //Horizontal Line 4
  tft.drawLine(320, 170, 480, 170, WHITE); //Horizontal Line 5
  tft.drawLine(320, 240, 480, 240, WHITE); //Horizontal Line 6
}

bool OIL_TEMP_soft_warning_cleared = false;
bool OIL_TEMP_soft_warning_drawn = false;
bool OIL_TEMP_hard_warning_cleared = true;
bool OIL_TEMP_hard_warning_drawn = false;

bool WATER_TEMP_soft_warning_cleared = false;
bool WATER_TEMP_soft_warning_drawn = false;
bool WATER_TEMP_hard_warning_cleared = true;
bool WATER_TEMP_hard_warning_drawn = false;

bool BATT_VOLT_soft_warning_cleared = false;
bool BATT_VOLT_soft_warning_drawn = false;
bool BATT_VOLT_hard_warning_cleared = true;
bool BATT_VOLT_hard_warning_drawn = false;

bool OIL_PRES_soft_warning_cleared = false;
bool OIL_PRES_soft_warning_drawn = false;
bool OIL_PRES_hard_warning_cleared = true;
bool OIL_PRES_hard_warning_drawn = false;

bool EXH_TEMP_soft_warning_cleared = false;
bool EXH_TEMP_soft_warning_drawn = false;
bool EXH_TEMP_hard_warning_cleared = true;
bool EXH_TEMP_hard_warning_drawn = false;

void warningChecks() {
  //These are to set a flag if the parameter is out of limits

  //Oil Temperature
  if(OIL_TEMP > 110 && OIL_TEMP < 120)
  {OIL_TEMP_soft_warning_cleared = false;}
  else if(OIL_TEMP > 120)
  {OIL_TEMP_hard_warning_cleared = false;}
  else
  {
    OIL_TEMP_soft_warning_cleared = true;
    OIL_TEMP_hard_warning_cleared = true;
  }

  //Water Temperature
  if(WATER_TEMP > 100 && WATER_TEMP < 105)    //Note: Tune here
  {WATER_TEMP_soft_warning_cleared = false;}
  else if(WATER_TEMP > 105)                   //Note: Tune here
  {WATER_TEMP_hard_warning_cleared = false;}
  else
  {
    WATER_TEMP_soft_warning_cleared = true;
    WATER_TEMP_hard_warning_cleared = true;
  }

  //Battery Voltage
  //TODO: Change these after the decimal is implemented
  if(BATT_VOLT > 119 && BATT_VOLT < 125)    //Note: Tune here
  {BATT_VOLT_soft_warning_cleared = false;}
  else if(BATT_VOLT < 119)                   //Note: Tune here
  {BATT_VOLT_hard_warning_cleared = false;}
  else
  {
    BATT_VOLT_soft_warning_cleared = true;
    BATT_VOLT_hard_warning_cleared = true;
  }

  //Oil Temperature Graphics
  if(!OIL_TEMP_soft_warning_cleared && !OIL_TEMP_soft_warning_drawn)
  {
    tft.fillRect(0,0, 50, 100, YELLOW);
    tft.fillRect(0,0, 125, 25, YELLOW);
    tft.fillRect(110,0, 50, 100, YELLOW);
    tft.fillRect(0,60, 125, 40, YELLOW);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, 0);
    tft.println("OIL TEMP");
    OIL_TEMP_soft_warning_drawn = true;
  }
  else if(!OIL_TEMP_hard_warning_cleared && !OIL_TEMP_hard_warning_drawn)
  {
    tft.fillRect(0,0, 50, 100, RED);
    tft.fillRect(0,0, 125, 25, RED);
    tft.fillRect(110,0, 50, 100, RED);
    tft.fillRect(0,60, 125, 40, RED);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, 0);
    tft.println("OIL TEMP");
    OIL_TEMP_hard_warning_drawn = true;
  }
  else if(OIL_TEMP_soft_warning_cleared && OIL_TEMP_soft_warning_drawn)
  {
    tft.fillRect(0,0, 50, 100, BLACK);
    tft.fillRect(0,0, 125, 25, BLACK);
    tft.fillRect(110,0, 50, 100, BLACK);
    tft.fillRect(0,60, 125, 40, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, 0);
    tft.println("OIL TEMP");
    OIL_TEMP_soft_warning_drawn = false;
  }
  else if(OIL_TEMP_hard_warning_cleared && OIL_TEMP_hard_warning_drawn)
  {
    tft.fillRect(0,0, 50, 100, BLACK);
    tft.fillRect(0,0, 125, 25, BLACK);
    tft.fillRect(110,0, 50, 100, BLACK);
    tft.fillRect(0,60, 125, 40, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, 0);
    tft.println("OIL TEMP");
    OIL_TEMP_hard_warning_drawn = false;
  }

  //Water Temperature Graphics
  if(!WATER_TEMP_soft_warning_cleared && !WATER_TEMP_soft_warning_drawn)
  {
    tft.fillRect(320, 0, 160, 25, YELLOW);
    tft.fillRect(320, 0, 60, 100, YELLOW);
    tft.fillRect(320, 60, 160, 40, YELLOW);
    tft.fillRect(440, 0, 40, 100, YELLOW);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(330, 0);
    tft.println("WTR TEMP");
    WATER_TEMP_soft_warning_drawn = true;
  }
  else if(!WATER_TEMP_hard_warning_cleared && !WATER_TEMP_hard_warning_drawn)
  {
    tft.fillRect(320, 0, 160, 25, RED);
    tft.fillRect(320, 0, 60, 100, RED);
    tft.fillRect(320, 60, 160, 40, RED);
    tft.fillRect(440, 0, 40, 100, RED);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(330, 0);
    tft.println("WTR TEMP");
    WATER_TEMP_hard_warning_drawn = true;
  }
  else if(WATER_TEMP_soft_warning_cleared && WATER_TEMP_soft_warning_drawn)
  {
    tft.fillRect(320, 0, 160, 25, BLACK);
    tft.fillRect(320, 0, 60, 100, BLACK);
    tft.fillRect(320, 60, 160, 40, BLACK);
    tft.fillRect(440, 0, 40, 100, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(330, 0);
    tft.println("WTR TEMP");
    WATER_TEMP_soft_warning_drawn = false;
  }
  else if(WATER_TEMP_hard_warning_cleared && WATER_TEMP_hard_warning_drawn)
  {
    tft.fillRect(320, 0, 160, 25, BLACK);
    tft.fillRect(320, 0, 60, 100, BLACK);
    tft.fillRect(320, 60, 160, 40, BLACK);
    tft.fillRect(440, 0, 40, 100, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(3);
    tft.setCursor(330, 0);
    tft.println("WTR TEMP");
    WATER_TEMP_hard_warning_drawn = false;
  }

  //Battery Voltage Graphics
  if(!BATT_VOLT_soft_warning_cleared && !BATT_VOLT_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, YELLOW);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_soft_warning_drawn = true;
  }
  else if(!BATT_VOLT_hard_warning_cleared && !BATT_VOLT_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, RED);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_hard_warning_drawn = true;
  }
  else if(BATT_VOLT_soft_warning_cleared && BATT_VOLT_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_soft_warning_drawn = false;
  }
  else if(BATT_VOLT_hard_warning_cleared && BATT_VOLT_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_hard_warning_drawn = false;
  }

  //Oil Pressure Graphics
  //TODO: Change these values
  if(!OIL_PRES_soft_warning_cleared && !OIL_PRES_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, YELLOW);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    OIL_PRES_soft_warning_drawn = true;
  }
  else if(!OIL_PRES_hard_warning_cleared && !OIL_PRES_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, RED);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    OIL_PRES_hard_warning_drawn = true;
  }
  else if(OIL_PRES_soft_warning_cleared && OIL_PRES_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    OIL_PRES_soft_warning_drawn = false;
  }
  else if(OIL_PRES_hard_warning_cleared && OIL_PRES_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    OIL_PRES_hard_warning_drawn = false;
  }

  //Exhaust Temperature Graphics
  //TODO: Change these values
  if(!BATT_VOLT_soft_warning_cleared && !BATT_VOLT_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, YELLOW);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_soft_warning_drawn = true;
  }
  else if(!BATT_VOLT_hard_warning_cleared && !BATT_VOLT_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, RED);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_hard_warning_drawn = true;
  }
  else if(BATT_VOLT_soft_warning_cleared && BATT_VOLT_soft_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_soft_warning_drawn = false;
  }
  else if(BATT_VOLT_hard_warning_cleared && BATT_VOLT_hard_warning_drawn)
  {
    tft.fillRect(0, 100, 70, 70, BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 125);
    tft.println("BATT");
    BATT_VOLT_hard_warning_drawn = false;
  }
  
  //Water Fans Graphics
  if(WATER_FAN_L)
  {tft.fillRect(395, 135, 20, 20, GREEN);}
  else
  {tft.fillRect(395, 135, 20, 20, RED);}
  if(WATER_FAN_R)
  {tft.fillRect(443, 135, 20, 20, GREEN);}
  else
  {tft.fillRect(443, 135, 20, 20, RED);}

  //Fans Graphics
  if(FAN_L)
  {tft.fillRect(395, 275, 20, 20, GREEN);}
  else
  {tft.fillRect(395, 275, 20, 20, RED);}
  if(FAN_R)
  {tft.fillRect(443, 275, 20, 20, GREEN);}
  else
  {tft.fillRect(443, 275, 20, 20, RED);}

  //Fuel Pump Graphics
  if(FUEL_PUMP)
  {tft.fillRect(395, 185, 45, 45, GREEN);}
  else
  {tft.fillRect(395, 185, 45, 45, RED);}
}

void updateDriverScreen() {
  /*
   * CHECKLIST
   * 
   * Make sure the values are associated with the correct labels. The numbers update and the
   * labels are in different functions.
   */

  
  drawDivides();
  warningChecks();
  
  //Left Column
  replaceNum(OIL_TEMP  , prev_OIL_TEMP  , 0  , 30 , 3, true);
  replaceNum(RPM     , prev_RPM       , 100, 118, 6, true); //FIX: Problem with the RPM Numbers
  replaceNum(BATT_VOLT , prev_BATT_VOLT , 20  , 120, 3, true); //Change to include the decimal place
  replaceNum(OIL_PRES  , prev_OIL_PRES  , 20  , 190, 3, true);
  replaceNum(EXH_TEMP  , prev_EXH_TEMP  , 20  , 270, 3, true);

  //Right Column
  replaceNum(WATER_TEMP, prev_WATER_TEMP, 330, 30 , 3, true);
  //replaceNum(SPEED     , prev_SPEED     , 0  , 110, 3, true);
  //replaceNum(OIL_PRES  , prev_OIL_PRES  , 330, 110, 3, true);
  replaceChar(GEAR     , prev_GEAR      , 210, 0  , 12);
  

  updateTime();
  
  //replaceNum(TIME_SECOND, prev_TIME_SECOND, 180, 295, 3, true);
  //replaceNum(TIME_MINUTE, prev_TIME_MINUTE, 140, 295, 3, true);
}

void updateTime() {
  long timeNow = millis();
  
  TIME_SECOND = (timeNow) % 60000 / 1000;
  TIME_MINUTE = (timeNow) % 3600000 / 60000;

  //Serial.print("Second:");Serial.println(TIME_SECOND, DEC);
  //Serial.print("Minute:");Serial.println(TIME_MINUTE, DEC);
}

void updatePowertrainScreen() {
  replaceNum(WATER_TEMP  , prev_WATER_TEMP  , 300, 5,   2, false);
  replaceNum(OIL_TEMP    , prev_OIL_TEMP    , 300, 35,  2, true);
  replaceNum(OIL_PRES    , prev_OIL_PRES    , 300, 65,  2, false);
  replaceNum(MAP         , prev_MAP         , 300, 95,  2, true);
  replaceNum(AFR         , prev_AFR         , 300, 125, 2, false);
  replaceNum(EXH_TEMP    , prev_EXH_TEMP    , 300, 155, 2, true);
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
  tft.println("OIL TEMP");

  tft.setCursor(330, 0);
  tft.println("WTR TEMP");

  //Second Row
  tft.setTextSize(2);
  tft.setCursor(0, 125);
  tft.println("BATT");

  tft.setCursor(330, 110);
  tft.println("WTR");
  tft.setCursor(330, 130);
  tft.println("PUMP");
  tft.setCursor(400, 110);
  tft.println("L   R");

  //Third Row
  tft.setCursor(0, 180);
  tft.println("OIL");
  tft.setCursor(0, 200);
  tft.println("PRES");
  
  tft.setCursor(330, 180);
  tft.println("FUEL");
  tft.setCursor(330, 200);
  tft.println("PUMP");

  //Fourth Row
  tft.setCursor(0, 270);
  tft.println("EXHT");
  tft.setCursor(0, 290);
  tft.println("TEMP");
  
  tft.setCursor(330, 280);
  tft.println("FAN");
  tft.setCursor(400, 250);
  tft.println("L   R");


  //IGNORE BELOW FOR NOW
  //tft.setCursor(0, 80);
  //tft.println("SPEED");

  //tft.setCursor(330, 80);
  //tft.println("OIL PRES");

  //tft.setCursor(0, 295);
  //tft.println("LAPTIME:");

  //tft.setTextSize(4);
  //tft.setCursor(200, 150);
  //tft.println("RPM");
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

void replaceChar(char curr, char prev, uint16_t x, uint16_t y, uint16_t fontSize){
  if(curr != prev)
  {
    tft.drawChar(x, y, curr, WHITE, BLACK, fontSize);
  }
}

void replaceNum(int curr, int prev, int x, int y, int numSize, int isBlack) {
  //FIX: Different font sizes that aren't 3 or 6 are bunched together
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
    case 4:
      numWidth = 24;
      break;
    case 5:
      numWidth = 30;
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
    //tft.print(0, DEC);
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
  int OIL_TEMP_value = 1;
  int WATER_TEMP_value = 1;
  int BATT_VOLT_value = 1;
  int EXH_TEMP_value = 1;
  
void test_change_all_curr() {
  RPM = abs(RPM + random(-20, 21));
  //WATER_TEMP = abs(WATER_TEMP + random(-3, 4));

  if(WATER_TEMP > 120){
    WATER_TEMP_value = -1;
    WATER_FAN_L = true;
    WATER_FAN_R = false;
    FAN_L = false;
    FAN_R = true;
    FUEL_PUMP = false;
    }
  else if(WATER_TEMP < 90){
    WATER_TEMP_value =  1;
    WATER_FAN_L = false;
    WATER_FAN_R = true;
    FAN_L = true;
    FAN_R = false;
    FUEL_PUMP = true;
    }
  WATER_TEMP = WATER_TEMP + WATER_TEMP_value;

  if(BATT_VOLT > 130){
    BATT_VOLT_value = -1;}
  else if(BATT_VOLT < 100){
    BATT_VOLT_value =  1;}
  BATT_VOLT = BATT_VOLT + BATT_VOLT_value;
  
  SPEED = abs(SPEED + random(-10, 11));
  //OIL_TEMP = abs(OIL_TEMP + random(-5, 6));

  if(OIL_TEMP > 150){
    OIL_TEMP_value = -1;}
  else if(OIL_TEMP < 90){
    OIL_TEMP_value =  1;}
  OIL_TEMP = OIL_TEMP + OIL_TEMP_value;
  
  OIL_PRES = abs(OIL_PRES + random(-5, 6));  
  MAP = abs(MAP + random(-8, 9));
  AFR = abs(AFR + random(-8, 9));
  //EXH_TEMP = abs(EXH_TEMP + random(-8, 9));

  if(EXH_TEMP > 150){
    EXH_TEMP_value = -1;}
  else if(EXH_TEMP < 90){
    EXH_TEMP_value =  1;}
  EXH_TEMP = EXH_TEMP + EXH_TEMP_value;
  
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
  prev_BATT_VOLT = BATT_VOLT;
  prev_OIL_TEMP = OIL_TEMP;
  prev_OIL_PRES = OIL_PRES;
  prev_MAP = MAP;
  prev_AFR = AFR;
  prev_EXH_TEMP = EXH_TEMP;
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
