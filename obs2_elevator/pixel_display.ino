// This code is for the 64 x 32 pixel display that shows go or stop
// run on the arduino mega
#include "RGBmatrixPanel.h"
#include <string.h>
#include <stdlib.h>

#define CLK 11
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3
//pin used to send data from elevator to activate display
//need second arduino so that 
const int SIG_PIN = 3;          // reads HIGH=STOP, LOW=GO


RGBmatrixPanel matrix(A,B,C,D,CLK,LAT,OE,false,64);

void screen_clear() { matrix.fillScreen(0); }

void display_text(int x,int y,const char* txt,uint16_t color){
  matrix.setTextSize(2);
  matrix.setTextWrap(false);
  matrix.setFont(nullptr);
  matrix.setCursor(x,y);
  matrix.setTextColor(color);
  matrix.print(txt);
}

void drawStop()
{
  screen_clear();
  int w=matrix.width(), h=matrix.height();
  uint16_t col = matrix.Color333(7,0,0);
  matrix.drawRect(0,0,w,h,col); matrix.drawRect(1,1,w-2,h-2,col);
  display_text(9,8,"Stop", col);
}

void drawGo()
{
  screen_clear();
  int w=matrix.width(), h=matrix.height();
  uint16_t col = matrix.Color333(0,7,0);
  matrix.drawRect(0,0,w,h,col); matrix.drawRect(1,1,w-2,h-2,col);
  display_text(18,8,"Go", col);
}

void setup()
{
  pinMode(SIG_PIN, INPUT_PULLUP);   // HIGH = STOP by default
  matrix.begin();
  drawStop();                       // start with red frame
}

void loop()
{
  static bool lastLevel = HIGH;
  bool level = digitalRead(SIG_PIN);

  if (level != lastLevel) {         // state changed → redraw once
    if (level == HIGH) drawGo();
    else               drawStop();
    lastLevel = level;
  }
  delay(10);                        // cheap debounce & CPU breath
}
