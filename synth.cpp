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
   - decimation
   - interpolated sample playback
   - switch
   - exponential envgen, DADSR, parameterized shape
   - wave terrain
   - use yacc to parse modules?
   - support for arbitrary parameter count
     + phase moduration?
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
*/
#include "synth.h"

#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "exception.h"
#include "file.h"
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
  { "pentminor",  "\3\2\2\3\2" },
  { "chromatic",  "\1" },
  { "whole",      "\2" },
  { "minor3rd",   "\3" },
  { "3rd",        "\4" },
  { "4th",        "\5" },
  { "tritone",    "\6" },
  { "5th",        "\7" },
  { "octave",     "\12" },
  { 0,            0 }
};

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
    WaveIn(const string filename)
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
    WaveOut(const string filename, float scaler=32768, bool stereo=false)
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

EXCEPTION_D(InvalidInputRangeExcept,  Exception, "Invalid Input Range")
EXCEPTION_D(InvalidOutputRangeExcept, Exception, "Invalid Output Range")

class Module
{
  public:
    Module() : m_last_fill(0), m_waveout(0)
    {
      memset(m_output, 0, max_buffer_size*sizeof(float));
      //memset(&m_within_lower_bound, 0xff, sizeof(float));
      //memset(&m_within_upper_bound, 0xff, sizeof(float));
    }
    ~Module()
    {
      if(m_waveout) delete m_waveout;
    }

    virtual void fill(float last_fill, int samples) = 0;
    virtual void getOutputRange(float *out_min, float *out_max) = 0;
    virtual const char *moduleName() = 0;
    virtual void validateInputRange() = 0;
    virtual bool stereo() const { return false; }

    virtual const float *output(float last_fill, int samples)
    {
      if(m_last_fill < last_fill)
      {
        validateInputRange();
        fill(last_fill, samples);
        m_last_fill = last_fill;
        if(m_waveout) m_waveout->writeBuffer(m_output, samples);
        #ifndef NDEBUG
        validateOutputRange(m_output, samples);
        #endif
      }

      return m_output;
    }

    virtual void createLog(const string filename)
    {
      float min, max;
      getOutputRange(&min, &max);

      ostringstream fn;
      fn << filename << " (" << min << " to " << max << ").wav";

      if(-min > max) max = -min;
      if(!m_waveout) m_waveout = new WaveOut(fn.str(), 32767/max);
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
        if(buffer[i] < min || m_output[i] > max)
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
    float m_last_fill;
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

    const float *output(float last_fill, int samples)
    {
      if(m_last_fill < last_fill)
      {
        fill(last_fill, samples);
        m_last_fill = last_fill;
        if(m_waveout) m_waveout->writeBuffer(m_output, samples*2);
        validateOutputRange(m_output, samples*2);
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
      if(!m_waveout) m_waveout = new WaveOut(fn.str(), 32767/max, true);
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
      readInputAxis(m_axis, m_output, samples);
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

EXCEPTION(ParseExcept, Exception, "Parse Error")
EXCEPTION(TooManyParamsExcept, ParseExcept, "Too many parameters")

class ModuleInfo
{
  public:
    ModuleInfo(string name, Module *(*instantiator)(vector<ModuleParam *>))
    : m_name(name), m_instantiator(instantiator) {}

    void addParameter(string name, string type)
    {
      m_parameters.push_back(pair<string, string>(name, type));
    }
    
    const pair<string, string> parameter(unsigned int n) const
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
    vector<pair<string, string> > m_parameters; // name, type
    Module *(*m_instantiator)(vector<ModuleParam *> parameters);
};

map<string, ModuleInfo *> g_module_infos;

EXCEPTION_D(ModuleExcept, Exception, "Module exception")

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
  vector<ModuleParam *> params;
  do
  {
    if(type == "")
    {
      type = t;
      if(g_module_infos.count(type) == 0)
        throw(UnknownModuleTypeExcept(def_copy+t));
      continue;
    }
    if(name == "") { name = t; continue; }
    string param_type;
    try
    {
      param_type = g_module_infos[type]->parameter(params.size()).second;
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
  if(params.size() != g_module_infos[type]->parameterCount())
    throw(TooFewParamsExcept(def_copy));
  if(g_modules.count(name) > 0) throw(ReusedModuleNameExcept(def_copy+name));
  g_modules[name] = g_module_infos[type]->instantiate(params);
}

EXCEPTION  (NoOutputModuleExcept,  Exception, "No output module")
EXCEPTION  (OutputNotStereoExcept, Exception, "Output not stereo")
EXCEPTION_D(CouldntOpenFileExcept, Exception, "Couldn't open file")

Module *setupStream()
{
  fillModuleList();
  g_module_infos["Input"] = new ModuleInfo("Input", Input::create);
  g_module_infos["Input"]->addParameter("axis", "int");

  char *patch_filename = "patches/pad.pat";
  for(int i=1; i<argCount(); i++)
    if(!strchr(getArg(i), ':')) patch_filename = getArg(i);

  File in(patch_filename, File::READ);

  string s;
  while(!in.eof())
  {
    static char buf[256];
    s = in.readLine();
    strncpy(buf, s.c_str(), 255);
    addModule(buf);
  }
  
  in.close();

  for(int i=1; i<argCount(); i++)
    if(memcmp(getArg(i), "log:", 4) == 0)
    {      
      char loglist[256];
      strncpy(loglist, getArg(i)+4, 255);
      char *name = strtok(loglist, ",");
      do
      {
        if(!g_modules.count(name)) throw(UnknownModuleExcept(name));
        g_modules[name]->createLog(name);
      }        
      while(name = strtok(0, ","));
    }

  if(!g_modules.count("output")) throw(NoOutputModuleExcept());
  Module *output = g_modules["output"];
  if(!output->stereo()) throw(OutputNotStereoExcept());
  return output;
}

void produceStream(short *buffer, int samples)
{ 
  static bool first = true;
  static Module *output;
  if(first)
  {
    first = false;
    output = setupStream();
  }
  
  static float time = 0;
  const float *o = output->output(time, samples);
  time += 1.0;
  
  for(int i=0; i<samples*2; i+=2)
  {
#ifdef __APPLE__
    *buffer++ = short(o[i]+o[i+1] * 16384);
#else
    *buffer++ = short(o[i]   * 32767);
    *buffer++ = short(o[i+1] * 32767);
#endif
  }
}
