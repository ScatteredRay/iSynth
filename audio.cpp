/* multiplex modules: must take at least one vectors of buffers in, maintains
   multiple channels for output, but can mixdown if a mono module requests.
   
   should be a way to create and destroy channels.  presumably create on
   note-on, how do we know when to destroy?  some way to flag, like when the
   envgen goes idle?  note-off and n samples of silence?  should be after the
   reverb trails off.  can't check for that, though, because we're not
   reverbing each channel individually.
   
   stereo: ulch, for now just let the convolution be a special case at the end.
   maybe we can make mono/stereo modules work similarly, where stereo modules
   maintain both channels but can mixdown if a mono module requests -- but it
   really should work the opposite way, whereas multiplex modules are a way
   to take poly input and produce a single output, stereo is a way to take a
   single input and produce two outputs.
   
   okay, okay, hear me out on this one: a stereo module can *convert to
   stereo* if given mono input.  huh?  huh?  i like it.
*/

#include <cstdio>
#include <cmath>
#include <vector>

const float pi = 3.1415926535897932384626f, e = 2.71828183f;
const int max_buffer_size = 4000;
const int sample_rate = 44100;

class Module
{
  public:
    Module()
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = 0;
      m_latest_fill = 0;
    }
        
    virtual void fill(float latest_fill, int samples) = 0;
    
    const float *output(float latest_fill, int samples)
    {
      if(m_latest_fill < latest_fill)
      {
        fill(latest_fill, samples);
        m_latest_fill = latest_fill;
      }
      return m_output;
    }
  
  protected:  
    float m_output[max_buffer_size];
    float m_latest_fill;
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
    
    void fill(float latest_fill, int samples)
    {
      const float *frequency = m_frequency.output(latest_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        
        m_output[i] = (m_position-0.5f)*2;
      
        m_position += frequency[i] / 44100;
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
    
    void fill(float latest_fill, int samples)
    {
      const float *frequency  = m_frequency .output(latest_fill, samples);
      const float *pulsewidth = m_pulsewidth.output(latest_fill, samples);
      
      for(int i=0; i<samples; i++)
      {
        m_output[i] = (m_position > pulsewidth[i]) ? 1.0f:-1.0f;      
        m_position += frequency[i] / 44100;
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

    void fill(float latest_fill, int samples)
    {
      const float *frequency  = m_frequency.output(latest_fill, samples);

      for(int i=0; i<samples; i++)
      {
        m_output[i] = sin(m_position*2*pi);
      
        m_position += frequency[i] / 44100;
        m_position -= int(m_position);
      }
    }
};

class Triangle : public Oscillator
{
  public:
    Triangle(Module &frequency) : Oscillator(frequency)
    {}

    void fill(float latest_fill, int samples)
    {
      const float *frequency  = m_frequency.output(latest_fill, samples);

      for(int i=0; i<samples; i++)
      {
        if(m_position < .5f) m_output[i] = (m_position-0.25f) *  4;
        else                 m_output[i] = (m_position-0.75f) * -4;
      
        m_position += frequency[i] / 44100;
        m_position -= int(m_position);
      }
    }
};

class Add : public Module
{
  public:
    Add(Module &a, Module &b)
    {
      addInput(&a);
      addInput(&b);
    }
   
    void addInput(Module *input) { m_inputs.push_back(input); }
    
    void fill(float latest_fill, int samples)
    {
      if(m_inputs.size() == 0) return;
      
      memcpy(m_output, m_inputs[0]->output(latest_fill, samples), samples * sizeof(float));
      for(unsigned int i=1; i<m_inputs.size(); i++)
      {
        const float *input = m_inputs[i]->output(latest_fill, samples);
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

class Delay : public Module
{
  public:
    Delay(int max_length = 44100) : Module()
    {
      buffer = new float[max_length];
    }
    
    ~Delay()
    {
      delete[] buffer;
    }
  
  private:
    float *buffer;
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
    Constant(float value) : Module()
    {
      setValue(value);
    }
  
    void fill(float last_fill, int samples) {}

    void setValue(float value)
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = value;
    }
};

void produceStream(short *buffer, int samples)
{
  static Constant freq_lfo_frequency(5);
  static Sine freq_lfo_unit(freq_lfo_frequency);
  static UnitScaler freq_lfo(freq_lfo_unit, -3, 3);

  static Constant osc1_base_frequency(55);
  static Constant osc2_base_frequency(55*1.01f);
  static Add osc1_frequency(osc1_base_frequency, freq_lfo);
  static Add osc2_frequency(osc2_base_frequency, freq_lfo);

  static Constant gate_frequency(6.0f);
  static Constant gate_pulsewidth(0.2f);
  static Pulse gate_unit(gate_frequency, gate_pulsewidth);
  static UnitScaler gate(gate_unit, 0, 1);
  static EnvelopeGenerator env(gate, 0.01f, 0.5f, 0.25f, 0.03f);

  static Saw osc1(osc1_frequency);
  static Saw osc2(osc2_frequency);
  static Add osc_mix(osc1, osc2);

  static Overdrive overdrive(osc_mix, 1);

  static Constant cutoff_lfo_frequency(0.2f);
  static UnitScaler cutoff_env(env, 250, 750); 
  static Sine cutoff_lfo_unit(cutoff_lfo_frequency);
  static UnitScaler cutoff_lfo(cutoff_lfo_unit, 0, 600);
  static Add cutoff(cutoff_lfo, cutoff_env); 
  static Constant filter_resonance(0.9f);
  static Filter filter(overdrive, cutoff, filter_resonance);

  static Multiply output(filter, env);
  
  static float time = 0;  
  const float *o = output.output(time, samples);
  time += 1.0;
  
  for(int i=0; i<samples; i++)
  {
    *buffer++ = short(o[i] * 32767);
    *buffer++ = short(o[i] * 32767);
  }
}
