/* hey hey hey -- better idea than just polyphony for multitouch; additional fingers
   provide additional axes of input!  e.g. x, y, touch, angle, distance?

   multiplex modules: must take at least one vectors of buffers in, maintains
   multiple channels for output, but can mixdown if a mono module requests.

   what has to be multiplexed?

   maybe we really should treat stereo as a special case of multiplexing :/
   if so, creating and destroying channels might not be reasonable

   should be a way to create and destroy channels.  presumably create on
   note-on, how do we know when to destroy?  some way to flag, like when the
   envgen goes idle?  note-off and n samples of silence?  should be after the
   reverb trails off.  can't check for that, though, because we're not
   reverbing each channel individually.

   Todo:
   - interpolated sample playback
   - switch
   - DADSR, parameterized shape
   - wave terrain
   - support for arbitrary parameter count
     + sequencer (retriggerable)
   - additional filters?  eq?
   - support relative bending after touchdown
   - multiple intonations!  just, meantone, quarter tone, well-tempered, etc.
   - replay output from wave, to find nasty clicks (wave streamer)
   - oscillator band-limiting (http://www.fly.net/~ant/bl-synth/ ?)
   - reverb
   - multiplexing subsystem?
   Done:
   - x/y input
   - scale quantizer -- actually, "scale" should be a parameter of
     "notetofrequency"!
   - stereo
   - pan module
   - stereoadd module
   - ping pong delay
   - stereo rotate
   - hard/soft-limiter (overdrive)
   - panner
   - output range calculation
   - rename unitscaler to rescaler; make it scale input range to new range
   - input range validation
   - module definition preprocessor
   - text file patch definitions
   - escape to quit
   - patch selection from command line
   - output logging from command line
   - reimplemented: limiter, rectifier, clipper, stereo rotate, slew limiter
   - sample playback (wave reader)
   - oscillator hard sync
   - exponential envgen
   - bitcrusher
   - default parameter support
   - phase modulation
*/
#include "synth.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "io.h"
#include "exception.h"
#include "input.h"

using namespace std;

#ifdef _WIN32
#define snprintf _snprintf
#endif //_WIN32

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4244) // double conversion to float
#pragma warning(disable:4305) // double conversion to float
#pragma warning(disable:4267) // size_t conversion to int
#pragma warning(disable:4996) // snprintf() purportedly unsafe

const float pi = 3.1415926535897932384626f, e = 2.71828183f;
const float note_0 = 8.1757989156;
const float middle_c = 261.625565;
const int g_sample_rate = 44100;

const char *g_keys[] =
{
  "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb",
  "B"
};

scale g_scales[] =
{
  { "major",      "\2\2\1\2\2\2\1" },
  { "minor",      "\2\1\2\2\1\2\2" },
  { "dorian",     "\2\1\2\2\2\1\2" },
  { "phrygian",   "\1\2\2\2\1\2\2" },
  { "lydian",     "\2\2\2\1\2\2\1" },
  { "mixolydian", "\2\2\1\2\2\1\2" },
  { "locrian",    "\1\2\2\1\2\2\2" },
  { "pentatonic", "\2\2\3\2\3" },
  { "pentminor",  "\3\2\2\3\2" },
  { "chromatic",  "\1" },
  { "whole",      "\2" },
  { "minor3rd",   "\3" },
  { "3rd",        "\4" },
  { "4th",        "\5" },
  { "tritone",    "\6" },
  { "5th",        "\7" },
  { "octave",     "\14" },
  { 0,            0 }
};

int g_key = 0;
char g_closest_notes[128];
const char* g_patch("pad");
int g_octave_range = 2;
int g_start_octave = 4;

EXCEPTION_D(UnhandledWaveFormatException, Exception,
            "Can only handle 16-bit PCM .wav files")
