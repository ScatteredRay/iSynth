#include <cstdarg>
#include <cstdlib>
#include <jni.h>
#include <string>
#include <vector>
#include <cassert>
#include <android/log.h>
#include <unistd.h>

#include "file.h"

#define D(s) __android_log_write(ANDROID_LOG_DEBUG, "file_jni.cpp", s)
#define E(s) __android_log_write(ANDROID_LOG_ERROR, "file_jni.cpp", s)
using namespace std;

struct Asset {
    string fn;
    long pos;
    long len;
};

struct FileHandle {
    FILE *apkfile;
    Asset *asset;
};


static string apkpath;
static vector<Asset*> assets;


extern "C" {

JNIEXPORT void JNICALL
Java_com_iSynth_iSynth_setApkPath(JNIEnv *env, jobject obj, jstring path)
{
    const char *str = env->GetStringUTFChars(path, NULL);
    if (str == NULL)
        return;
    apkpath = str;
    env->ReleaseStringUTFChars(path, NULL);
}

JNIEXPORT void JNICALL
Java_com_iSynth_iSynth_addFile(JNIEnv *env, jobject obj, jstring fn, jlong pos, jlong len)
{
    const char *str = env->GetStringUTFChars(fn, NULL);
    if (str == NULL)
        return;
    Asset *a = new Asset();
    a->fn = str;
    a->pos = pos;
    a->len = len;
    assets.push_back(a);
    env->ReleaseStringUTFChars(fn, NULL);
}

}



bool File::open(const string &filename, int8 mode)
{
    m_mode     = mode;
    m_filename = filename;

    if (m_mode & WRITE) return false;

    Asset *asset;
    vector<Asset*>::iterator it;
    for (it=assets.begin(); it != assets.end(); ++it) {
        if ((*it)->fn == filename) {
            asset = *it;
            break;
        }
    }
    if (it == assets.end()) {
        E("asset not found");
        return false;
    }
    
    if (!m_file) {
        m_file = new FileHandle();
        m_file->apkfile = fopen(apkpath.c_str(), "rb");
        if (!m_file->apkfile) {
            E("Couldn't open APK file");
            return false;
        }
    }

    m_file->asset = asset;
    seek(0);

    //D(("opened file " + filename).c_str());

    return true;
}

void File::close()
{ 
    if(m_file && m_file->apkfile) {
        fclose(m_file->apkfile);
        delete m_file;
    }
    m_file=0, m_mode=0;
}

bool File::inReadMode () { return m_mode & READ;  }
bool File::inWriteMode() { return m_mode & WRITE; }
/* TODO seek functions need to make sure we're not straying from the file's boundaries */
void File::seek(int32 offset) { fseek(m_file->apkfile, m_file->asset->pos+offset, SEEK_SET); }
void File::seekForward(int32 offset) { fseek(m_file->apkfile, offset, SEEK_CUR); }
void File::seekEnd(int32 offset)
{
    fseek(m_file->apkfile,
          m_file->asset->pos + m_file->asset->len - offset,
          SEEK_SET);
}
int32 File::pos() { return ftell(m_file->apkfile) - m_file->asset->pos; }

bool File::eof()
{
    if(m_cached_size == -1) m_cached_size = size();  
    return pos() == m_cached_size;
}

int32 File::size()
{
    if (m_file && m_file->asset)
        return m_file->asset->len;
    return 0;
}

int32 File::read(void *data, int32 size, int32 count)
{
    assert(m_file);
    assert(m_file->apkfile);
    assert(m_mode & READ);

    //TODO prevent reading past EOF
    return (int32)(fread(data, size, count, m_file->apkfile));
}

int32 File::write(const void *data, int32 size, int32 count)
{
    return 0;
}

void File::printf(char *fmt, ...)
{ 
    assert(m_file && m_file->apkfile);
    assert(m_mode & WRITE);

    va_list args;
    va_start(args, fmt);
    vfprintf(m_file->apkfile, fmt, args);
    va_end(args);

}

string File::readLine()
{
  string s;
  
  while(!eof())
  {
    int8 c = readInt8();
    if(c == '\r' || c == '\n')
    {
      if(c == '\n') break;
      int32 position = pos();
      int8 linefeed = readInt8();
      if(linefeed != '\n') seek(position);
      break;
    }
    s += c;
  }
  
  return s;
}

int8 File::readInt8()
{
  int8 n;
  read(&n, 1, 1);
  return n;
}

int16 File::readInt16()
{
  int16 n;
  read(&n, 2, 1);
  return n;
}

int16 File::readInt16MSB()
{
  uint8 d[2];
  read(d, 1, 2);
  return d[0] + (d[1]<<8);
}

int32 File::readInt32()
{ 
  int32 n;
  read(&n, sizeof(int32), 1);
  return n;
}

int32 File::readInt32MSB()
{
  uint8 d[4];
  read(d, 1, 4);

  int32 n=d[0];
  for(int32 i=1; i<4; i++) n<<=8, n+=d[i];
  
  return n;
}

float32 File::readFloat32()
{
  float32 n;
  read(&n, sizeof(float32), 1);
  return n;
}

float64 File::readFloat64()
{
  float64 n;
  read(&n, sizeof(float64), 1);
  return n;
}

string File::readString()
{
  int16 i = readInt16();
  char *temp_string = new char[i+1];
  temp_string[i] = '\0';
  read(temp_string, sizeof(int8), i);
  string s = string(temp_string);
  delete[] temp_string;
  return s;
}

void File::writeString(const std::string &s)
{
  writeInt16(int16(s.length()));
  write(s.c_str(), sizeof(int8), int32(s.length()));
}


void File::writeInt8   (int8    n) { write(&n, sizeof(int8   ), 1); }
void File::writeInt16  (int16   n) { write(&n, sizeof(int16  ), 1); }
void File::writeInt32  (int32   n) { write(&n, sizeof(int32  ), 1); }
void File::writeFloat32(float32 n) { write(&n, sizeof(float32), 1); }
void File::writeFloat64(float64 n) { write(&n, sizeof(float64), 1); }
