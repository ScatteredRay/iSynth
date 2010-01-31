#include <cstdio>
#include <cmath>
#include <vector>

const float pi = 3.1415926535897932384626f;
const int max_buffer_size = 4000;
const int sample_rate = 44100;

class Module
{
  public:
    Module()
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = 0;      
    }
    
    virtual void fill(int samples) = 0;
    
    const float *output() const { return m_output; }
  
  protected:
    float m_output[max_buffer_size];
};

class Oscillator : public Module
{
  public:
    Oscillator(const float *frequency, const float *amplitude)
    : Module(), m_position(0),
      m_frequency_input_buffer(frequency),
      m_amplitude_input_buffer(amplitude)
    {
    }
  
    void setFrequency(const float *f) { m_frequency_input_buffer = f; }
    void setAmplitude(const float *a) { m_amplitude_input_buffer = a; }
    
  protected:
    const float *m_frequency_input_buffer;
    const float *m_amplitude_input_buffer;
  
    float m_position;
};

class Saw : public Oscillator
{
  public:
    Saw(const float *frequency, const float *amplitude)
    : Oscillator(frequency, amplitude)
    {}
    
    void fill(int samples)
    {
      for(int i=0; i<samples; i++)
      {
        m_output[i] = (m_position-0.5f)*2 * m_amplitude_input_buffer[i];
      
        m_position += m_frequency_input_buffer[i] / 44100;
        m_position -= int(m_position);
      }
    }
};

class Sine : public Oscillator
{
  public:
    Sine(const float *frequency, const float *amplitude)
    : Oscillator(frequency, amplitude)
    {}

    void fill(int samples)
    {
      for(int i=0; i<samples; i++)
      {
        m_output[i] = sin(m_position*2*pi) * m_amplitude_input_buffer[i];
      
        m_position += m_frequency_input_buffer[i] / 44100;
        m_position -= int(m_position);
      }
    }
};

class Mixer : public Module
{
  public:
    void addInput(const float *input)
    {
      m_inputs.push_back(input);
    }
    
    void fill(int samples)
    {
      if(m_inputs.size() == 0) return;
      
      memcpy(m_output, m_inputs[0], samples * sizeof(float));
      for(unsigned int i=1; i<m_inputs.size(); i++)
      {
        const float *input = m_inputs[i];      
        for(int j=0; j<samples; j++)
          m_output[j] += input[j];
      }
    }
    
  private:
    std::vector<const float *> m_inputs;
};

class Triangle : public Oscillator
{
  public:
    Triangle(const float *frequency, const float *amplitude)
    : Oscillator(frequency, amplitude)
    {}

    void fill(int samples)
    {
      for(int i=0; i<samples; i++)
      {
        float value;
        if(m_position < .5f) value = (m_position-0.25f) *  4;
        else                 value = (m_position-0.75f) * -4;
        
        m_output[i] = value * m_amplitude_input_buffer[i];
      
        m_position += m_frequency_input_buffer[i] / 44100;
        m_position -= int(m_position);
      }
    }
};

class Overdrive : public Module
{
  public:
    Overdrive(const float *input, float preamp = 3.0f,
              float threshold = 0.75f, float intensity = 2.0f)
    : Module(), m_input(input), m_preamp(preamp), m_threshold(threshold),
      m_intensity(intensity), m_value(0)
    {}
    
    void fill(int samples)
    {
      for(int i=0; i<samples; i++)
      {
        float sample = m_input[i] * m_preamp;
        //m_output[i] = sample;
        //continue;
        
        if(sample > 0) m_value =  1.0 - 1.0/(sample+1.0);
        else           m_value = -1.0 - 1.0/(sample+1.0);
        
        if(sample < m_threshold && sample > -m_threshold) m_value = sample;
        else
        {
          if(sample > 0) m_value =  m_threshold;
          else           m_value = -m_threshold;
          /*
          float filter = 1.0 / ((fabs(sample) - m_threshold) * m_intensity); 
          m_value = m_value * filter + sample * (1.0f - filter);
          
          if(m_value >  1.0f) m_value =  1.0f;
          if(m_value < -1.0f) m_value = -1.0f;*/
        }
        m_output[i] = m_value;
      }
    }
    
    void setInput(const float *i) { m_input = i; }
  
  private:
    const float *m_input;
    
    float m_value;
    
    float m_preamp;
    float m_threshold;
    float m_intensity;
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

class Constant : public Module
{
  public:
    Constant(float value) : Module()
    {
      setValue(value);
    }
  
    void fill(int samples) {}

    void setValue(float value)
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = value;
    }
};

void produceStream(short *buffer, int samples)
{
  static Constant freq_lfo_frequency(4);
  static Constant freq_lfo_amplitude(10);
  static Constant osc1_base_frequency(200);
  static Constant osc2_base_frequency(200*1.5);
  static Constant osc_amplitude(.25);
  static Sine freq_lfo(freq_lfo_frequency.output(), freq_lfo_amplitude.output());
  static Mixer osc1_frequency;
  static Mixer osc2_frequency;
  static Triangle osc1(osc1_frequency.output(), osc_amplitude.output());
  static Triangle osc2(osc2_frequency.output(), osc_amplitude.output());
  static Mixer osc_mix;
  static Overdrive output(osc_mix.output(), 3.0, 0.5, 3.0);
  
  static bool first = true;
  if(first)
  {
    osc1_frequency.addInput(osc1_base_frequency.output());
    osc1_frequency.addInput(freq_lfo.output());
    osc2_frequency.addInput(osc2_base_frequency.output());
    osc2_frequency.addInput(freq_lfo.output());
    osc_mix.addInput(osc1.output());
    osc_mix.addInput(osc2.output());    

    freq_lfo_frequency.fill(samples);
    freq_lfo_amplitude.fill(samples);
    osc1_base_frequency.fill(samples);
    osc2_base_frequency.fill(samples);
    osc_amplitude.fill(samples);

    first = false;
  }
  
  freq_lfo.fill(samples);
  osc1_frequency.fill(samples);
  osc2_frequency.fill(samples);
  osc1.fill(samples);
  osc2.fill(samples);
  osc_mix.fill(samples);
  output.fill(samples);
  
  for(int i=0; i<samples; i++)
  {
    const float *o = output.output();
    *buffer++ = short(o[i] * 32767);
    *buffer++ = short(o[i] * 32767);
  }
}
