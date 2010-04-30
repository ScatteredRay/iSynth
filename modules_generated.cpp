// Generated code! Poke around in modules.pre and moduleprocess.py, not here.

class Saw : public Module
{
  public:
    Saw(vector<ModuleParam *> parameters)
    {
      m_frequency = parameters[0]->m_module;
      m_retrigger = parameters[1]->m_module;
      m_position = 0;
      m_last_trigger = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Saw(parameters);
    }

    const char *moduleName() { return "Saw"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *frequency = m_frequency->output(last_fill, samples);
      const float *retrigger = m_retrigger->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = (m_position-0.5f)*2;
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
        if(m_last_trigger < 0.1 && retrigger[i] > 0.9)
          m_position = 0;
        m_last_trigger = retrigger[i];
      }
      g_profiler.addTime("Saw", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
      validateWithin(*m_frequency, 0, 22050);
    }
  private:
    Module *m_frequency;
    Module *m_retrigger;
    float m_position;
    float m_last_trigger;
};


class Pulse : public Module
{
  public:
    Pulse(vector<ModuleParam *> parameters)
    {
      m_frequency = parameters[0]->m_module;
      m_pulsewidth = parameters[1]->m_module;
      m_retrigger = parameters[2]->m_module;
      m_position = 0;
      m_last_trigger = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Pulse(parameters);
    }

    const char *moduleName() { return "Pulse"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *frequency = m_frequency->output(last_fill, samples);
      const float *pulsewidth = m_pulsewidth->output(last_fill, samples);
      const float *retrigger = m_retrigger->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = (m_position > pulsewidth[i]) ? -1.0f:1.0f;
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
        if(m_last_trigger < 0.1 && retrigger[i] > 0.9)
          m_position = 0;
        m_last_trigger = retrigger[i];
      }
      g_profiler.addTime("Pulse", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
      validateWithin(*m_frequency, 0, 22050);
    }
  private:
    Module *m_frequency;
    Module *m_pulsewidth;
    Module *m_retrigger;
    float m_position;
    float m_last_trigger;
};


class Sine : public Module
{
  public:
    Sine(vector<ModuleParam *> parameters)
    {
      m_frequency = parameters[0]->m_module;
      m_retrigger = parameters[1]->m_module;
      m_phase_offset = parameters[2]->m_module;
      m_position = 0;
      m_last_trigger = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Sine(parameters);
    }

    const char *moduleName() { return "Sine"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *frequency = m_frequency->output(last_fill, samples);
      const float *retrigger = m_retrigger->output(last_fill, samples);
      const float *phase_offset = m_phase_offset->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = sin((m_position+phase_offset[i])*2*pi);
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
        if(m_last_trigger < 0.1 && retrigger[i] > 0.9)
          m_position = 0;
        m_last_trigger = retrigger[i];
      }
      g_profiler.addTime("Sine", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
      validateWithin(*m_frequency, 0, 22050);
    }
  private:
    Module *m_frequency;
    Module *m_retrigger;
    Module *m_phase_offset;
    float m_position;
    float m_last_trigger;
};


class Triangle : public Module
{
  public:
    Triangle(vector<ModuleParam *> parameters)
    {
      m_frequency = parameters[0]->m_module;
      m_retrigger = parameters[1]->m_module;
      m_position = 0;
      m_last_trigger = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Triangle(parameters);
    }

    const char *moduleName() { return "Triangle"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *frequency = m_frequency->output(last_fill, samples);
      const float *retrigger = m_retrigger->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(m_position < .5f) *output++ = (m_position-0.25f) *  4;
        else                 *output++ = (m_position-0.75f) * -4;
        m_position += frequency[i] / sample_rate;
        m_position -= int(m_position);
        if(m_last_trigger < 0.1 && retrigger[i] > 0.9)
          m_position = 0;
        m_last_trigger = retrigger[i];
      }
      g_profiler.addTime("Triangle", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
      validateWithin(*m_frequency, 0, 22050);
    }
  private:
    Module *m_frequency;
    Module *m_retrigger;
    float m_position;
    float m_last_trigger;
};


class Noise : public Module
{
  public:
    Noise(vector<ModuleParam *> parameters)
    {

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Noise(parameters);
    }

    const char *moduleName() { return "Noise"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = 2.0f*rand()/RAND_MAX - 1.0;
      }
      g_profiler.addTime("Noise", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
    }
  private:
};


class Within : public Module
{
  public:
    Within(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_min = parameters[1]->m_float;
      m_max = parameters[2]->m_float;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Within(parameters);
    }

