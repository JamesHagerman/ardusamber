#include "Ardusamber.h"
#include "Bun.h"
#include "TimeLib.h"
#include "TFT_ILI9163C.h"

#define CLOCK_X 14
#define CLOCK_Y 6
#define CLOCK_SIZE 100
#define CLOCK_LINE_THICKNESS 1
#define TEXT_X 25
#define TEXT_MARGIN_Y 8

//#define DEBUG 0 // Uncomment this line to enable debug mode

// DEBUG COLORS
#define WHITE           0xFFFF      /* 255, 255, 255 */
#define LIGHTGREY       0xC618      /* 192, 192, 192 */
#define DARKGREY        0x7BEF      /* 128, 128, 128 */
#define BLACK           0x0000      /*   0,   0,   0 */
#define NAVY            0x000F      /*   0,   0, 128 */
#define BLUE            0x001F      /*   0,   0, 255 */
#define DARKCYAN        0x03EF      /*   0, 128, 128 */
#define CYAN            0x07FF      /*   0, 255, 255 */
#define GREEN           0x07E0      /*   0, 255,   0 */
#define DARKGREEN       0x03E0      /*   0, 128,   0 */
#define OLIVE           0x7BE0      /* 128, 128,   0 */
#define GREENYELLOW     0xAFE5      /* 173, 255,  47 */
#define YELLOW          0xFFE0      /* 255, 255,   0 */
#define ORANGE          0xFD20      /* 255, 165,   0 */
#define RED             0xF800      /* 255,   0,   0 */
#define MAROON          0x7800      /* 128,   0,   0 */
#define MAGENTA         0xF81F      /* 255,   0, 255 */
#define PURPLE          0x780F      /* 128,   0, 128 */

#ifndef DEBUG
#define COLOR_ONE_CUR WHITE
#define COLOR_ONE_PREV BLACK
#define COLOR_TWO_CUR WHITE
#define COLOR_TWO_PREV BLACK
#define COLOR_THREE_CUR WHITE
#define COLOR_THREE_PREV BLACK
#else
#define COLOR_ONE_CUR BLUE
#define COLOR_ONE_PREV NAVY
#define COLOR_TWO_CUR GREEN
#define COLOR_TWO_PREV DARKGREEN
#define COLOR_THREE_CUR RED
#define COLOR_THREE_PREV MAROON
#endif

Ardusamber _dTime;
TFT_ILI9163C _tft = TFT_ILI9163C(10, 8, 9);
Bun _buttonBeatTwo(2);
Bun _buttonBeatThree(3);
Bun _buttonPulseOne(4);

const int LINE_LENGTH = CLOCK_SIZE - CLOCK_LINE_THICKNESS;

float _onePerc;
float _twoPerc;
float _threePerc;

// 0 xCur, 1 yCur, 2 lCur, 3 xPrev, 4 yPrev, 5 lPrev
int _one[6];
int _two[6];
int _three[6];

void setup()
{
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  _one[0] = CLOCK_LINE_THICKNESS; // xCur
  _one[1] = CLOCK_LINE_THICKNESS; // yCur
  _one[4] = CLOCK_LINE_THICKNESS; // yPrev
  
  setTime(18, 32, 00, 3, 6, 2018);

  _tft.begin();
  _tft.fillScreen(BLACK);
  _tft.setTextColor(WHITE,BLACK);
  _tft.setTextSize(1);

  drawContainer();
}

void loop()
{
  // 24h*60m*60s = 86400 total seconds per day
  // beat1  = 8640 s
  // beat2  =  864 s
  // beat3  =   86.4 s
  // pulse1 =    8.64 s
  // pulse2 =    0.864 s
  // pulse3 =    0.0864 s
  
  if(_buttonBeatTwo.isPressed()) 
  {
    adjustTime(864);
    // HACK: Line erasing issues happen with the clock rendering when time is changed drastically like this, haven't worked out why yet.
    // Bug 1) beat lines/pluse tips are often left behind un-erased.
    // Bug 2) beat three renders and erases bottom right of container line at 999:999
    _tft.fillRect(CLOCK_X + CLOCK_LINE_THICKNESS, CLOCK_Y + CLOCK_LINE_THICKNESS, CLOCK_SIZE - CLOCK_LINE_THICKNESS, CLOCK_SIZE - CLOCK_LINE_THICKNESS, BLACK); // Hack fix to clean up missed lines
    drawContainer(); // Hack fix to redraw bottom container line
  }
  if(_buttonBeatThree.isPressed()) 
  {
    adjustTime(86.4);
  }
  if(_buttonPulseOne.isPressed()) 
  {
    adjustTime(8.64);
  }

  _dTime.update();

  drawClock(_dTime.getTime());

  // TEXT
  _tft.setCursor(TEXT_X, CLOCK_Y + CLOCK_SIZE + TEXT_MARGIN_Y);
  _tft.println(_dTime.getFormattedDate() + " " + _dTime.getFormattedTime());
  
  delay(8.6);
}

