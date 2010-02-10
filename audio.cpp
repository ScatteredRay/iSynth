/* multiplex modules: must take at least one vectors of buffers in, maintains
   multiple channels for output, but can mixdown if a mono module requests.
   
   should be a way to create and destroy channels.  presumably create on
   note-on, how do we know when to destroy?  some way to flag, like when the
   envgen goes idle?  note-off and n samples of silence?  should be after the
   reverb trails off.  can't check for that, though, because we're not
   reverbing each channel individually.
   
   stereo: 
   should be separate stereo modules.  there'll be some code duplication but
   not a crazy amount.  conversion between them should be explicit.  i.e. a
   StereoModule that takes Modules as input, or vice versa.
   
   Todo:
   - sequencer (retriggerable)
   - multiple intonations!  just, meantone, quarter tone, well-tempered, etc.
   - clipper
   - hard/soft-limiter
   - panner
   - ping pong delay
   - rectifier
   - sample playback
   - oscillator hardsync
   - oscillator band-limiting
   - slew limiter
   - switch   
   - additional filters?  eq?
   - reverb
   - exponential envgen, DADSR, parameterized shape
   - stereo subsystem
   - multiplexing subsystem
   Done:
   - x/y input
   - scale quantizer -- actually, "scale" should be a parameter of "notetofrequency"!
*/

#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include "input.h"

using namespace std;

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4996) // fopen() purportedly unsafe

const float pi = 3.1415926535897932384626f, e = 2.71828183f;
const float note_0 = 8.1757989156;
const int max_buffer_size = 4000;
const int sample_rate = 44100;

struct
{ const char *name;
  const char *steps;
} scales[] =
{
  { "major",      "\2\2\1\2\2\2\1" },
  { "minor",      "\2\1\2\2\1\2\2" },
  { "dorian",     "\2\1\2\2\2\1\2" },
  { "phrygian",   "\1\2\2\2\1\2\2" },
  { "lydian",     "\2\2\2\1\2\2\1" },
  { "mixolydian", "\2\2\1\2\2\1\2" },
  { "locrian",    "\1\2\2\1\2\2\2" },
  { "pentatonic", "\2\2\3\2\3" },
  { "pent minor", "\3\2\2\3\2" },
  { "chromatic",  "\1" },
  { "whole",      "\2" },
  { "Minor 3rd",  "\3" },
  { "3rd",        "\4" },
  { "4ths",       "\5" },
  { "Tritone",    "\6" },
  { "5ths",       "\7" },
  { "Octave",     "\12" },
  { 0,            0 }
};

class WaveOut
{
  public:    
    WaveOut(const std::string filename, float scaler=32768, bool stereo=false)
    : m_scaler(scaler), m_length(0)
    {
      m_out = fopen(filename.c_str(), "wb");
      unsigned char header[] = 
      { 
        'R', 'I', 'F', 'F',
        0x00, 0x00, 0x00, 0x00, // wave size+36; patch later
        'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
        0x10, 0x00, 0x00, 0x00, 0x01, 0x00, // PCM
        0x01, 0x00, // channels
        0x44, 0xAC, 0x00, 0x00, // 44.1khz
        0x10, 0xB1, 0x02, 0x00, // 176400 bytes/sec
        0x02, 0x00, // bytes per sample*channels
        0x10, 0x00, // 16 bits
        'd', 'a', 't', 'a',
        0x00, 0x00, 0x00, 0x00, // wave size again
      };
      if(stereo) header[22] = 0x02, header[32] = 0x04;
      fwrite(header, sizeof(header), 1, m_out);
    }
    
    ~WaveOut() { close(); }
    
    void close()
    {
      if(!m_out) return;

      int chunklength = m_length*2+36;
      fseek(m_out, 4, SEEK_SET);
      fwrite(&chunklength, 4, 1, m_out);
      
      chunklength -= 36;
      fseek(m_out, 40, SEEK_SET);
      fwrite(&chunklength, 4, 1, m_out);
      
      fclose(m_out);
      m_out = 0;
    }
    
    void writeBuffer(const float *buffer, int size)
    {
      static short out[max_buffer_size];
      for(int i=0; i<size; i++)
        out[i] = short(buffer[i]*m_scaler);
      fwrite(out, 2, size, m_out);
      m_length += size;
    }

  private:
    FILE *m_out;
    float m_scaler;
    int m_length;
};

class Module
{
  public:
    Module() : m_last_fill(0), m_wout(0)
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = 0;
    }
    ~Module()
    {
      if(m_wout) delete m_wout;
    }
        
    virtual void fill(float last_fill, int samples) = 0;
    
    const float *output(float last_fill, int samples)
    {
      if(m_last_fill < last_fill)
      {
        fill(last_fill, samples);
        m_last_fill = last_fill;
        if(m_wout) m_wout->writeBuffer(m_output, samples);
      }
      return m_output;
    }
    
    void log(const string filename, float scaler=32768)
    {
      if(!m_wout) m_wout = new WaveOut(filename, scaler);
    }
  
  protected:  
    float m_output[max_buffer_size];
    float m_last_fill;
    WaveOut *m_wout;
};

