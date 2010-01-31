#include <dsound.h>
#include <windows.h>
#include <winerror.h>
#include <dxerr.h>
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
  static Constant lfo_frequency(4);
  static Constant lfo_amplitude(10);
  static Constant lfo_offset(500);
  static Constant lfo_offset2(500*1.5);
  static Constant unit(.25);
  static Sine lfo(lfo_frequency.output(), lfo_amplitude.output());
  static Mixer lfo_final;
  static Mixer lfo2_final;
  static Triangle triangle(lfo_final.output(), unit.output());
  static Triangle triangle2(lfo2_final.output(), unit.output());
  static Mixer triangle_mix;
  static Overdrive output(triangle_mix.output());
  
  static bool setup = false;
  if(!setup)
  {
    lfo_final.addInput(lfo.output());
    lfo_final.addInput(lfo_offset.output());
    lfo2_final.addInput(lfo.output());
    lfo2_final.addInput(lfo_offset2.output());
    triangle_mix.addInput(triangle.output());
    triangle_mix.addInput(triangle2.output());    
    setup = true;
  }
  
  lfo_frequency.fill(samples);
  lfo_amplitude.fill(samples);
  lfo_offset.fill(samples);
  lfo_offset2.fill(samples);
  unit.fill(samples);
  lfo.fill(samples);
  lfo_final.fill(samples);
  lfo2_final.fill(samples);
  triangle.fill(samples);
  triangle2.fill(samples);
  triangle_mix.fill(samples);
  output.fill(samples);
  
  for(int i=0; i<samples; i++)
  {
    const float *o = output.output();
    *buffer++ = short(o[i] * 32767);
    *buffer++ = short(o[i] * 32767);
  }
  
/*  static float oscillator = -pi;
  static float pitch = 500;
  for(int i=0; i<samples; i++)
  {
    short sample = short(sin(oscillator) * 32767);
    *buffer++ = sample;
    *buffer++ = sample;
    oscillator += pitch / 44100;
    while(oscillator > pi) oscillator -= 2*pi;
    pitch += 0.5f;
    while(pitch >= 10000) pitch -= 9000;    
  }*/
}

IDirectSoundBuffer *buffer;
HANDLE notification_event;

void setupSound(unsigned int buffer_size)
{
  IDirectSound8 *dsound;
  if(FAILED(DirectSoundCreate8(0, &dsound, 0)))
    throw "couldn't create dsound interface";
  if(FAILED(dsound->SetCooperativeLevel(GetConsoleWindow(), DSSCL_PRIORITY)))
    throw "couldn't set dsound cooperative level";
    
  WAVEFORMATEX wave_format =
  {
    WAVE_FORMAT_PCM, // format tag
    2,               // channel count
    44100,           // sample rate
    44100*4,         // bytes per second
    4,               // block alignment (bytes per sample * channel count)
    16,              // bits per sample
    0                // size of additional data in this descriptor
  };
  
  DSBUFFERDESC buffer_descriptor = 
  {
    sizeof(DSBUFFERDESC), // size of this descriptor
    DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY,
    buffer_size*8,        // size of buffer
    0,                    // "reserved"
    &wave_format,         // wave format descriptor
    DS3DALG_DEFAULT       // 3d virtualization algorithm
  };
  
  
  if(FAILED(dsound->CreateSoundBuffer(&buffer_descriptor, &buffer, 0)))
    throw "couldn't create dsound buffer";  
  
  IDirectSoundNotify *notify;
  if(FAILED(buffer->QueryInterface(IID_IDirectSoundNotify, 
                                   (void **)&notify)))
    throw "couldn't create dsound notify interface";
  
  notification_event = CreateEvent(0, false, false, 0);
  if(!notification_event) throw "couldn't create notification event";
  
  const int notify_count = 8;
  DSBPOSITIONNOTIFY notify_positions[notify_count];
  for(int i=0; i<8; i++)
  {
    notify_positions[i].dwOffset     = i * buffer_size * 8 / notify_count;
    notify_positions[i].hEventNotify = notification_event;
  }
  
  if(FAILED(notify->SetNotificationPositions(notify_count, notify_positions)))
    throw "couldn't set dsound notification positions";
  
  void *write_buffer;
  DWORD write_buffer_size;
  if(FAILED(buffer->Lock(0, 0, &write_buffer, &write_buffer_size,
                         0, 0, DSBLOCK_ENTIREBUFFER)))
    throw "couldn't lock dsound buffer";

  produceStream((short *)write_buffer, write_buffer_size/4);

  if(FAILED(buffer->Unlock(write_buffer, write_buffer_size, 0, 0)))
    throw "couldn't unlock dsound buffer";  

  if(FAILED(buffer->Play(0, 0, DSBPLAY_LOOPING)))
    throw "couldn't play dsound buffer";
}

void streamSound(unsigned int buffer_size)
{
  unsigned int next_write_position = 0;
  
  for(;;)
  {
    DWORD event = MsgWaitForMultipleObjects(1, &notification_event, false,
                                            INFINITE,  QS_ALLEVENTS);
    if(event == WAIT_OBJECT_0) // time to stream more audio
    {
      DWORD write_window_start, write_window_end;
      buffer->GetCurrentPosition(&write_window_end, &write_window_start);
      write_window_start /= 4;
      write_window_end   /= 4;

      if(write_window_end == 0) write_window_end = buffer_size*2-1;
      else write_window_end--;

      next_write_position %= (buffer_size*2);
//      printf("nwp:%d, wws:%d, wwe:%d\n", next_write_position, write_window_start, write_window_end);
        
      if(write_window_end < write_window_start) write_window_end +=
        buffer_size*2;
      
      if(next_write_position < write_window_start) next_write_position +=
        buffer_size*2;
        
      if(next_write_position < write_window_end)
      {
        int write_length = write_window_end-next_write_position;
        next_write_position %= (buffer_size*2);
        
        void  *write_buffers     [2];
        DWORD  write_buffer_sizes[2];
        if(FAILED(buffer->Lock(next_write_position*4, write_length*4,
                               write_buffers,         write_buffer_sizes,
                               write_buffers+1,       write_buffer_sizes+1, 0)))
          throw "couldn't lock dsound buffer";
        
//        printf("[%p, %d], [%p, %d]\n", write_buffers[0], write_buffer_sizes[0],
//                                       write_buffers[1], write_buffer_sizes[1]);

        for(int i=0; i<2; i++)
          if(write_buffers[i])
            produceStream((short *)(write_buffers[i]),
                                    write_buffer_sizes[i]/4);
   
        if(FAILED(buffer->Unlock(write_buffers[0], write_buffer_sizes[0], 
                                 write_buffers[1], write_buffer_sizes[1])))
          throw "couldn't unlock dsound buffer";
          
        next_write_position += write_length;
      }      
    }
    else // other crap
    {
      MSG message;
      while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
      { 
        TranslateMessage(&message); 
        DispatchMessage (&message); 
      }
    }
  }
}

void makeNoise(unsigned int buffer_size)
{ 
  setupSound (buffer_size);
  streamSound(buffer_size);
}