    const char *moduleName() { return "Within"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = (input[i]>=m_min && input[i]<=m_max) ? 1 : 0;
      }
      g_profiler.addTime("Within", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = 0;
      *out_max = 1;
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    float m_min;
    float m_max;
};


class Sample : public Module
{
  public:
    Sample(vector<ModuleParam *> parameters)
    {
      m_sample_name = parameters[0]->m_string;
      m_frequency = parameters[1]->m_module;
      m_trigger = parameters[2]->m_module;
      m_loop_start = parameters[3]->m_module;
      m_loop_end = parameters[4]->m_module;
      m_position = 0;
      m_sample = 0;
      m_rate_coefficient = 0;
      m_last_trigger = -1;
      m_sample = new WaveIn(m_sample_name);
      m_rate_coefficient = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Sample(parameters);
    }

    const char *moduleName() { return "Sample"; }
    ~Sample() { delete m_sample; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *frequency = m_frequency->output(last_fill, samples);
      const float *trigger = m_trigger->output(last_fill, samples);
      const float *loop_start = m_loop_start->output(last_fill, samples);
      const float *loop_end = m_loop_end->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = m_sample->valueAt(m_position);
        if(m_last_trigger < 0.1 && trigger[i] > 0.9)
        {
          m_position = 0;
          m_rate_coefficient = m_sample->nativeSampleRate() / middle_c /
                               m_sample->length() / sample_rate;
        }
        m_last_trigger = trigger[i];
        m_position += frequency[i] * m_rate_coefficient;
        if(m_position > loop_end[i])
        {
          m_position -= (loop_end[i] - loop_start[i]);
          if(m_position > loop_end[i]) m_position = loop_end[i];
        }
      }
      g_profiler.addTime("Sample", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1;
      *out_max =  1;
    }

    void validateInputRange()
    {
      validateWithin(*m_frequency, 0, 22050);
      validateWithin(*m_trigger, 0, 1);
      validateWithin(*m_loop_start, 0, 1);
      validateWithin(*m_loop_end, 0, 1);
    }
  private:
    string m_sample_name;
    Module *m_frequency;
    Module *m_trigger;
    Module *m_loop_start;
    Module *m_loop_end;
    float m_position;
    WaveIn* m_sample;
    float m_rate_coefficient;
    float m_last_trigger;
};


class Rescaler : public Module
{
  public:
    Rescaler(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_to_min = parameters[1]->m_float;
      m_to_max = parameters[2]->m_float;
      m_from_min = 0;
      m_from_range = 0;
      m_to_range = 0;
      float from_max;
      m_input->getOutputRange(&m_from_min, &from_max);
      m_to_range   = m_to_max - m_to_min;
      m_from_range = from_max - m_from_min;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Rescaler(parameters);
    }

    const char *moduleName() { return "Rescaler"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = (input[i]-m_from_min) / m_from_range * m_to_range + m_to_min;
      }
      g_profiler.addTime("Rescaler", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = m_to_min;
      *out_max = m_to_min + m_to_range;
      if(*out_max < *out_min) swap(*out_max, *out_min);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    float m_to_min;
    float m_to_max;
    float m_from_min;
    float m_from_range;
    float m_to_range;
};


class Filter : public Module
{
  public:
    Filter(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_cutoff = parameters[1]->m_module;
      m_resonance = parameters[2]->m_module;
      m_oldcutoff = -1;
      m_oldresonance = -1;
      m_scale = 0;
      m_f = 0;
      m_k = 0;
      m_p = 0;
      m_r = 0;
      m_y1 = 0;
      m_y2 = 0;
      m_y3 = 0;
      m_y4 = 0;
      m_oldx = 0;
      m_oldy1 = 0;
      m_oldy2 = 0;
      m_oldy3 = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Filter(parameters);
    }

    const char *moduleName() { return "Filter"; }
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
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *cutoff = m_cutoff->output(last_fill, samples);
      const float *resonance = m_resonance->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(m_oldcutoff != cutoff[i] || m_oldresonance != resonance[i])
        {
          recalculateFilter(cutoff[i], resonance[i]);
          m_oldcutoff = cutoff[i], m_oldresonance = resonance[i];
        }
        m_oldcutoff = cutoff[i], m_oldresonance = resonance[i];
        //Inverted feed back for corner peaking
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
        *output++ = m_y4;
      }
      g_profiler.addTime("Filter", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max); // maybe?  this math is hard.
    }

