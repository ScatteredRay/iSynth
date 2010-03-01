#include "input.h"

#include <cmath>
#include <cstdio>
#include <windows.h>

static HANDLE console;
static float input_x, input_y, input_button;

static const int key_buffer_size = 16;
static char key_buffer[key_buffer_size];
static int key_write_pos = 0;
static int key_read_pos = 0;

static int argc;
static char **argv;

void gotoXY(int x, int y)
{
  COORD coord = {x, y};
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void initInput(int _argc, char **_argv)
{
  argc = _argc, argv = _argv;
  
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

int argCount() { return argc; }
char *getArg(int n)
{
  if(n >= 0 && n < argc) return argv[n];
  return 0;
}

void deinitInput()
{
  COORD origin = { 0, 0 };
  DWORD out;
  FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 80*50,
                             origin, &out);
}

void readInput();

void getMouseState(float *x, float *y, float *button)
{
  readInput();
  *x = input_x;
  *y = input_y;
  *button = input_button;
}

char getKey()
{
  readInput();
  if(key_read_pos == key_write_pos) return 0;
  char c = key_buffer[key_read_pos++];
  key_read_pos %= key_buffer_size;
  return c;
}

void readInput()
{
  static float oldx, oldy, oldbutton;
  input_x = oldx;
  input_y = oldy;
  input_button = oldbutton;

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
        input_x = -1 + events[i].Event.MouseEvent.dwMousePosition.X / 40.0f;
        input_y =  1 - events[i].Event.MouseEvent.dwMousePosition.Y / 25.0f;
        input_button = (events[i].Event.MouseEvent.dwButtonState &
                        FROM_LEFT_1ST_BUTTON_PRESSED) ? 1.0f:0.0f;
      }
      else if(events[i].EventType==KEY_EVENT &&
              events[i].Event.KeyEvent.bKeyDown)
      {
        int key = events[i].Event.KeyEvent.uChar.AsciiChar;
        if(key && (key_write_pos+1) % key_buffer_size != key_read_pos)
        {
          key_buffer[key_write_pos++] = key;          
          key_write_pos %= key_buffer_size;
        }
      }
      
    events_left -= events_read;
  }
  
  oldx = input_x;
  oldy = input_y;
  oldbutton = input_button;
  
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