#include <cstdio>
#include "audio.h"
#include "input.h"

int main(int argc, char **argv)
{
  try
  {
    initInput();
    makeNoise();
  }
  catch(const char *s)
  {
    printf("Exception: %s", s);
  }

  return 0;
}