    void validateInputRange()
    {
      validateWithin(*m_cutoff, 0, 22050);
      validateWithin(*m_resonance, 0, 1);
    }
  private:
    Module *m_input;
    Module *m_cutoff;
    Module *m_resonance;
    float m_oldcutoff;
    float m_oldresonance;
    float m_scale;
    float m_f;
    float m_k;
    float m_p;
    float m_r;
    float m_y1;
    float m_y2;
    float m_y3;
    float m_y4;
    float m_oldx;
    float m_oldy1;
    float m_oldy2;
    float m_oldy3;
};


class Multiply : public Module
{
  public:
    Multiply(vector<ModuleParam *> parameters)
    {
      m_a = parameters[0]->m_module;
      m_b = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Multiply(parameters);
    }

    const char *moduleName() { return "Multiply"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *a = m_a->output(last_fill, samples);
      const float *b = m_b->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = a[i]*b[i];
      }
      g_profiler.addTime("Multiply", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float a_min, a_max, b_min, b_max;
      m_a->getOutputRange(&a_min, &a_max);
      m_b->getOutputRange(&b_min, &b_max);
      *out_min = min(min(a_min*b_max, b_min*a_max), a_min*b_min);
      *out_max = a_max*b_max;
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_a;
    Module *m_b;
};

// should i bother giving this an arbitrary parameter count?

class Add : public Module
{
  public:
    Add(vector<ModuleParam *> parameters)
    {
      m_a = parameters[0]->m_module;
      m_b = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Add(parameters);
    }

    const char *moduleName() { return "Add"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *a = m_a->output(last_fill, samples);
      const float *b = m_b->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = a[i] + b[i];
      }
      g_profiler.addTime("Add", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float a_min, a_max, b_min, b_max;
      m_a->getOutputRange(&a_min, &a_max);
      m_b->getOutputRange(&b_min, &b_max);
      *out_min = a_min + b_min;
      *out_max = a_max + b_max;
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_a;
    Module *m_b;
};


class Quantize : public Module
{
  public:
    Quantize(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Quantize(parameters);
    }

    const char *moduleName() { return "Quantize"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = floor(input[i]+0.5);
      }
      g_profiler.addTime("Quantize", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
      *out_min = floor(*out_min + 0.5);
      *out_max = floor(*out_max + 0.5);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
};


class EnvelopeGenerator : public Module
{
  public:
    EnvelopeGenerator(vector<ModuleParam *> parameters)
    {
      m_gate = parameters[0]->m_module;
      m_att = parameters[1]->m_float;
      m_dec = parameters[2]->m_float;
      m_sus = parameters[3]->m_float;
      m_rel = parameters[4]->m_float;
      m_position = 0;
      m_coefficient = 0;
      m_destination = 0;
      m_held = false;
      m_stage = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new EnvelopeGenerator(parameters);
    }

    const char *moduleName() { return "EnvelopeGenerator"; }
    enum { IDLE = 0, ATTACK, DECAY, SUSTAIN, RELEASE };

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *gate = m_gate->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(!m_held)
        {
          if(gate[i] > 0.9)
          {
            m_stage       = ATTACK;
            m_coefficient = onepoleCoefficient(m_att);
            m_destination = 1;
            m_held        = true;
          }
        }
        else
        {
          if(gate[i] < 0.1)
          {
            m_stage       = RELEASE;
            m_coefficient = onepoleCoefficient(m_rel);
            m_destination = 0;
            m_held        = false;
          }
        }
        m_position = m_position * m_coefficient + m_destination * (1-m_coefficient);
        switch(m_stage)
        {
          case ATTACK:
            if(m_position >= 0.999)
            {
              m_stage       = DECAY;
              m_coefficient = onepoleCoefficient(m_dec);
              m_destination = m_sus;
            }
            break;
          case RELEASE:
            if(m_position <= 0.00001)
            {
              m_stage       = IDLE;
              m_position    = 0;
              m_destination = 0;
              m_coefficient = 0;
            }
            break;
        }
        *output++ = m_position;
      }
      g_profiler.addTime("EnvelopeGenerator", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = 0;
      *out_max = 1;
    }

    void validateInputRange()
    {
      validateWithin(*m_gate, -1, 1);
    }
  private:
    Module *m_gate;
    float m_att;
    float m_dec;
    float m_sus;
    float m_rel;
    float m_position;
    float m_coefficient;
    float m_destination;
    bool m_held;
    int m_stage;
};


class SampleAndHold : public Module
{
  public:
    SampleAndHold(vector<ModuleParam *> parameters)
    {
      m_source = parameters[0]->m_module;
      m_trigger = parameters[1]->m_module;
      m_waiting = true;
      m_value = 0;
      m_last_trigger = 0;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new SampleAndHold(parameters);
    }

