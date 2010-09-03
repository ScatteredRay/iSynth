#include "ui.h"

#include <vector>

#include "file.h"
#include "input.h"
#include "io.h"
#include "synth.h"

using namespace std;

void gotoXY(int x, int y);

class Selector
{
  public:
    Selector(string name, int x, int y, const vector<string> list)
    : m_name(name), m_x(x), m_y(y), m_list(list), m_pos(0) {}

    void draw(bool current)
    {
      gotoXY(m_x, m_y);
      printf("%s", m_name.c_str());
      for(int i=0; i<m_list.size(); i++)
      {
        char pre=' ', post=' ';
        if(i==m_pos)
        {
          if(current) pre='[', post=']';
          else pre = '*';          
        }
        gotoXY(m_x, i+m_y+1);
        printf("%c%s%c", pre, m_list[i].c_str(), post);
      }
    }

    void up()
    {
      m_pos--;
      if(m_pos<0) m_pos = m_list.size()-1;
    }

    void down()
    {
      m_pos++;
      if(m_pos>=m_list.size()) m_pos = 0;
    }

    int getPos() const { return m_pos; }
    void setPos(int pos) { m_pos = pos; }

  private:
    string m_name;
    int m_x, m_y, m_pos;
    vector<string> m_list;
};

vector<FileRef> g_patches;
vector<Selector> g_selectors;

int g_current_selector;

void uiInit()
{
  g_current_selector = 0;
  
  populatePatchList(g_patches);
  g_selectors.push_back(Selector("Patch", 0, 2, g_patches));

  vector<string> keys;
  for(int i=0; i<12; i++) keys.push_back(string(g_keys[i]));
  g_selectors.push_back(Selector("Key", 25, 2, keys));

  vector<string> scales;
  for(int i=0; g_scales[i].steps != 0; i++)
    scales.push_back(string(g_scales[i].name));
  g_selectors.push_back(Selector("Scale", 35, 2, scales));
  
  vector<string> octaves;
  for(int i=1; i<=5; i++)
  {
    char s[2];
    sprintf(s, "%d", i);
    octaves.push_back(string(s));
  }
  g_selectors.push_back(Selector("Octave", 48, 2, octaves));
  g_selectors.back().setPos(3);
  
  vector<string> octave_ranges;
  for(int i=1; i<=3; i++)
  {
    char s[2];
    sprintf(s, "%d", i);
    octave_ranges.push_back(string(s));
  }
  g_selectors.push_back(Selector("Octave Range", 55, 2, octave_ranges));
  g_selectors.back().setPos(1);

  synthSetScale(g_scales[g_selectors[2].getPos()].name);
  synthSetKey(g_selectors[1].getPos());
  synthSetRange(g_selectors[3].getPos()+1, g_selectors[4].getPos()+1);
}

bool uiTick()
{
  char key = getKey();
  if(key == K_ESCAPE) return true;
  
  if(key == K_LEFT )
  {
    g_current_selector--;
    if(g_current_selector<0) g_current_selector = g_selectors.size()-1;  
  }
  if(key == K_RIGHT)
  {
    g_current_selector++;
    if(g_current_selector>=g_selectors.size()) g_current_selector = 0;
  }

  if(key == K_UP  ) g_selectors[g_current_selector].up  ();
  if(key == K_DOWN) g_selectors[g_current_selector].down();
  if(key == K_UP || key == K_DOWN)
  {
    if(g_current_selector == 0)
      synthSetPatch(g_patches[g_selectors[0].getPos()]);
    if(g_current_selector == 1) synthSetKey(g_selectors[1].getPos());
    if(g_current_selector == 2)
      synthSetScale(g_scales[g_selectors[2].getPos()].name);
    if(g_current_selector == 3 || g_current_selector == 4)
      synthSetRange(g_selectors[3].getPos()+1, g_selectors[4].getPos()+1);
  }
  
  for(int i=0; i<g_selectors.size(); i++)
    g_selectors[i].draw(i==g_current_selector);
   
  return false;
}