static unsigned char wave_header[] =
{
  'R', 'I', 'F', 'F',
  0x00, 0x00, 0x00, 0x00, // wave size+36
  'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, // PCM
  0x01, 0x00, // channels
  0x44, 0xAC, 0x00, 0x00, // 44.1khz
  0x10, 0xB1, 0x02, 0x00, // 176400 bytes/sec
  0x02, 0x00, // bytes per sample*channels (mono by default)
  0x10, 0x00, // 16 bits
  'd', 'a', 't', 'a',
  0x00, 0x00, 0x00, 0x00, // wave size
};

class WaveIn
{
  public:
    WaveIn(const FileRef filename)
    {
      File in(filename, File::READ);
      static unsigned char header[sizeof(wave_header)];
      in.read(header, sizeof(wave_header), 1);

      if(memcmp(header, wave_header, 4) || memcmp(header+8, wave_header+8, 8) ||
         memcmp(header+20, wave_header+20, 2) ||
         memcmp(header+34, wave_header+34, 6))
        throw(UnhandledWaveFormatException(filename));
      
      memcpy(&m_native_sample_rate, header+24, 4);
      memcpy(&m_length, header+40, 4);      
      m_length /= 2;
      m_stereo = header[22]==2 ? true : false;
      
      m_buffer = new float[m_length];
      static short buf[1024];
      for(int i=0; !in.eof() && i<m_length; i+=1024)
      {
        int len = min(1024, m_length-i);
        in.read(buf, 2, len);
        for(int j=0; j<len; j++) m_buffer[i+j] = buf[j]/32768.0;
      }
      in.close();
    }
    ~WaveIn() { delete m_buffer; }
    
    int length() const { return m_length; }
    int nativeSampleRate() const { return m_native_sample_rate; }
    float valueAt(float position)
    {
      return m_buffer[int(position*(m_length-1))];
    }
    
  private:
    float *m_buffer;
    int    m_length;
    int    m_native_sample_rate;
    bool   m_stereo;
};

class WaveOut
{
  public:
    WaveOut(const string filename, int sample_rate, float scaler=32768,
            bool stereo=false)
    : m_scaler(scaler), m_length(0), m_out(filename, File::WRITE)
    {
      if(stereo) wave_header[22] = 0x02, wave_header[32] = 0x04;
      else       wave_header[22] = 0x01, wave_header[32] = 0x02;
      m_out.write(wave_header, sizeof(wave_header), 1);
    }

    ~WaveOut() { close(); }

    void close()
    {
      updateLength();
      m_out.close();
    }

    void writeBuffer(const float *buffer, int size)
    {
      static short out[max_buffer_size];
      for(int i=0; i<size; i++) out[i] = short(buffer[i]*m_scaler);
      m_out.write(out, 2, size);
      m_length += size;

      updateLength();
    }

  private:
    void updateLength()
    {
      int chunklength = m_length*2;
      m_out.seek(40);
      m_out.write(&chunklength, 4, 1);

      chunklength += 36;
      m_out.seek(4);
      m_out.write(&chunklength, 4, 1);

      m_out.seekEnd();
    }

    File  m_out;
    float m_scaler;
    int   m_length;
};

EXCEPTION(MismatchedProfilePop, Exception, "MismatchedProfilePop")

bool sortby(const pair<double, string> &a, const pair<double, string> &b)
{
  return a.first > b.first;
}

class Profiler
{
  public:
    char *describeTimeSpent(double audio_time)
    {
      static char buf[4096];
      buf[0] = 0;
      
      vector<pair<double, string> > mlist;
        
      for(map<string, double>::iterator i=m_time_spent.begin();
          i!=m_time_spent.end(); i++)
        mlist.push_back(pair<double, string>(i->second, i->first));

      sort(mlist.begin(), mlist.end(), sortby);

      double total_time = 0;
      for(vector<pair<double, string> >::iterator i = mlist.begin();
          i!=mlist.end(); i++)
        total_time += i->first;

      for(vector<pair<double, string> >::iterator i = mlist.begin();
          i!=mlist.end(); i++)
        sprintf(buf+strlen(buf), "%g (%2.2f%%) %s\n", i->first,
                i->first/total_time*100, i->second.c_str());
      sprintf(buf+strlen(buf), "total: %.2fs CPU / %.2fs audio (%g%%)",
              total_time, audio_time, total_time*100/audio_time);
      return buf;
    }
    