    const char *moduleName() { return "SampleAndHold"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *source = m_source->output(last_fill, samples);
      const float *trigger = m_trigger->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(m_last_trigger < 0.1 && trigger[i] > 0.9)
          m_value = source[i];
        m_last_trigger = trigger[i];
        *output++ = m_value;
      }
      g_profiler.addTime("SampleAndHold", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_source->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
      validateWithin(*m_trigger, -1, 1);
    }
  private:
    Module *m_source;
    Module *m_trigger;
    bool m_waiting;
    float m_value;
    float m_last_trigger;
};


class Limiter : public Module
{
  public:
    Limiter(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_preamp = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Limiter(parameters);
    }

    const char *moduleName() { return "Limiter"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *preamp = m_preamp->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = tanh(input[i] * preamp[i]);
      }
      g_profiler.addTime("Limiter", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = -1, *out_max = 1;
    }

    void validateInputRange()
    {
      validateWithin(*m_preamp, 1, 10);
    }
  private:
    Module *m_input;
    Module *m_preamp;
};


class Rectifier : public Module
{
  public:
    Rectifier(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Rectifier(parameters);
    }

    const char *moduleName() { return "Rectifier"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = abs(input[i]);
      }
      g_profiler.addTime("Rectifier", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float imin, imax;
      m_input->getOutputRange(&imin, &imax);
      *out_min = 0, *out_max = max(abs(imin), abs(imax));
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
};


class Clipper : public Module
{
  public:
    Clipper(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_min = parameters[1]->m_module;
      m_max = parameters[2]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Clipper(parameters);
    }

    const char *moduleName() { return "Clipper"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *min = m_min->output(last_fill, samples);
      const float *max = m_max->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        float sample = input[i];
        if(sample > max[i]) sample = max[i];
        if(sample < min[i]) sample = min[i];
        *output++ = sample;
      }
      g_profiler.addTime("Clipper", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float min_min, min_max, max_min, max_max;
      m_min->getOutputRange(&min_min, &min_max);
      m_max->getOutputRange(&max_min, &max_max);
      *out_min = min(min_min, max_min), *out_max = max(min_max, max_max);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    Module *m_min;
    Module *m_max;
};


class BitCrusher : public Module
{
  public:
    BitCrusher(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_range = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new BitCrusher(parameters);
    }

    const char *moduleName() { return "BitCrusher"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *range = m_range->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = int(input[i] * range[i]) / range[i];
      }
      g_profiler.addTime("BitCrusher", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    Module *m_range;
};

// basically the same thing as slew limiter, but don't tell anybody!

class AttackRelease : public Module
{
  public:
    AttackRelease(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_attack = parameters[1]->m_module;
      m_release = parameters[2]->m_module;
      m_last = 0;
      m_current_attack = -1;
      m_current_release = -1;
      m_up = 0;
      m_down = 0;
      float dummy;
      m_input->getOutputRange(&m_last, &dummy);

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new AttackRelease(parameters);
    }

    const char *moduleName() { return "AttackRelease"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *attack = m_attack->output(last_fill, samples);
      const float *release = m_release->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(attack [i] != m_current_attack )
          m_current_attack   = attack [i], m_up   = onepoleCoefficient(attack [i]);
        if(release[i] != m_current_release)
          m_current_release  = release[i], m_down = onepoleCoefficient(release[i]);
        if(input[i] > m_last)
          m_last = m_last*m_up   + input[i]*(1-m_up  );
        else
          m_last = m_last*m_down + input[i]*(1-m_down);
        *output++ = m_last;
      }
      g_profiler.addTime("AttackRelease", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    Module *m_attack;
    Module *m_release;
    float m_last;
    float m_current_attack;
    float m_current_release;
    float m_up;
    float m_down;
};


class SlewLimiter : public Module
{
  public:
    SlewLimiter(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_up = parameters[1]->m_float;
      m_down = parameters[2]->m_float;
      m_last = 0;
      float dummy;
      m_input->getOutputRange(&m_last, &dummy);

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new SlewLimiter(parameters);
    }

