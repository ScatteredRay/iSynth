#ifndef _CONFIG_H
#define _CONFIG_H

#include <map>
#include <string>
#include <vector>

enum { MAJOR, MINOR, DIMINISHED };

void loadConfig(std::string filename);

class Fretting
{ 
  public:
    Fretting() {} // buh?  std::map wants this for some reason?
    
    Fretting(int root, int type, std::string name)
    : m_root(root), m_chord_type(type), m_chord_name(name) {}
    
    int root() const { return m_root;       }    
    int type() const { return m_chord_type; }
    const std::string name() const { return m_chord_name; }

  private:    
    int m_root;
    int m_chord_type;
    std::string m_chord_name;
};

class Chord
{ 
  public:
    Chord() {}
  
    void addNoteOffset(int note_offset)
    {
      m_note_offsets.push_back(note_offset);
    }
    
    const std::vector<int> &noteOffsets() const { return m_note_offsets; }
 
  private:   
    std::vector<int> m_note_offsets;
};

class Tuning
{
  public:
    Tuning() {}
    
    void addString(int note) { m_strings.push_back(note); }
    
    const std::vector<int> &strings() { return m_strings; }
    
    std::vector<int> fretChord(int root_note, const Chord &chord,
                               int capo_position = 0);
    
  private:
    std::vector<int> m_strings;
};

extern std::map<std::string, Fretting> g_frettings;
extern std::map<int,         Chord>    g_chords;
extern std::map<std::string, Tuning>   g_tunings;

#endif