    void clear()
    {
      m_time_spent.clear();
    }
    
    void addTime(string name, double time)
    {
      if(m_time_spent.count(name) == 0) m_time_spent[name] = time;
      else m_time_spent[name] += time;
    }
  
    map<string, double> m_time_spent;
};

Profiler g_profiler;

EXCEPTION_D(InvalidInputRangeExcept,  Exception, "Invalid Input Range")
EXCEPTION_D(InvalidOutputRangeExcept, Exception, "Invalid Output Range")

inline float interpolate(float a, float b, float frac)
{
  return b*frac + a*(1.0-frac);
}

        // sample rate, <buffer,  previous sample>
typedef map<int,    pair<float *, float> > ResampleBufferMap;

class Module
{
  public:
    Module() : m_last_fill(0), m_waveout(0), m_sample_rate(g_sample_rate)
    {
      memset(m_output, 0, max_buffer_size*sizeof(float));
      //memset(&m_within_lower_bound, 0xff, sizeof(float));
      //memset(&m_within_upper_bound, 0xff, sizeof(float));
    }
    ~Module()
    {
      if(m_waveout) delete m_waveout;
      for(ResampleBufferMap::iterator i = m_converted_outputs.begin();
          i != m_converted_outputs.end(); i++)
        delete[] i->second.first; // free buffer
    }
    
    void setName(string name) { m_name = name; }
    void setSampleRate(int sample_rate) { m_sample_rate = sample_rate; }

    virtual void fill(float last_fill, int samples) = 0;
    virtual void getOutputRange(float *out_min, float *out_max) = 0;
    virtual const char *moduleName() = 0;
    virtual void validateInputRange() = 0;
    virtual bool stereo() const { return false; }    

    virtual void convertToRate(int output_rate, int samples)
    {      
      double start_time = hires_time();
      
      int from = max(1.0f, float(samples)*m_sample_rate/g_sample_rate);
      int to   = max(1.0f, float(samples)*  output_rate/g_sample_rate);
      float *buf      = m_converted_outputs[output_rate].first;
      float  position = m_converted_outputs[output_rate].second;
      if(m_last_fill == 0) position = m_output[0];
      
      if(from==1) for(int i=0; i<to; i++) *buf++ = *m_output;        
      else if(to < from) // downsample
      {
        float delta = float(from)/to;
        float pos   = 0.0;
        for(int i=0; i<to; i++)
        {
          *buf++ = m_output[int(pos)];
          pos += delta;   
        }
      }
      else // upsample
      {
        int per = to/from;
        int segments = from;

        for(int i=0; i<from; i++)
        {
          if(i+1 == from) per += to - per*from;
          float delta = (m_output[i] - position) / per;
          for(int j=0; j<per-1; j++)
          {
            *buf++ = position;
            position += delta;
          }
          position = m_output[i];
          *buf++ = position;
        }
      }
      m_converted_outputs[output_rate].second = position;
      
      g_profiler.addTime("Resampling", hires_time() - start_time);
    }

    virtual const float *output(float last_fill, int samples, int output_rate)
    {
      if(m_last_fill < last_fill)
      {
        validateInputRange();
        fill(last_fill, samples);
        int sample_count = max(1.0f, samples*m_sample_rate/g_sample_rate);
        if(m_waveout) m_waveout->writeBuffer(m_output, sample_count);
        #ifndef NDEBUG
        validateOutputRange(m_output, sample_count);
        #endif
        
        for(ResampleBufferMap::iterator i = m_converted_outputs.begin();
            i != m_converted_outputs.end(); i++)
          convertToRate(i->first, samples);
      }
      
      if(output_rate != m_sample_rate && m_converted_outputs.count(output_rate) == 0)
      {
        m_converted_outputs[output_rate] =
          pair<float *, float>(new float[max_buffer_size], 0);
        convertToRate(output_rate, samples);
      }
      
      m_last_fill = last_fill;
      
      if(output_rate == m_sample_rate) return m_output;
      return m_converted_outputs[output_rate].first;
    }