    const char *moduleName() { return "SlewLimiter"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        if(input[i] > m_last) m_last = input[i]*m_up   + m_last*(1-m_up  );
        else                  m_last = input[i]*m_down + m_last*(1-m_down);
        *output++ = m_last;
      }
      g_profiler.addTime("SlewLimiter", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
    }
  private:
    Module *m_input;
    float m_up;
    float m_down;
    float m_last;
};


class NoteToFrequency : public Module
{
  public:
    NoteToFrequency(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_scale_name = parameters[1]->m_string;
      m_scale = 0;
      m_closest_notes = new char[128];
      for(int i=0; scales[i].name; i++)
        if(m_scale_name == scales[i].name)
          m_scale = (char *)(scales[i].steps);
      if(m_scale == 0)
        throw(ModuleExcept(string("Unknown scale: ")+m_scale_name));
      for(int note=0; note<128; note++)
      {
        int closest = -128;
        int step = 0;
        for(int trying=0; trying<128; trying+=m_scale[step])
        {
          if(m_scale[++step] == 0) step = 0;
          if(abs(trying-note) < abs(closest-note)) closest = trying;
        }
        m_closest_notes[note] = closest;
      }

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new NoteToFrequency(parameters);
    }

    const char *moduleName() { return "NoteToFrequency"; }
    ~NoteToFrequency() { delete[] m_closest_notes; }
    float freqFromNote(float note)
    {
      return pow(2.0, m_closest_notes[int(note)]/12.0) * note_0;
    }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = freqFromNote(input[i]);
      }
      g_profiler.addTime("NoteToFrequency", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float min, max;
      m_input->getOutputRange(&min, &max);
      *out_min = freqFromNote(min), *out_max = freqFromNote(max);
    }

    void validateInputRange()
    {
      validateWithin(*m_input, 0, 127);
    }
  private:
    Module *m_input;
    string m_scale_name;
    char* m_scale;
    char* m_closest_notes;
};


class MixDown : public Module
{
  public:
    MixDown(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_stereomodule;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new MixDown(parameters);
    }

    const char *moduleName() { return "MixDown"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = *input++ + *input++;
      }
      g_profiler.addTime("MixDown", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float min, max;
      m_input->getOutputRange(&min, &max);
      *out_min = min*2;
      *out_max = max*2;
    }

    void validateInputRange()
    {
    }
  private:
    StereoModule *m_input;
};


class Pan : public StereoModule
{
  public:
    Pan(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_module;
      m_position = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Pan(parameters);
    }

    const char *moduleName() { return "Pan"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *position = m_position->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = input[i] * (1.0-position[i]);
        *output++ = input[i] *      position[i];
      }
      g_profiler.addTime("Pan", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
      validateWithin(*m_position, 0, 1);
    }
  private:
    Module *m_input;
    Module *m_position;
};


class StereoAdd : public StereoModule
{
  public:
    StereoAdd(vector<ModuleParam *> parameters)
    {
      m_a = parameters[0]->m_stereomodule;
      m_b = parameters[1]->m_stereomodule;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new StereoAdd(parameters);
    }

    const char *moduleName() { return "StereoAdd"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *a = m_a->output(last_fill, samples);
      const float *b = m_b->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        *output++ = *a++ + *b++;
        *output++ = *a++ + *b++;
      }
      g_profiler.addTime("StereoAdd", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float a_min, a_max, b_min, b_max;
      m_a->getOutputRange(&a_min, &a_max);
      m_b->getOutputRange(&b_min, &b_max);
      *out_min = a_min + b_min;
      *out_max = a_max + b_max;
    }

    void validateInputRange()
    {
    }
  private:
    StereoModule *m_a;
    StereoModule *m_b;
};


class PingPongDelay : public StereoModule
{
  public:
    PingPongDelay(vector<ModuleParam *> parameters)
    {
      m_len = parameters[0]->m_float;
      m_filter = parameters[1]->m_float;
      m_input = parameters[2]->m_module;
      m_wet = parameters[3]->m_module;
      m_dry = parameters[4]->m_module;
      m_feedback = parameters[5]->m_module;
      m_length = int(m_len*sample_rate);
      m_buffer_left = new float[m_length];
      m_buffer_right = new float[m_length];
      m_read_pos = 1;
      m_write_pos = 0;
      m_last_sample_left = 0;
      m_last_sample_right = 0;
      memset(m_buffer_left, 0, sizeof(float)*m_length);
      memset(m_buffer_right, 0, sizeof(float)*m_length);

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new PingPongDelay(parameters);
    }

