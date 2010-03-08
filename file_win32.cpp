#include <cstdarg>
#include <cstdlib>

#include "file.h"

#pragma warning(disable:4267) // size_t conversion to int
#pragma warning(disable:4800) // bool "performance warning"
#pragma warning(disable:4996) // fopen() purportedly unsafe

using namespace std;

class FileHandle:public FILE {};

bool File::open(const string &filename, int8 mode)
{
  m_mode     = mode;
  m_filename = filename;

  if((m_mode & READ) && (m_mode & WRITE)) throw ExceptionReadWriteUnsup();

  char *m;
  if(m_mode & READ ) m="rb";
  if(m_mode & WRITE) m="wb";
  m_file = (FileHandle *)fopen(m_filename.c_str(), m);
  
  return m_file ? true : false;
}

void File::close()
{ 
  if(m_file) fclose(m_file);
  m_file=0, m_mode=0;
}

bool File::inReadMode () { return m_mode & READ;  }
bool File::inWriteMode() { return m_mode & WRITE; }
void File::seek(int32 offset) { fseek(m_file, offset, SEEK_SET); }
void File::seekForward(int32 offset) { fseek(m_file, offset, SEEK_CUR); }
void File::seekEnd(int32 offset) { fseek(m_file, offset, SEEK_END); }
int32 File::pos() { return ftell(m_file); }

bool File::eof()
{
  if(m_cached_size == -1) m_cached_size = size();  
  return pos() == m_cached_size;
}

int32 File::size()
{
  if(!m_file) throw ExceptionNoFileOpen();

  int32 size = 0, position = ftell(m_file);
   
  fseek(m_file, 0, SEEK_END);
  size=ftell(m_file);
  
  fseek(m_file, position, SEEK_SET);
  
  return size;
}

int32 File::read(void *data, int32 size, int32 count)
{
  if(!m_file) throw ExceptionNoFileOpen();
  if(!(m_mode & READ)) throw ExceptionNotInReadMode(m_filename);

  return (int32)(fread(data, size, count, m_file));
}

int32 File::write(const void *data, int32 size, int32 count)
{
  if(!m_file) throw ExceptionNoFileOpen();
  if(!(m_mode & WRITE)) throw ExceptionNotInWriteMode(m_filename);

  return fwrite(data, size, count, m_file);
}

void File::printf(char *fmt, ...)
{
  if(!m_file)           throw ExceptionNoFileOpen();
  if(!(m_mode & WRITE)) throw ExceptionNotInWriteMode(m_filename);

  va_list args;
  va_start(args, fmt);
  vfprintf(m_file, fmt, args);
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