    virtual void createLog(const string filename)
    {
      float min, max;
      getOutputRange(&min, &max);

      ostringstream fn;
      fn << filename << " (" << min << " to " << max << ").wav";

      if(-min > max) max = -min;
      if(!m_waveout)
        m_waveout = new WaveOut(fn.str(), m_sample_rate, 32767/max);
    }

    void validateWithin(Module &input, float min, float max)
    {
      float inp_min, inp_max;
      input.getOutputRange(&inp_min, &inp_max);
      if(inp_min < min || inp_max > max)
      {
        static char error[256];
        snprintf(error, 255,
                  "%s (%g, %g) for %s (%g, %g)",
                  input.moduleName(), inp_min, inp_max, moduleName(), min, max);
        throw(InvalidInputRangeExcept(error));
      }
    }

    void validateOutputRange(float *buffer, int samples)
    {
      float min, max;
      getOutputRange(&min, &max);
      for(int i=0; i<samples; i++)
      {
        if(buffer[i] < min || buffer[i] > max)
        {
          static char error[256];
          getOutputRange(&min, &max);
          snprintf(error, 255, "%s: %g <= %g <= %g",
                   moduleName(), min, m_output[i], max);
          throw(InvalidOutputRangeExcept(error));
        }
/*        float upper_bound_distance = abs(max-m_output[i]);
        float lower_bound_distance = abs(min-m_output[i]);
        if(_isnan(m_within_upper_bound) || upper_bound_distance<m_within_upper_bound) m_within_upper_bound = upper_bound_distance;
        if(_isnan(m_within_lower_bound) || lower_bound_distance<m_within_lower_bound) m_within_lower_bound = lower_bound_distance; */
      }
    }

  protected:
    float m_output[max_buffer_size];
    ResampleBufferMap m_converted_outputs;
    float m_last_fill;
    float m_sample_rate;
    string m_name;
    //float m_within_lower_bound;
    //float m_within_upper_bound;
    WaveOut *m_waveout;
};

class StereoModule : public Module
{
  public:
    StereoModule() : Module()
    {
      memset(m_output, 0, max_buffer_size*2*sizeof(float));
    }

    bool stereo() const { return true; }

    const float *output(float last_fill, int samples, int at_rate)
    {
      if(at_rate != g_sample_rate) ;//throw "Ugh";
      if(m_last_fill < last_fill)
      {
        fill(last_fill, samples);
        m_last_fill = last_fill;
        if(m_waveout) m_waveout->writeBuffer(m_output, samples*2);
#ifndef NDEBUG
        validateOutputRange(m_output, samples*2);
#endif
      }

      return m_output;
    }

    void createLog(const string filename)
    {
      float min, max;
      getOutputRange(&min, &max);
      ostringstream fn;
      fn << filename << " (" << min << " to " << max << ").wav";
      if(-min > max) max = -min;
      if(!m_waveout)
        m_waveout = new WaveOut(fn.str(), m_sample_rate, 32767/max, true);
    }

  protected:
    float m_output[max_buffer_size*2];
};

struct ModuleParam
{
  Module       *m_module;
  StereoModule *m_stereomodule;
  float         m_float;
  int           m_int;
  string        m_string;
};

class Constant : public Module
{
  public:
    Constant(float value) { setValue(value); }
  
    const char *moduleName() { return "Constant"; }

    void fill(float last_fill, int samples) {}

