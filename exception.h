/** @file exception.h
    @brief Provides a base exception class and macros used to create
           new exception subclasses. */
#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string>

/// Exception base class.
/** Exception is the pure abstract base class (interface) for the engine's
    exception subclasses.
    
    @sa EXCEPTION(), EXCEPTION_D() */
class Exception
{
   public:
    /// @brief Describe the exception.
    /** @return A string that describes the exception. */
    virtual std::string describe()=0;
};

/// Macro used to define a regular exception class.
/** The @c EXCEPTION macro defines an exception class which stores a
    description of the exception.  It returns this description on calls to
    @c describe().  

    @param name The name of the exception type.
    @param from The parent exception class.
    @param desc The string of text that describes the exception.

    @warning The @c EXCEPTION macro does not instantiate an exception of this
             type, it defines the subclass for future instantiation. */
#define EXCEPTION(name, from, desc)                   \
  class name:public from                              \
  {                                                   \
    public:                                           \
      virtual std::string describe() { return desc; } \
  };

/// Macro used to define an exception class that takes additional data.
/** The @c EXCEPTION_D macro defines an exception class which stores arbitrary
    data pertaining to the exception.  This data is stored internally as a
    string, and is passed into the exception object on its construction.
    The data is then output when @c describe() is called.

    @param name The name of the exception type.
    @param from The parent exception class.
    @param desc The string of text that precedes the data representation when
                @c describe() is called for an instance of this exception
                class.
                
    @warning The @c EXCEPTION_D macro does not instantiate an exception of
             this type, it defines the subclass for future instantiation. */
#define EXCEPTION_D(name, from, desc)                 \
  class name:public from                              \
  {                                                   \
    public:                                           \
      name(std::string data) : data(data) {};         \
      virtual std::string describe()                  \
      {                                               \
        return std::string(desc) + ": " + data;       \
      }                                               \
      std::string data;                               \
  };

#endif
