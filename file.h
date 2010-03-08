/** @file file.h
    @brief Provides file I/O. */
#ifndef _FILE_H
#define _FILE_H

#include <string>

#include "exception.h"

typedef signed   char  int8;
typedef unsigned char  uint8;
typedef signed   short int16;
typedef unsigned short uint16;
typedef signed   long  int32;
typedef unsigned long  uint32;
typedef float          float32;
typedef double         float64;

/* Thrown on attempt to open a file in read and write mode. */
EXCEPTION(ExceptionReadWriteUnsup, Exception,
          "Open as read and write unsupported.")

/* Thrown on attempted file access when no file is open. */
EXCEPTION(ExceptionNoFileOpen, Exception, "No file open.")

/* Thrown on attempted read when file is not in read mode. */
EXCEPTION_D(ExceptionNotInReadMode,   Exception, "File not in read mode: ")

/* Thrown on attempted write when file is not in write mode. */
EXCEPTION_D(ExceptionNotInWriteMode,  Exception, "File not in write mode: ")

/* Thrown when unable to open requested file. */
EXCEPTION_D(ExceptionCouldntOpenFile, Exception, "Couldn't open file: ")

class FileHandle;

/// @brief Provides file I/O
class File
{ 
  public:
    /// @brief Construct without opening a file.
    File() : m_file(0), m_cached_size(-1), m_mode(0) {}

    /// @brief Construct with a file open.
    /** @param filename Name of file to open.
        @param mode Mode in which to open the file. */
    File(const std::string &filename, int8 mode)
    : m_cached_size(-1)
    { 
      if(!open(filename, mode)) throw ExceptionCouldntOpenFile(filename);
    }

    /// @brief Close open file, if any.
    ~File() { close(); }
    
    /// @brief Open a file.
    /** @param filename Name of file to open.
        @param m Mode in which to open the file. */     
    bool open(const std::string &filename, int8 mode);

    /// @brief Close open file, if any.
    void close();
    
    /// @return Whether the file is in read mode.
    bool inReadMode();

    /// @return Whether the file is in write mode.
    bool inWriteMode();
    
    /// @return Whether we have reached the end of the file.
    bool eof();
    
    /// @return Size of file.
    int32 size();

    /// @brief Sets the cursor to offset from the beginning of the file.
    void seek(int32 offset);

    /// @brief Sets the cursor to offset from the current position.
    void seekForward(int32 offset);

    /// @brief Sets the cursor to offset from the end of the file.
    void File::seekEnd(int32 offset=0);

    /// @return Current cursor position.
    int32 pos();

    /// @brief Read binary data from the file.
    /** @param data Pointer to location at which to store read data.
        @param size Size of the data blocks to read, in bytes.
        @param count Number of data blocks to read.
        
        @return The number of blocks successfully read. */
    int32 read(void *data, int32 size, int32 count);

    /// @brief Write binary data to file.
    /** @param data Pointer to data to write.
        @param size Size of data blocks to write, in bytes.
        @param count Number of data blocks to write.
        
        @return The number of block successfully written. */
    int32 write(const void *data, int32 size, int32 count);

    /// @brief Write a formatted text string to file.
    /** @param fmt  printf format of text to write to file.
        @param ...  Parameters to format string. */
    void printf(char *fmt, ...);
    
    /// @brief Read a line of text from file.
    std::string readLine();

    int8    readInt8    (); ///< @brief Read a 8  bit int.
    int16   readInt16   (); ///< @brief Read a 16 bit int in LSB form.
    int16   readInt16MSB(); ///< @brief Read a 16 bit int in MSB form.
    int32   readInt32   (); ///< @brief Read a 32 bit int in LSB form.
    int32   readInt32MSB(); ///< @brief Read a 32 bit int in MSB form.
    float32 readFloat32 (); ///< @brief Read a 32 bit float in IEEE-754 form.
    float64 readFloat64 (); ///< @brief Read a 64 bit float in IEEE-754 form.

    std::string readString(); ///< Read a pascal string
    
    void writeInt8   (int8    n); ///< @brief Write a 8  bit int in LSB form.
    void writeInt16  (int16   n); ///< @brief Write a 16 bit int in LSB form.
    void writeInt32  (int32   n); ///< @brief Write a 32 bit int in LSB form.

    /// @brief Write a 32 bit float in IEEE-754 form.
    void writeFloat32(float32 n); 

    /// @brief Write a 64 bit float in IEEE-754 form.
    void writeFloat64(float64 n); 

    /// @brief Write a pascal string.
    void writeString(const std::string &s);
    
    enum { READ=1, WRITE=2 };

  private:
    FileHandle *m_file;
    int8        m_mode;
    std::string m_filename;
    int32       m_cached_size;
};

#endif
