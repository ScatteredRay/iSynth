#include <cstdio>
#include "audio.h"
#include "exception.h"
#include "input.h"
#include "synth.h"

int main(int argc, char **argv)
{
  try
  {
    initInput(argc, argv);
    makeNoise();
    deinitInput();
    puts(describeTimeSpent());
  }
  catch(Exception &e)
  {
    deinitInput();
    printf("%s", e.describe().c_str());
    return 1;
  }
  
  return 0;
}