class Oscillator : public Module
{
  public:
    Oscillator(Module &frequency)
    : Module(), m_position(0), m_frequency(frequency)
    {}
    
  protected:
    Module &m_frequency;
  
    float m_position;
};

class Saw : public Oscillator
{
  public:
    Saw(Module &frequency) : Oscillator(frequency) {}
    
    void fill(float last_fill, int samples)
    {
      const float *frequency = m_frequency.output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        
        m_output[i] = (m_position-0.5f)*2;
      
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
      }
    }
};

class Pulse : public Oscillator
{
  public:
    Pulse(Module &frequency, Module &pulsewidth)
    : Oscillator(frequency), m_pulsewidth(pulsewidth)
    {}
    
    void fill(float last_fill, int samples)
    {
      const float *frequency  = m_frequency .output(last_fill, samples);
      const float *pulsewidth = m_pulsewidth.output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        m_output[i] = (m_position > pulsewidth[i]) ? -1.0f:1.0f;      
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
      }
    }

  protected:
    Module &m_pulsewidth;
};

class Sine : public Oscillator
{
  public:
    Sine(Module &frequency) : Oscillator(frequency) {}

    void fill(float last_fill, int samples)
    {
      const float *frequency  = m_frequency.output(last_fill, samples);

      for(int i=0; i<samples; i++)
      {
        m_output[i] = sin(m_position*2*pi);
      
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
      }
    }
};

class Triangle : public Oscillator
{
  public:
    Triangle(Module &frequency) : Oscillator(frequency)
    {}

    void fill(float last_fill, int samples)
    {
      const float *frequency  = m_frequency.output(last_fill, samples);

      for(int i=0; i<samples; i++)
      {
        if(m_position < .5f) m_output[i] = (m_position-0.25f) *  4;
        else                 m_output[i] = (m_position-0.75f) * -4;
      
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
      }
    }
};

class Noise : public Module
{
  public:
    Noise() {}
    
    void fill(float last_fill, int samples)
    {
      for(int i=0; i<samples; i++)
        m_output[i] = 2.0f*rand()/RAND_MAX - 1.0;
    }
};

class SampleAndHold : public Module
{
  public:
    SampleAndHold(Module &source, Module &trigger)
    : m_source(source), m_trigger(trigger), m_value(0), m_waiting(true)
    {}
    
    void fill(float last_fill, int samples)
    {
      const float *source  = m_source .output(last_fill, samples);
      const float *trigger = m_trigger.output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        if(!m_waiting)
        {
          if(trigger[i] < 0.1) m_waiting = true;
        }
        else
        {
          if(trigger[i] > 0.9)
          {
            m_waiting = false;
            m_value = source[i];
          }
        }
        m_output[i] = m_value;
      }
    }
    
  private:
    Module &m_source, &m_trigger;
    float m_value;
    bool m_waiting;
};

class Quantize : public Module
{
  public: 
    Quantize(Module &input) : m_input(input) {}
    
    void fill(float last_fill, int samples)
    {
      const float *input = m_input.output(last_fill, samples);
      for(int i=0; i<samples; i++)
        m_output[i] = floor(input[i]+0.5);
    }
  
  private:
    Module &m_input;
};

class NoteToFrequency : public Module
{
  public:
    NoteToFrequency(Module &input, int key, const char *scale)
    : m_input(input)
    {
      setScale(key, scale);
    }
    
    void setScale(int key, const char *scale)
    {
      int note = key;
      const char *scalepos = scale;
      for(int i=0; i<128; i++)
      {
        notes[i] = note;
        note += *scalepos++;
        if(*scalepos == 0) scalepos = scale;
      }
    }
        
    void fill(float last_fill, int samples)
    {
      const float *input = m_input.output(last_fill, samples);
      for(int i=0; i<samples; i++)
      {
        int scaledegree = input[i];
        if(scaledegree > 127) scaledegree = 127;
        if(scaledegree < 0  ) scaledegree = 0;
        int note = notes[scaledegree];
        m_output[i] = pow(2.0, note/12.0) * note_0;
      }
    }
    
  private:
    Module &m_input;
    int notes[128];
};

class Add : public Module
{
  public:
    Add() {}
    Add(Module &a, Module &b)
    {
      addInput(a);
      addInput(b);
    }
   
    void addInput(Module &input) { m_inputs.push_back(&input); }
    