    void setValue(float value)
    {
      for(int i=0; i<max_buffer_size; i++) m_output[i] = value;
    }
 
    void validateInputRange() {} // no input

    void getOutputRange(float *out_min, float *out_max)
    {      
      *out_min = *out_max = m_output[0];
    }
};

class Input : public Module
{
  public:
    Input(int axis) : m_axis(axis) {}
    
    static Module *create(vector<ModuleParam *> parameters)
    {
      return new Input(parameters[0]->m_int);
    }
    
    const char *moduleName() { return "Input"; }

    void fill(float last_fill, int samples)
    {
      //double start = hires_time();      
      readInputAxis(m_axis, m_output, samples);
      //g_profiler.addTime("Input", hires_time() - start);
    }
  
    void getOutputRange(float *out_min, float *out_max)
    {      
      *out_min = (m_axis==2)?0:-1;
      *out_max = 1;
    }

    void validateInputRange() {} // no input

  private:
    int m_axis;
};

class Curve
{
  public:
    Curve() : m_coefficient(0), m_start(0), m_end(0), m_level(0) {}
    
    void start(float start, float end, float time, int sample_rate)
    {
      m_start       = start;
      m_end         = end;
      m_level       = 1.0f;
      m_coefficient = (log(0.00001f)-log(1.0f)) / (time*sample_rate);
    }
    
    void step()
    {
      m_level += m_coefficient*m_level;
      if(m_level < 0.00001f) m_level = 0.0;
    }
    
    float position() const { return (m_start-m_end) * m_level + m_end; }
    
    bool finished() const { return m_level < 0.00001f; }

  private:
    float m_level;
    float m_coefficient;
    float m_start, m_end;  
};

EXCEPTION(ParseExcept, Exception, "Parse Error")
EXCEPTION(TooManyParamsExcept, ParseExcept, "Too many parameters")

struct ModuleParamInfo
{
  string name, type;
  bool   hasdefault;
  float  defaultvalue;
};

class ModuleInfo
{
  public:
    ModuleInfo(string name, Module *(*instantiator)(vector<ModuleParam *>))
    : m_name(name), m_instantiator(instantiator) {}

    void addParameter(string name, string type, float defaultvalue)
    {
      ModuleParamInfo m;
      m.name = name;
      m.type = type;
      m.hasdefault = true;
      m.defaultvalue = defaultvalue;
      m_parameters.push_back(m);
    }
    
    void addParameter(string name, string type)
    {
      ModuleParamInfo m;
      m.name = name;
      m.type = type;
      m.hasdefault = false;
      m_parameters.push_back(m);
    }
    
    const ModuleParamInfo parameter(unsigned int n) const
    {
      if(n>=m_parameters.size())
        throw(TooManyParamsExcept());
      return m_parameters[n];
    }
    
    int parameterCount() const { return m_parameters.size(); }
    
    Module *instantiate(vector<ModuleParam *> parameters)
    {
      return m_instantiator(parameters);
    }
    
  private:
    string m_name;
    vector<ModuleParamInfo> m_parameters;
    Module *(*m_instantiator)(vector<ModuleParam *> parameters);
};

// hardcoded to 44100khz.  rescale to other sample rates by multiplying time?
float onepoleCoefficient(float time)
{
  return (360481000.0*pow(time, 1.1f) + 1) /
         (360481000.0*pow(time, 1.1f) + 100000);
}

inline float freqFromX(float x)
{
  int note = (x+1)/2*g_octave_range*12+g_start_octave*12;
  return pow(2.0, (g_closest_notes[note]+g_key)/12.0) * note_0;
}

map<string, ModuleInfo *> g_module_infos;

EXCEPTION_D(ModuleExcept, Exception, "Module exception")

#include "tables.cpp"
#include "modules_generated.cpp"

map<string, Module *> g_modules;

