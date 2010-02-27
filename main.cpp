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
  }
  catch(Exception &e)
  {
    printf("%s", e.describe().c_str());
    return 1;
  }

  return 0;
}