    void fill(float last_fill, int samples)
    {
      if(m_inputs.size() == 0) return;
      
      memcpy(m_output, m_inputs[0]->output(last_fill, samples), samples * sizeof(float));
      for(unsigned int i=1; i<m_inputs.size(); i++)
      {
        const float *input = m_inputs[i]->output(last_fill, samples);
        for(int j=0; j<samples; j++)
          m_output[j] += input[j];
      }
    }
    
  private:
    std::vector<Module *> m_inputs;
};

class Filter : public Module
{
  public:
    Filter(Module &input, Module &cutoff, Module &resonance)
    : m_input(input), m_cutoff(cutoff), m_resonance(resonance),
      m_oldcutoff(-1), m_oldresonance(-1)
    {
      m_y1=m_y2=m_y3=m_y4=m_oldx=m_oldy1=m_oldy2=m_oldy3 = 0;
    }
    
    inline void recalculateFilter(float cutoff, float resonance)
    {
      m_f = 2.0f * cutoff / sample_rate;
      m_k = 3.6f*m_f - 1.6f*m_f*m_f -1.0f;
      m_p = (m_k+1.0f)*0.5f;
      m_scale = pow(e, (1.0f-m_p)*1.386249f);
      m_r = resonance * m_scale;      
    }
    
    void fill(float last_fill, int samples)
    {
      const float *input     = m_input    .output(last_fill, samples);
      const float *cutoff    = m_cutoff   .output(last_fill, samples);
      const float *resonance = m_resonance.output(last_fill, samples);

      for(int i=0; i<samples; i++)
      {
        if(m_oldcutoff != cutoff[i] || m_oldresonance != resonance[i])
          recalculateFilter(cutoff[i], resonance[i]);
          
        float x = input[i] - m_r*m_y4;

        //Four cascaded onepole filters (bilinear transform)
        m_y1 =    x*m_p + m_oldx *m_p - m_k*m_y1;
        m_y2 = m_y1*m_p + m_oldy1*m_p - m_k*m_y2;
        m_y3 = m_y2*m_p + m_oldy2*m_p - m_k*m_y3;
        m_y4 = m_y3*m_p + m_oldy3*m_p - m_k*m_y4;

        //Clipper band limited sigmoid
        m_y4 -= pow(m_y4, 3)/6;

        m_oldx  = x;
        m_oldy1 = m_y1;
        m_oldy2 = m_y2;
        m_oldy3 = m_y3;
        
        m_output[i] = m_y4;
      }
    }

  private:
    Module &m_input, &m_cutoff, &m_resonance;
    float m_oldcutoff, m_oldresonance;
    float m_f, m_k, m_p, m_scale, m_r;
    float m_y1, m_y2, m_y3, m_y4, m_oldx, m_oldy1, m_oldy2, m_oldy3;
};

class Overdrive : public Module
{
  public:
    Overdrive(Module &input, float amount = 2.0f)
    : m_input(input), m_amount(1.0f - 1.0f/amount)
    {}
    
    void fill(float last_fill, int samples)
    {
      const float *input = m_input.output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        float x = input[i];
        float k = 2*m_amount/(1-m_amount);
        m_output[i] = (1+k)*x/(1+k*abs(x));
      }
    }
  
  private:
    Module &m_input;
    
    float m_amount;
};

class Multiply : public Module
{
  public:
    Multiply(Module &a, Module &b) : m_a(a), m_b(b) {}
    
    void fill(float last_fill, int samples)
    {      
      const float *a = m_a.output(last_fill, samples);
      const float *b = m_b.output(last_fill, samples);
      for(int i=0; i<samples; i++) m_output[i] = a[i]*b[i];
    }

  private:
    Module &m_a, &m_b;
};

class EnvelopeGenerator : public Module
{
  public:
    EnvelopeGenerator(Module &gate, float attack, float decay,
                      float sustain, float release)
    : m_gate(gate), m_attack(attack), m_decay(decay), m_sustain(sustain),
      m_release(release), m_position(0), m_rate(0), m_held(false), m_stage(IDLE)
    { 
    }
    
    enum { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
    
    void fill(float last_fill, int samples)
    {
      const float *gate = m_gate.output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        if(!m_held)
        { 
          if(gate[i] > 0.9)
          {
            m_stage = ATTACK;
            m_rate = (1-m_position) / (sample_rate*m_attack);
            m_held = true;
          }
        }
        else
        { 
          if(gate[i] < 0.1)
          {
            m_stage = RELEASE;
            m_rate = -m_position / (sample_rate*m_release);
            m_held = false;
          }
        }

        m_position += m_rate;
        
        switch(m_stage)
        {
          case IDLE: case SUSTAIN: break;
          case ATTACK:
            if(m_position >= 1)          
            {
              m_stage = DECAY;
              m_position = 1;
              m_rate = (m_sustain-1) / (sample_rate*m_decay);
            }
            break;
          case DECAY:
            if(m_position <= m_sustain)
            {
              m_stage = SUSTAIN;
              m_position = m_sustain;
              m_rate = 0;
            }
            break;
          case RELEASE:
            if(m_position <= 0.0)
            {
              m_stage = IDLE;
              m_position = 0.0;
              m_rate = 0.0;
            }
            break;
        }
                
        m_output[i] = m_position; 
      }
    }
    
