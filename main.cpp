#include <cstdio>
#include "audio.h"
#include "exception.h"
#include "input.h"

int main(int argc, char **argv)
{
  try
  {
    initInput();
    makeNoise();
    deinitInput();
  }
  catch(Exception &e)
  {
    deinitInput();
    printf("%s", e.describe().c_str());
    return 1;
  }
  
  return 0;
}