void drawContainer()
{
  _tft.drawFastVLine(CLOCK_X, CLOCK_Y, CLOCK_SIZE, WHITE); // L
  _tft.drawFastHLine(CLOCK_X, CLOCK_Y, CLOCK_SIZE, WHITE); // T
  _tft.drawFastVLine(CLOCK_X + CLOCK_SIZE, CLOCK_Y, CLOCK_SIZE + 1, WHITE); // R
  _tft.drawFastHLine(CLOCK_X, CLOCK_Y + CLOCK_SIZE, CLOCK_SIZE, WHITE); // B
}

void drawClock(String dtime)
{
  // PERCENT
  _onePerc = (dtime.toInt() / 1000000.00);
  _twoPerc = (dtime.substring(1,6).toInt() / 100000.00);
  _threePerc = (dtime.substring(2,6).toInt() / 10000.00); 

  // LINE
  _one[1] = CLOCK_LINE_THICKNESS + (_onePerc * LINE_LENGTH);
  _two[0] = CLOCK_LINE_THICKNESS + (_twoPerc * LINE_LENGTH);
  _two[1] = CLOCK_LINE_THICKNESS * 2 + (_onePerc * LINE_LENGTH);
  _two[2] = LINE_LENGTH - (_onePerc * LINE_LENGTH);
  _three[0] = CLOCK_LINE_THICKNESS + _two[0];
  _three[1] = CLOCK_LINE_THICKNESS + _one[1] + (_threePerc * (LINE_LENGTH - _one[1]));
  _three[2] = LINE_LENGTH + CLOCK_LINE_THICKNESS - _three[0];
    
  // ERASE
  if (_three[1] < CLOCK_SIZE) // Don't render if height exceeded (happens around 992:000+)
  {
    if (_three[4] != _three[1]) // three's yPrev != yCur
    {
      _tft.drawFastHLine(CLOCK_X + _three[3], CLOCK_Y + _three[4], _three[5], COLOR_THREE_PREV); // BLACK
    }
  }
  if (_two[3] != _two[0]) // two's xPrev != xCur
  {
    _tft.drawFastVLine(CLOCK_X + _two[3], CLOCK_Y + _two[4], _two[5], COLOR_TWO_PREV); // BLACK
  }
  if (_one[4] != _one[1]) // one's yPrev != yCur
  {
    _tft.drawFastHLine(CLOCK_X + _one[0], CLOCK_Y + _one[4], LINE_LENGTH, COLOR_ONE_PREV); // BLACK

  }
  
  // DRAW
  if (_one[4] != _one[1])
  {
    _tft.drawFastHLine(CLOCK_X + _one[0], CLOCK_Y + _one[1], LINE_LENGTH, COLOR_ONE_CUR); // WHITE
  }
  if (_two[3] != _two[0])
  {
    _tft.drawFastVLine(CLOCK_X + _two[0], CLOCK_Y + _two[1], _two[2], COLOR_TWO_CUR); // WHITE
  }
  if (_three[1] < CLOCK_SIZE) // Don't render if height exceeded (happens around 992:000+)
  {
    if (_three[4] != _three[1])
    {
      _tft.drawFastHLine(CLOCK_X + _three[0], CLOCK_Y + _three[1], _three[2], COLOR_THREE_CUR); // WHITE
    }
  }

  // PREV
  _one[4] = _one[1]; // yPrev = yCur
  _two[3] = _two[0]; // xPrev = xCur
  _two[4] = _two[1]; // yPrev = yCur
  _two[5] = _two[2]; // lPrev = lCur
  _three[3] = _three[0]; // xPrev = xCur
  _three[4] = _three[1]; // yPrev = yCur
  _three[5] = _three[2]; // lPrev = lCur
}
