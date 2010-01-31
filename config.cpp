#include <cstdio>
#include <cstdlib>
#include <cctype>

#include "config.h"

using namespace std;

map<string, Fretting> g_frettings;
map<int,    Chord>    g_chords;
map<string, Tuning>   g_tunings;

int chordFromText(char *s)
{
  if     (strcmp(s, "maj") == 0) return MAJOR;
  else if(strcmp(s, "min") == 0) return MINOR;
  else if(strcmp(s, "dim") == 0) return DIMINISHED;
  else throw "bad chord type";
}

int noteFromText(char *s)
{                       // A,  B, C, D, E, F, G
  static int offsets[] = { 9, 11, 0, 2, 4, 5, 7 };
  
  int note = offsets[s[0] - 'A'];
  if(s[1] == '#') note++;
  if(s[1] == 'b') note++;
  
  int octave;
  if(isdigit(s[1])) octave = s[1] - '0';
  else              octave = s[2] - '0';
  
  return octave*12 + note;
}

class FileReader
{
  public:
    FileReader(const char *filename, char *separators)
    : m_separators(separators), m_indentation(0), m_next_token(0)
    {
      FILE *in = fopen(filename, "rb");
      if(!in) throw "couldn't open file";
      
      fseek(in, 0, SEEK_END);
      m_buffer_size = ftell(in);
      fseek(in, 0, SEEK_SET);
      
      m_buffer = (char *)malloc(m_buffer_size+1);
      if(!m_buffer) throw "out of memory";
      
      fread(m_buffer, m_buffer_size, 1, in);
      m_buffer[m_buffer_size] = 0;
      
      fclose(in);
      
      m_buffer_position = m_buffer;
    }
    
    ~FileReader()
    {
      free(m_buffer);
    }
    
    bool reachedEOF()
    {
      return m_buffer + m_buffer_size <= m_buffer_position;
    }
    
    char *nextToken(int *out_indentation = 0)
    {
      char *output;
      
      if(m_next_token)
      {
        output = m_next_token;
        m_next_token = 0;
        if(out_indentation) *out_indentation = m_next_indentation;
        return output;
      }
      
      skipSeparator();
      if(reachedEOF())
      {
        if(out_indentation) *out_indentation = 0;
        return "";
      }

      if(out_indentation) *out_indentation = m_indentation;
      
      char *end = findSeparator();
      *end = 0;
      
      output = m_buffer_position;
      
      m_buffer_position = end+1;
      skipSeparator();
      
      return output;
    }
    
    void setNextToken(char *token, int indentation)
    {
      m_next_token       = token;
      m_next_indentation = indentation;
    }
    
    bool expect(char *token)
    {
      return strcmp(token, nextToken()) == 0;
    }

  private:    
    void skipSeparator()
    { 
      while(!reachedEOF() && strchr(m_separators, *m_buffer_position))
      {
        if(*m_buffer_position == '\n') m_indentation = 0;
        else m_indentation++; 
        m_buffer_position++;
      }
    }
    
    char *findSeparator()
    {
      char *output = m_buffer_position;
      while(!reachedEOF() && !strchr(m_separators, *output)) output++;
      return output;
    }
    
    char *m_buffer;
    int   m_buffer_size;
    char *m_separators;

    char *m_buffer_position;
    int   m_indentation;
    
    char *m_next_token;
    int   m_next_indentation;

};

void loadConfig(string filename)
{
  char *mode = "";
  
  FileReader config(filename.c_str(), " \r\n\t:");
  while(!config.reachedEOF())
  {
    int indentation;
    char *token;
    
    token = config.nextToken(&indentation);
    
    if(indentation == 0) mode = token;
    else
      switch(*mode)
      {
        case 'F': // frettings
        {
          int   root  = atoi(config.nextToken());
          char *chord = config.nextToken();
          char *name  = config.nextToken();
          g_frettings[token] = Fretting(root, chordFromText(chord), name);

          break;
        }
        case 'T': // tunings
        {
          Tuning &t = g_tunings[token] = Tuning();
          
          for(;;)
          {
            int inner_indentation;
            char *note = config.nextToken(&inner_indentation);
            if(inner_indentation <= indentation) // new line
            {
              config.setNextToken(note, inner_indentation);
              break;
            }
            
            t.addString(noteFromText(note));
          }
          
          break; 
        }
        case 'C': // chords
        {
          Chord &c = g_chords[chordFromText(token)] = Chord();
          
          for(;;)
          {
            int inner_indentation;
            char *note_offset = config.nextToken(&inner_indentation);
            if(inner_indentation <= indentation) // new line
            {
              config.setNextToken(note_offset, inner_indentation);
              break;
            }
            c.addNoteOffset(atoi(note_offset));
          }
          
          break;
        }
        default:
          throw "bad config mode";
      } 
  }
}

std::vector<int> Tuning::fretChord(int root_note, const Chord &chord,
                                   int capo_position)
{
  std::vector<int> output_notes;
  const std::vector<int> &note_offsets = chord.noteOffsets();
  
  for(unsigned int i=0; i<m_strings.size(); i++)
  {
    int lowest_note = 999;
    for(unsigned int j=0; j<note_offsets.size(); j++)
    {
      int note = (note_offsets[j] + root_note) % 12;
      int k;
      for(k = capo_position + m_strings[i]; k % 12 != note; k++)
        ;
      if(k < lowest_note) lowest_note = k;
    }
    output_notes.push_back(lowest_note);
  }
  
  return output_notes;
}
