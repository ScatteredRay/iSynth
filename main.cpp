#include "audio.h"
#include "config.h"

using namespace std;

string textFromNote(int note)
{
  static char *names[12] =
  { 
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };
  
  char buffer[5];
  sprintf(buffer, "%s%d", names[note % 12], note/12);
  
  return buffer;
}

int main(int argc, char **argv)
{
  loadConfig("config.txt");
  
  for(map<string, Fretting>::iterator i = g_frettings.begin();
      i != g_frettings.end(); i++)
  {
    const Fretting &f = i->second;
    vector<int> result = g_tunings["standard"].fretChord(f.root(),
                                                         g_chords[f.type()]);
    printf("%s: ", f.name().c_str());
    for(unsigned int i=0; i<result.size(); i++)
      printf("%s (%d) ", textFromNote(result[i]).c_str(),
                         result[i] - g_tunings["standard"].strings()[i]);
    printf("\n");
  }
  
  makeNoise();
}