EXCEPTION_D(UnknownModuleTypeExcept, ParseExcept, "Unknown module type")
EXCEPTION_D(ExpectingFloatExcept,    ParseExcept, "Expecting a float")
EXCEPTION_D(ExpectingIntExcept,      ParseExcept, "Expecting an int")
EXCEPTION_D(ExpectingModuleExcept,   ParseExcept, "Expecting module name")
EXCEPTION_D(ExpectingStringExcept,   ParseExcept, "Expecting string")
EXCEPTION_D(UnknownModuleExcept,     ParseExcept, "Unknown module instance")
EXCEPTION_D(UnknownTypeExcept,       ParseExcept, "Unknown parameter type")
EXCEPTION_D(NotStereoExcept,         ParseExcept, "Module not stereo")
EXCEPTION_D(NotMonoExcept,           ParseExcept, "Module not mono")
EXCEPTION_D(TooFewParamsExcept,      ParseExcept, "Too few parameters")
EXCEPTION_D(TooManyParamsExcept_D,   ParseExcept, "Too many parameters")
EXCEPTION_D(ReusedModuleNameExcept,  ParseExcept, "Reused Module Name")
EXCEPTION_D(BadStringExcept,         ParseExcept,
            "Bad string (no ,() or whitespace allowed)")

void addModule(char *definition)
{
  string def_copy = definition;

  const char *delim = ",() \r\n\t";
  char *comment = strchr(definition, '#');
  if(comment) *comment = 0;
  char *t = strtok(definition, delim);
  if(!t) return;
  
  string name = "", type = "";
  int at_rate = 44100;
  
  vector<ModuleParam *> params;
  do
  {
    if(type == "")
    {
      if(isdigit(*t))
      {
        at_rate = atoi(t);
        continue;
      }
      type = t;
      if(g_module_infos.count(type) == 0)
        throw(UnknownModuleTypeExcept(def_copy+t));
      continue;
    }
    if(name == "") { name = t; continue; }
    string param_type;
    try
    {
      param_type = g_module_infos[type]->parameter(params.size()).type;
    }
    catch(TooManyParamsExcept)
    {
      throw(TooManyParamsExcept_D(def_copy));
    }
    ModuleParam *m = new ModuleParam();
    if(param_type=="float")
    {
      if(!isdigit(*t) && *t!='-' && *t!='.')
        throw(ExpectingFloatExcept(def_copy+t));
      m->m_float = atof(t);
    }
    else if(param_type=="int")
    {
      if(!isdigit(*t) && *t!='-') throw(ExpectingIntExcept(def_copy+t));
      m->m_int = atoi(t);
    }
    else if(param_type=="Module")
    {
      if(isdigit(*t) || *t=='-' || *t=='.')
        m->m_module = new Constant(atof(t));
      else
      {
        if(!g_modules.count(t)) throw(UnknownModuleExcept(def_copy+t));
        if(g_modules[t]->stereo())
          throw(NotMonoExcept(def_copy + t));
        m->m_module = g_modules[t];
      } 
    }
    else if(param_type=="StereoModule")
    {
      if(!isalpha(*t))
        throw(ExpectingModuleExcept(def_copy+t));
      else
      {
        if(!g_modules.count(t)) throw(UnknownModuleExcept(def_copy+t));
        if(!g_modules[t]->stereo())
          throw(NotStereoExcept(def_copy + t));
        m->m_stereomodule = (StereoModule *)g_modules[t];
      } 
    }
    else if(param_type=="string")
    {
      if(*t != '"') throw(ExpectingStringExcept(def_copy+t));
      char *end = strchr(t+1, '"');
      if(!end) throw(BadStringExcept(def_copy+t));
      *end = 0;
      m->m_string = t+1;
    }
    else throw(UnknownTypeExcept(def_copy+t));
    params.push_back(m);
  } while(t = strtok(0, delim));

  while(params.size() != g_module_infos[type]->parameterCount())
  {
    if(!g_module_infos[type]->parameter(params.size()).hasdefault) break;
    
    ModuleParam *m = new ModuleParam();
    m->m_module =
      new Constant(g_module_infos[type]->parameter(params.size()).defaultvalue);
    params.push_back(m);
  }
  
  if(params.size() != g_module_infos[type]->parameterCount())
    throw(TooFewParamsExcept(def_copy));
  if(g_modules.count(name) > 0) throw(ReusedModuleNameExcept(def_copy+name));
  g_modules[name] = g_module_infos[type]->instantiate(params);
  g_modules[name]->setName(name);
  g_modules[name]->setSampleRate(at_rate);
}