    const char *moduleName() { return "PingPongDelay"; }
    ~PingPongDelay()
    {
      delete[] m_buffer_left;
      delete[] m_buffer_right;
    }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *wet = m_wet->output(last_fill, samples);
      const float *dry = m_dry->output(last_fill, samples);
      const float *feedback = m_feedback->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        float left_sample = m_buffer_left[m_read_pos]*m_filter +
                            m_last_sample_left*(1-m_filter);
        m_last_sample_left = left_sample;
        m_buffer_left[m_write_pos] = input[i] +
                                     m_buffer_right[m_read_pos] * feedback[i];
        *output++ = input[i]*dry[i] + left_sample*wet[i];
        float right_sample = m_buffer_right[m_read_pos]*m_filter +
                            m_last_sample_right*(1-m_filter);
        m_last_sample_right = right_sample;
        m_buffer_right[m_write_pos] = m_buffer_left[m_read_pos] * feedback[i];
        *output++ = input[i]*dry[i] + right_sample*wet[i];
        if(++m_write_pos >= m_length) m_write_pos -= m_length;
        if(++m_read_pos  >= m_length) m_read_pos  -= m_length;
      }
      g_profiler.addTime("PingPongDelay", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      float min, max;
      m_input->getOutputRange(&min, &max);
      *out_min = min*2; // um, i think...
      *out_max = max*2;
    }

    void validateInputRange()
    {
      validateWithin(*m_wet, 0, 1);
      validateWithin(*m_dry, 0, 1);
      validateWithin(*m_feedback, 0, 1);
    }
  private:
    float m_len;
    float m_filter;
    Module *m_input;
    Module *m_wet;
    Module *m_dry;
    Module *m_feedback;
    int m_length;
    float* m_buffer_left;
    float* m_buffer_right;
    int m_read_pos;
    int m_write_pos;
    int m_last_sample_left;
    int m_last_sample_right;
};


class Rotate : public StereoModule
{
  public:
    Rotate(vector<ModuleParam *> parameters)
    {
      m_input = parameters[0]->m_stereomodule;
      m_angle = parameters[1]->m_module;

    }
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Rotate(parameters);
    }

    const char *moduleName() { return "Rotate"; }

    
    void fill(float last_fill, int samples)
    {
      float *output = m_output;
      const float *input = m_input->output(last_fill, samples);
      const float *angle = m_angle->output(last_fill, samples);

      double time = hires_time();
      for(int i=0; i<samples; i++)
      {
        float left  = *input++;
        float right = *input++;
        float cos_mul = cos(angle[i]), sin_mul = sin(angle[i]);
        *output++ = left*cos_mul + right*sin_mul;
        *output++ = left*sin_mul + right*cos_mul;
      }
      g_profiler.addTime("Rotate", hires_time()-time);
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input->getOutputRange(out_min, out_max);
    }

