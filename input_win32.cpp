#include "input.h"

#include <cmath>
#include <windows.h>

static HANDLE console;

void initInput()
{
  console = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(console, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

  SMALL_RECT rect = {0, 0, 79, 49};
  SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &rect);
}

void readXY(float *x, float *y, float *button)
{
  static float oldx, oldy, oldbutton;
  *x = oldx;
  *y = oldy;
  *button = oldbutton;

  DWORD events_left = 0;
  
  GetNumberOfConsoleInputEvents(console, &events_left);
  
  static INPUT_RECORD events[16];
  while(events_left)
  {
    DWORD events_read;
    ReadConsoleInput(console, events, min(16, events_left), &events_read);
    
    for(unsigned int i=0; i<events_read; i++)
      if(events[i].EventType==MOUSE_EVENT)
      {
        *x = events[i].Event.MouseEvent.dwMousePosition.X / 40.0f - 1;
        *y = 1 - events[i].Event.MouseEvent.dwMousePosition.Y / 25.0f;
        *button = (events[i].Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? 1.0f:0.0f;
      }
      
    events_left -= events_read;
  }
  
  oldx = *x;
  oldy = *y;
  oldbutton = *button;

  COORD coord = {0, 0};
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void readInputAxis(int axis, float *buffer, int size)
{
  float x, y, button;
  readXY(&x, &y, &button);

  float value;
  if(axis==0) value = x;
  if(axis==1) value = y;
  if(axis==2) value = button;
  
  if(value < -1) value = -1;
  if(value >  1) value =  1;
  
  for(int i=0; i<size; i++) buffer[i] = value;
}