EXCEPTION  (NoOutputModuleExcept,  Exception, "No output module")
EXCEPTION  (OutputNotStereoExcept, Exception, "Output not stereo")
EXCEPTION_D(CouldntOpenFileExcept, Exception, "Couldn't open file")

static vector<FileRef> g_log_list;
static Module *g_stream_output;
float g_audio_time = 0;

Module *loadPatch(const char* patchname)
{
  g_modules.clear();
  g_profiler.clear();
  g_audio_time = 0;
  
  File in(getPatchLocation(patchname), File::READ);

  string s;
  while(!in.eof())
  {
    static char buf[256];
    s = in.readLine();
    strncpy(buf, s.c_str(), 255);
    addModule(buf);
  }
  
  in.close();

  for(vector<string>::iterator name = g_log_list.begin();
      name!=g_log_list.end(); name++)
  {
    if(!g_modules.count(*name)) throw(UnknownModuleExcept(*name));
    g_modules[*name]->createLog(*name);
  }
  
  if(!g_modules.count("output")) throw(NoOutputModuleExcept());
  Module *output = g_modules["output"];
  if(!output->stereo()) throw(OutputNotStereoExcept());
  
  return output;
}

void synthSetRange(int start_octave, int octave_range)
{
  g_octave_range = octave_range;
  g_start_octave = start_octave;
}

void synthSetPatch(const char* patch)
{
  g_patch = patch;
  g_stream_output = loadPatch(patch);  
}

void synthSetKey(int key)
{
  g_key = key;
}

void synthSetScale(const char *name)
{
  char *steps=0;
  for(int i=0; g_scales[i].name; i++)
    if(strcmp(name, g_scales[i].name) == 0)
      steps = (char *)(g_scales[i].steps);
  if(steps == 0)
    throw(ModuleExcept(string("Unknown scale: ")+name));
  for(int note=0; note<128; note++)
  {
    int closest = -128;
    int step = -1;
    for(int trying=0; trying<128; trying+=steps[step])
    {
      if(steps[++step] == 0) step = 0;
      if(abs(trying-note) < abs(closest-note)) closest = trying;
    }
    g_closest_notes[note] = closest;
  }
}

char *describeTimeSpent()
{
  return g_profiler.describeTimeSpent(g_audio_time);
}

Module *setupStream()
{
  fillModuleList();
  g_module_infos["Input"] = new ModuleInfo("Input", Input::create);
  g_module_infos["Input"]->addParameter("axis", "int");
  
  populateLogList(g_log_list);

  table_init();

  return loadPatch(g_patch);
}

void synthProduceStream(short *buffer, int samples)
{ 
  if(!g_stream_output)
    g_stream_output = setupStream();
  
  double time = hires_time();  
  const float *o = g_stream_output->output(g_audio_time, samples, g_sample_rate);
  g_audio_time += float(samples) / g_sample_rate;  
  
  for(int i=0; i<samples*2; i+=2)
  {
#ifdef __APPLE__
    *buffer++ = short(o[i]+o[i+1] * 16384);
#elif __CHUMBY__
	*buffer++ = ((long)o[i]*32767) | ((long)(o[i+1]*32767)<<16);
#else
  *buffer++ = short(o[i]   * 32767);
  *buffer++ = short(o[i+1] * 32767);
#endif
  }
}