    void validateInputRange()
    {
    }
  private:
    StereoModule *m_input;
    Module *m_angle;
};

/*
class Add : public Module
{
  public:
    Add() {}
    Add(Module &a, Module &b)
    {
      addInput(a);
      addInput(b);
    }

    const char *moduleName() { return "Add"; }

    void addInput(Module &input) { m_inputs.push_back(&input); }

    void fill(float last_fill, int samples)
    {
      if(m_inputs.size() == 0) return;

      memcpy(m_output, m_inputs[0]->output(last_fill, samples),
             samples * sizeof(float));
      for(unsigned int i=1; i<m_inputs.size(); i++)
      {
        const float *input = m_inputs[i]->output(last_fill, samples);
        for(int j=0; j<samples; j++)
          m_output[j] += input[j];
      }
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      *out_min = 0, *out_max = 0;
      for(unsigned int i=0; i<m_inputs.size(); i++)
      {
        float inp_min, inp_max;
        m_inputs[i]->getOutputRange(&inp_min, &inp_max);
        *out_min += inp_min;
        *out_max += inp_max;
      }
    }

    void validateInputRange() {} // don't care

  private:
    std::vector<Module *> m_inputs;
};

// maybe support sweeping the length too?  i bet that would sound awesome
// speed parameter currently unimplemented
class Delay : public Module
{
  public:
    Delay(float length, float filter, Module &input, Module &wet, Module &dry,
          Module &feedback, Module &speed)
    : m_buffer(new float[int(length*sample_rate)]),
      m_length(int(length*sample_rate)),
      m_read_pos(1), m_write_pos(0), m_last_sample(0), m_filter(filter),
      m_input(input), m_wet(wet), m_dry(dry), m_feedback(feedback),
      m_speed(speed)
    {
      memset(m_buffer, 0, m_length*sizeof(float));
    }

    ~Delay()
    {
      delete[] m_buffer;
    }

    const char *moduleName() { return "Delay"; }

    void fill(float last_fill, int samples)
    {
      const float *input    = m_input   .output(last_fill, samples);
      const float *wet      = m_wet     .output(last_fill, samples);
      const float *dry      = m_dry     .output(last_fill, samples);
      const float *feedback = m_feedback.output(last_fill, samples);
      const float *speed    = m_speed   .output(last_fill, samples);

      for(int i=0; i<samples; i++)
      {
        float sample = m_buffer[int(floor(m_read_pos))]*m_filter +
                       m_last_sample*(1-m_filter);
        m_last_sample = sample;
        m_output[i] = input[i]*dry[i] + sample * wet[i];
        m_buffer[m_write_pos] = input[i] + sample * feedback[i];

        if(++m_write_pos >= m_length) m_write_pos -= m_length;
        if(++m_read_pos  >= m_length) m_read_pos  -= m_length;
      }
    }

    void getOutputRange(float *out_min, float *out_max)
    {
      m_input.getOutputRange(out_min, out_max); // wrong for extreme settings
    }

    void validateInputRange()
    {
      validateWithin(m_wet,      0, 1);
      validateWithin(m_dry,      0, 1);
      validateWithin(m_feedback, 0, 1);
      validateWithin(m_speed,    0, 2);
    }

  private:
    float *m_buffer;
    int m_length;
    float m_filter;
    float m_read_pos;
    float m_last_sample;
    int m_write_pos;

    Module &m_input, &m_wet, &m_dry, &m_feedback, &m_speed;
};
*/
void fillModuleList()
{
  g_module_infos["Saw"] = new ModuleInfo("Saw", Saw::create);
  g_module_infos["Saw"]->addParameter("frequency", "Module");
  g_module_infos["Saw"]->addParameter("retrigger", "Module", 0);
  g_module_infos["Pulse"] = new ModuleInfo("Pulse", Pulse::create);
  g_module_infos["Pulse"]->addParameter("frequency", "Module");
  g_module_infos["Pulse"]->addParameter("pulsewidth", "Module", 0.5);
  g_module_infos["Pulse"]->addParameter("retrigger", "Module", 0);
  g_module_infos["Sine"] = new ModuleInfo("Sine", Sine::create);
  g_module_infos["Sine"]->addParameter("frequency", "Module");
  g_module_infos["Sine"]->addParameter("retrigger", "Module", 0);
  g_module_infos["Sine"]->addParameter("phase_offset", "Module", 0);
  g_module_infos["Triangle"] = new ModuleInfo("Triangle", Triangle::create);
  g_module_infos["Triangle"]->addParameter("frequency", "Module");
  g_module_infos["Triangle"]->addParameter("retrigger", "Module", 0);
  g_module_infos["Noise"] = new ModuleInfo("Noise", Noise::create);
  g_module_infos["Within"] = new ModuleInfo("Within", Within::create);
  g_module_infos["Within"]->addParameter("input", "Module");
  g_module_infos["Within"]->addParameter("min", "float");
  g_module_infos["Within"]->addParameter("max", "float");
  g_module_infos["Sample"] = new ModuleInfo("Sample", Sample::create);
  g_module_infos["Sample"]->addParameter("sample_name", "string");
  g_module_infos["Sample"]->addParameter("frequency", "Module");
  g_module_infos["Sample"]->addParameter("trigger", "Module", 0);
  g_module_infos["Sample"]->addParameter("loop_start", "Module", 0);
  g_module_infos["Sample"]->addParameter("loop_end", "Module", 1);
  g_module_infos["Rescaler"] = new ModuleInfo("Rescaler", Rescaler::create);
  g_module_infos["Rescaler"]->addParameter("input", "Module");
  g_module_infos["Rescaler"]->addParameter("to_min", "float");
  g_module_infos["Rescaler"]->addParameter("to_max", "float");
  g_module_infos["Filter"] = new ModuleInfo("Filter", Filter::create);
  g_module_infos["Filter"]->addParameter("input", "Module");
  g_module_infos["Filter"]->addParameter("cutoff", "Module");
  g_module_infos["Filter"]->addParameter("resonance", "Module");
  g_module_infos["Multiply"] = new ModuleInfo("Multiply", Multiply::create);
  g_module_infos["Multiply"]->addParameter("a", "Module");
  g_module_infos["Multiply"]->addParameter("b", "Module");
  g_module_infos["Add"] = new ModuleInfo("Add", Add::create);
  g_module_infos["Add"]->addParameter("a", "Module");
  g_module_infos["Add"]->addParameter("b", "Module");
  g_module_infos["Quantize"] = new ModuleInfo("Quantize", Quantize::create);
  g_module_infos["Quantize"]->addParameter("input", "Module");
  g_module_infos["EnvelopeGenerator"] = new ModuleInfo("EnvelopeGenerator", EnvelopeGenerator::create);
  g_module_infos["EnvelopeGenerator"]->addParameter("gate", "Module");
  g_module_infos["EnvelopeGenerator"]->addParameter("att", "float");
  g_module_infos["EnvelopeGenerator"]->addParameter("dec", "float");
  g_module_infos["EnvelopeGenerator"]->addParameter("sus", "float");
  g_module_infos["EnvelopeGenerator"]->addParameter("rel", "float");
  g_module_infos["SampleAndHold"] = new ModuleInfo("SampleAndHold", SampleAndHold::create);
  g_module_infos["SampleAndHold"]->addParameter("source", "Module");
  g_module_infos["SampleAndHold"]->addParameter("trigger", "Module");
  g_module_infos["Limiter"] = new ModuleInfo("Limiter", Limiter::create);
  g_module_infos["Limiter"]->addParameter("input", "Module");
  g_module_infos["Limiter"]->addParameter("preamp", "Module");
  g_module_infos["Rectifier"] = new ModuleInfo("Rectifier", Rectifier::create);
  g_module_infos["Rectifier"]->addParameter("input", "Module");
  g_module_infos["Clipper"] = new ModuleInfo("Clipper", Clipper::create);
  g_module_infos["Clipper"]->addParameter("input", "Module");
  g_module_infos["Clipper"]->addParameter("min", "Module");
  g_module_infos["Clipper"]->addParameter("max", "Module");
  g_module_infos["BitCrusher"] = new ModuleInfo("BitCrusher", BitCrusher::create);
  g_module_infos["BitCrusher"]->addParameter("input", "Module");
  g_module_infos["BitCrusher"]->addParameter("range", "Module");
  g_module_infos["AttackRelease"] = new ModuleInfo("AttackRelease", AttackRelease::create);
  g_module_infos["AttackRelease"]->addParameter("input", "Module");
  g_module_infos["AttackRelease"]->addParameter("attack", "Module");
  g_module_infos["AttackRelease"]->addParameter("release", "Module");
  g_module_infos["SlewLimiter"] = new ModuleInfo("SlewLimiter", SlewLimiter::create);
  g_module_infos["SlewLimiter"]->addParameter("input", "Module");
  g_module_infos["SlewLimiter"]->addParameter("up", "float");
  g_module_infos["SlewLimiter"]->addParameter("down", "float");
  g_module_infos["NoteToFrequency"] = new ModuleInfo("NoteToFrequency", NoteToFrequency::create);
  g_module_infos["NoteToFrequency"]->addParameter("input", "Module");
  g_module_infos["NoteToFrequency"]->addParameter("scale_name", "string");
  g_module_infos["MixDown"] = new ModuleInfo("MixDown", MixDown::create);
  g_module_infos["MixDown"]->addParameter("input", "StereoModule");
  g_module_infos["Pan"] = new ModuleInfo("Pan", Pan::create);
  g_module_infos["Pan"]->addParameter("input", "Module");
  g_module_infos["Pan"]->addParameter("position", "Module");
  g_module_infos["StereoAdd"] = new ModuleInfo("StereoAdd", StereoAdd::create);
  g_module_infos["StereoAdd"]->addParameter("a", "StereoModule");
  g_module_infos["StereoAdd"]->addParameter("b", "StereoModule");
  g_module_infos["PingPongDelay"] = new ModuleInfo("PingPongDelay", PingPongDelay::create);
  g_module_infos["PingPongDelay"]->addParameter("len", "float");
  g_module_infos["PingPongDelay"]->addParameter("filter", "float");
  g_module_infos["PingPongDelay"]->addParameter("input", "Module");
  g_module_infos["PingPongDelay"]->addParameter("wet", "Module");
  g_module_infos["PingPongDelay"]->addParameter("dry", "Module");
  g_module_infos["PingPongDelay"]->addParameter("feedback", "Module");
  g_module_infos["Rotate"] = new ModuleInfo("Rotate", Rotate::create);
  g_module_infos["Rotate"]->addParameter("input", "StereoModule");
  g_module_infos["Rotate"]->addParameter("angle", "Module");
}