  private:   
    float m_position, m_rate;
    bool m_held;
    int m_stage;    
    
    Module &m_gate;
    float m_attack, m_decay, m_sustain, m_release;    
};


// maybe support sweeping the length too?  i bet that would sound awesome
// speed parameter currently unimplemented
class Delay : public Module
{
  public: 
    Delay(float length, float filter, Module &input, Module &wet,
          Module &feedback, Module &speed)
    : m_buffer(new float[int(length*sample_rate)]),
      m_length(int(length*sample_rate)),
      m_read_pos(1), m_write_pos(0), m_last_sample(0), m_filter(filter),
      m_input(input), m_wet(wet), m_feedback(feedback), m_speed(speed)
    {
      memset(m_buffer, 0, m_length*sizeof(float));
    }
    
    ~Delay()
    {
      delete[] m_buffer;
    }
    
    void fill(float last_fill, int samples)
    {
      const float *input    = m_input   .output(last_fill, samples);
      const float *wet      = m_wet     .output(last_fill, samples);
      const float *feedback = m_feedback.output(last_fill, samples);
      const float *speed    = m_speed   .output(last_fill, samples);
      
      for(int i=0; i<samples; i++)
      { 
        float sample = m_buffer[int(floor(m_read_pos))]*m_filter + m_last_sample*(1-m_filter);
        m_last_sample = sample;
        m_output[i] = input[i] + sample * wet[i];
        m_buffer[m_write_pos] = input[i] + sample * feedback[i];
        
        if(++m_write_pos >= m_length) m_write_pos -= m_length;
        if(++m_read_pos  >= m_length) m_read_pos  -= m_length;
      }
    }
  
  private:
    float *m_buffer;
    int m_length;
    float m_filter;
    float m_read_pos;
    float m_last_sample;
    int m_write_pos;
    
    Module &m_input, &m_wet, &m_feedback, &m_speed;
};

class UnitScaler : public Module
{
  public:
    UnitScaler(Module &input, float min, float max)
    : m_input(input), m_range((max-min)/2)
    { 
      m_offset = min+m_range;
    }
    
    void fill(float last_fill, int samples)
    {
      const float *input = m_input.output(last_fill, samples);      
      for(int i=0; i<samples; i++) m_output[i] = input[i]*m_range + m_offset;
    }

  private:
    Module &m_input;
    float m_range, m_offset;
};

class Constant : public Module
{
  public:
    Constant(float value) { setValue(value); }
  
    void fill(float last_fill, int samples) {}

    void setValue(float value)
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = value;
    }
};

class Input : public Module
{
  public:
    Input(int axis) : m_axis(axis) {}
    
    void fill(float last_fill, int samples)
    {
      readInputAxis(m_axis, m_output, samples);
    }
  
  private:
    int m_axis;
};

void produceStream(short *buffer, int samples)
{
  static Input x(0);
  static Input y(1);
  static Input touch(2);

  static UnitScaler note_offset(x, 15, 25);
  static Quantize note_tuned(note_offset);
  static NoteToFrequency osc1_frequency(note_tuned, 5, "\3\2\2\3\2");
  static Constant freq_multiplier(1.01);
  static Multiply osc2_frequency(osc1_frequency, freq_multiplier);

  static Saw osc1(osc1_frequency);
  static Saw osc2(osc2_frequency);
  static Add osc_mix(osc1, osc2);

  static EnvelopeGenerator env(touch, 0.01f, 0.2f, 0.25f, 0.03f);

  static UnitScaler cutoff_y(y, 250, 1500); 
  static Add cutoff(osc1_frequency, cutoff_y); 
  
  static Constant filter_resonance(0.9f);
  static Filter filter(osc_mix, cutoff, filter_resonance);

  static Multiply notes(filter, env);
  
  static Constant delay_wet(0.5f);
  static Constant delay_feedback(0.6f);
  static Constant delay_speed(1.0f);
  static Delay output(1.5/3.0, 0.2, notes, delay_wet, delay_feedback, delay_speed);
  
  static float time = 0;  
  const float *o = output.output(time, samples);
  time += 1.0;
  
  for(int i=0; i<samples; i++)
  {
    *buffer++ = short(o[i] * 32767);
#ifndef __APPLE__
    *buffer++ = short(o[i] * 32767);
#endif
  }
}
