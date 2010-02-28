#include "input.h"

#include <cmath>
#include <cstdio>
#include <windows.h>

static HANDLE console;

void gotoXY(int x, int y)
{
  COORD coord = {x, y};
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void initInput()
{
  console = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(console, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);
  
  SMALL_RECT rect = {0, 0, 79, 49};
  SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &rect);;
  COORD origin = { 0, 0 };
  DWORD out;
  FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), '.', 80*50,
                             origin, &out);
  gotoXY(0, 0);
  printf("Click to make noise, escape to quit.");
}

void deinitInput()
{
  COORD origin = { 0, 0 };
  DWORD out;
  FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 80*50,
                             origin, &out);
}

static float g_x, g_y, g_button;

const int key_buffer_size = 16;
char g_key_buffer[key_buffer_size];
int g_key_write_pos = 0;
int g_key_read_pos = 0;

void readInput();

void getMouseState(float *x, float *y, float *button)
{
  readInput();
  *x = g_x;
  *y = g_y;
  *button = g_button;
}

char getKey()
{
  readInput();
  if(g_key_read_pos == g_key_write_pos) return 0;
  char c = g_key_buffer[g_key_read_pos++];
  g_key_read_pos %= key_buffer_size;
  return c;
}

void readInput()
{
  static float oldx, oldy, oldbutton;
  g_x = oldx;
  g_y = oldy;
  g_button = oldbutton;

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
        g_x = events[i].Event.MouseEvent.dwMousePosition.X / 40.0f - 1;
        g_y = 1 - events[i].Event.MouseEvent.dwMousePosition.Y / 25.0f;
        g_button = (events[i].Event.MouseEvent.dwButtonState &
                    FROM_LEFT_1ST_BUTTON_PRESSED) ? 1.0f:0.0f;
      }
      else if(events[i].EventType==KEY_EVENT &&
              events[i].Event.KeyEvent.bKeyDown)
      {
        int key = events[i].Event.KeyEvent.uChar.AsciiChar;
        if(key && (g_key_write_pos+1) % key_buffer_size != g_key_read_pos)
        {
          g_key_buffer[g_key_write_pos++] = key;          
          g_key_write_pos %= key_buffer_size;
        }
      }
      
    events_left -= events_read;
  }
  
  oldx = g_x;
  oldy = g_y;
  oldbutton = g_button;
  
  gotoXY(0, 0);
}

void readInputAxis(int axis, float *buffer, int size)
{
  float x, y, button;
  getMouseState(&x, &y, &button);

  float value;
  if(axis==0) value = x;
  if(axis==1) value = y;
  if(axis==2) value = button;
  
  if(value < -1) value = -1;
  if(value >  1) value =  1;
  
  for(int i=0; i<size; i++) buffer[i] = value;
}