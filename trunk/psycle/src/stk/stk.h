/***************************************************/
/*! \class Stk
    \brief STK base class

    Nearly all STK classes inherit from this class.
    The global sample rate and rawwave path variables
    can be queried and modified via Stk.  In addition,
    this class provides error handling and
    byte-swapping functions.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    Any person wishing to distribute modifications to the Software is
    asked to send the modifications to the original developer so that
    they can be incorporated into the canonical version.  This is,
    however, not a binding provision of this license.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/***************************************************/

#ifndef STK_STK_H
#define STK_STK_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

// Most data in STK is passed and calculated with the
// following user-definable floating-point type.  You
// can change this to "float" if you prefer or perhaps
// a "long double" in the future.
typedef double StkFloat;

//! STK error handling class.
/*!
  This is a fairly abstract exception handling class.  There could
  be sub-classes to take care of more specific error conditions ... or
  not.
*/
class StkError
{
public:
  enum Type {
    STATUS,
    WARNING,
    DEBUG_WARNING,
    MEMORY_ALLOCATION,
    MEMORY_ACCESS,
    FUNCTION_ARGUMENT,
    FILE_NOT_FOUND,
    FILE_UNKNOWN_FORMAT,
    FILE_ERROR,
    PROCESS_THREAD,
    PROCESS_SOCKET,
    PROCESS_SOCKET_IPADDR,
    AUDIO_SYSTEM,
    MIDI_SYSTEM,
    UNSPECIFIED
  };

protected:
  std::string message_;
  Type type_;

public:
  //! The constructor.
  StkError(const std::string& message, Type type = StkError::UNSPECIFIED)
    : message_(message), type_(type) {}

  //! The destructor.
  virtual ~StkError(void) {};

  //! Prints thrown error message to stderr.
  virtual void printMessage(void) { std::cerr << '\n' << message_ << "\n\n"; }

  //! Returns the thrown error message type.
  virtual const Type& getType(void) { return type_; }

  //! Returns the thrown error message string.
  virtual const std::string& getMessage(void) { return message_; }

  //! Returns the thrown error message as a C string.
  virtual const char *getMessageCString(void) { return message_.c_str(); }
};


class Stk
{
public:

  typedef unsigned long StkFormat;
  static const StkFormat STK_SINT8;   /*!< -128 to +127 */
  static const StkFormat STK_SINT16;  /*!< -32768 to +32767 */
  static const StkFormat STK_SINT24;  /*!< Upper 3 bytes of 32-bit signed integer. */
  static const StkFormat STK_SINT32;  /*!< -2147483648 to +2147483647. */
  static const StkFormat STK_FLOAT32; /*!< Normalized between plus/minus 1.0. */
  static const StkFormat STK_FLOAT64; /*!< Normalized between plus/minus 1.0. */

  //! Static method which returns the current STK sample rate.
  static StkFloat sampleRate( void ) { return srate_; }

  //! Static method that sets the STK sample rate.
  /*!
    The sample rate set using this method is queried by all STK
    classes which depend on its value.  It is initialized to the
    default SRATE set in Stk.h.  Many STK classes use the sample rate
    during instantiation.  Therefore, if you wish to use a rate which
    is different from the default rate, it is imperative that it be
    set \e BEFORE STK objects are instantiated.  A few classes that
    make use of the global STK sample rate are automatically notified
    when the rate changes so that internal class data can be
    appropriately updated.  However, this has not been fully
    implemented.  Specifically, classes that appropriately update
    their own data when either a setFrequency() or noteOn() function
    is called do not currently receive the automatic notification of
    rate change.  If the user wants a specific class instance to
    ignore such notifications, perhaps in a multi-rate context, the
    function Stk::ignoreSampleRateChange() should be called.
  */
  static void setSampleRate( StkFloat rate );

  //! A function to enable/disable the automatic updating of class data when the STK sample rate changes.
  /*!
    This function allows the user to enable or disable class data
    updates in response to global sample rate changes on a class by
    class basis.
  */
  void ignoreSampleRateChange( bool ignore = true ) { ignoreSampleRateChange_ = ignore; };

  //! Static method which returns the current rawwave path.
  static std::string rawwavePath(void) { return rawwavepath_; }

  //! Static method which sets the STK rawwave path.
  static void setRawwavePath( std::string path );

  //! Static method which byte-swaps a 16-bit data type.
  static void swap16( unsigned char *ptr );

  //! Static method which byte-swaps a 32-bit data type.
  static void swap32( unsigned char *ptr );

  //! Static method which byte-swaps a 64-bit data type.
  static void swap64( unsigned char *ptr );

  //! Static cross-platform method to sleep for a number of milliseconds.
  static void sleep( unsigned long milliseconds );

  //! Static function for error reporting and handling using c-strings.
  static void handleError( const char *message, StkError::Type type );

  //! Static function for error reporting and handling using c++ strings.
  static void handleError( std::string message, StkError::Type type );

  //! Toggle display of WARNING and STATUS messages.
  static void showWarnings( bool status ) { showWarnings_ = status; }

  //! Toggle display of error messages before throwing exceptions.
  static void printErrors( bool status ) { printErrors_ = status; }

private:
  static StkFloat srate_;
  static std::string rawwavepath_;
  static bool showWarnings_;
  static bool printErrors_;
  static std::vector<Stk *> alertList_;

protected:

  std::ostringstream errorString_;
  bool ignoreSampleRateChange_;

  //! Default constructor.
  Stk( void );

  //! Class destructor.
  virtual ~Stk( void );

  //! This function should be implemented in subclasses that depend on the sample rate.
  virtual void sampleRateChanged( StkFloat newRate, StkFloat oldRate );

  //! Add class pointer to list for sample rate change notification.
  void addSampleRateAlert( Stk *ptr );

  //! Remove class pointer from list for sample rate change notification.
  void removeSampleRateAlert( Stk *ptr );

  //! Internal function for error reporting which assumes message in \c errorString_ variable.
  void handleError( StkError::Type type );
};


/***************************************************/
/*! \class StkFrames
    \brief An STK class to handle vectorized audio data.

    This class can hold single- or multi-channel audio data in either
    interleaved or non-interleaved formats.  The data type is always
    StkFloat.  In an effort to maintain efficiency, no out-of-bounds
    checks are performed in this class.

    Possible future improvements in this class could include functions
    to inter- or de-interleave the data and to convert to and return
    other data types.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.
*/
/***************************************************/

class StkFrames
{
public:

  //! The default constructor initializes the frame data structure to size zero.
  StkFrames( unsigned int nFrames = 0, unsigned int nChannels = 0, bool interleaved = true );

  //! Overloaded constructor that initializes the frame data to the specified size with \c value.
  StkFrames( const StkFloat& value, unsigned int nFrames, unsigned int nChannels, bool interleaved = true );

  //! The destructor.
  ~StkFrames();

  //! Subscript operator which returns a reference to element \c n of self.
  /*!
    The result can be used as an lvalue . This reference is valid
    until the resize function is called or the array is destroyed. The
    index \c n must be between 0 and size less one.  No range checking
    is performed unless _STK_DEBUG_ is defined.
  */
  StkFloat& operator[] ( size_t n );

  //! Subscript operator that returns the value at element \c n of self.
  /*!
    The index \c n must be between 0 and size less one.  No range
    checking is performed unless _STK_DEBUG_ is defined.
  */
  StkFloat operator[] ( size_t n ) const;

  //! Channel / frame subscript operator that returns a reference.
  /*!
    The result can be used as an lvalue. This reference is valid
    until the resize function is called or the array is destroyed. The
    \c frame index must be between 0 and frames() - 1.  The \c channel
    index must be between 0 and channels() - 1.  No range checking is
    performed unless _STK_DEBUG_ is defined.
  */
  StkFloat& operator() ( size_t frame, unsigned int channel );

  //! Channel / frame subscript operator that returns a value.
  /*!
    The \c frame index must be between 0 and frames() - 1.  The \c
    channel index must be between 0 and channels() - 1.  No range checking
    is performed unless _STK_DEBUG_ is defined.
  */
  StkFloat operator() ( size_t frame, unsigned int channel ) const;

  //! Return an interpolated value at the fractional frame index and channel.
  /*!
    This function performs linear interpolation.  The \c frame
    index must be between 0.0 and frames() - 1.  The \c channel index
    must be between 0 and channels() - 1.  No range checking is
    performed unless _STK_DEBUG_ is defined.
  */
  StkFloat interpolate( StkFloat frame, unsigned int channel = 0 ) const;

  //! Returns the total number of audio samples represented by the object.
  size_t size() const { return size_; }; 

  //! Returns \e true if the object size is zero and \e false otherwise.
  bool empty() const;

  //! Resize self to represent the specified number of channels and frames.
  /*!
    Changes the size of self based on the number of frames and
    channels.  No element assignment is performed.  No memory
    deallocation occurs if the new size is smaller than the previous
    size.  Further, no new memory is allocated when the new size is
    smaller or equal to a previously allocated size.
  */
  void resize( size_t nFrames, unsigned int nChannels = 1 );

  //! Resize self to represent the specified number of channels and frames and perform element initialization.
  /*!
    Changes the size of self based on the number of frames and
    channels, and assigns \c value to every element.  No memory
    deallocation occurs if the new size is smaller than the previous
    size.  Further, no new memory is allocated when the new size is
    smaller or equal to a previously allocated size.
  */
  void resize( size_t nFrames, unsigned int nChannels, StkFloat value );

  //! Return the number of channels represented by the data.
  unsigned int channels( void ) const { return nChannels_; };

  //! Return the number of sample frames represented by the data.
  unsigned int frames( void ) const { return nFrames_; };

  //! Set the sample rate associated with the StkFrames data.
  /*!
    By default, this value is set equal to the current STK sample
    rate at the time of instantiation.
   */
  void setDataRate( StkFloat rate ) { dataRate_ = rate; };

  //! Return the sample rate associated with the StkFrames data.
  /*!
    By default, this value is set equal to the current STK sample
    rate at the time of instantiation.
   */
  StkFloat dataRate( void ) const { return dataRate_; };

  //! Returns \c true if the data is in interleaved format, \c false if the data is non-interleaved.
  bool interleaved( void ) const { return interleaved_; };

  //! Set the flag to indicate whether the internal data is in interleaved (\c true) or non-interleaved (\c false) format.
  /*!
    Note that this function does not modify the internal data order
    with respect to the argument value.  It simply changes the
    indicator flag value.
   */
  void setInterleaved( bool isInterleaved ) { interleaved_ = isInterleaved; };

private:

  StkFloat *data_;
  StkFloat dataRate_;
  size_t nFrames_;
  unsigned int nChannels_;
  size_t size_;
  size_t bufferSize_;
  bool interleaved_;

};


// Here are a few other useful typedefs.
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef signed short SINT16;
typedef signed int SINT32;
typedef float FLOAT32;
typedef double FLOAT64;

// The default sampling rate.
const StkFloat SRATE = 44100.0;

// The default real-time audio input and output buffer size.  If
// clicks are occuring in the input and/or output sound stream, a
// larger buffer size may help.  Larger buffer sizes, however, produce
// more latency.
const unsigned int RT_BUFFER_SIZE = 512;

// The default rawwave path value is set with the preprocessor
// definition RAWWAVE_PATH.  This can be specified as an argument to
// the configure script, in an integrated development environment, or
// below.  The global STK rawwave path variable can be dynamically set
// with the Stk::setRawwavePath() function.  This value is
// concatenated to the beginning of all references to rawwave files in
// the various STK core classes (ex. Clarinet.cpp).  If you wish to
// move the rawwaves directory to a different location in your file
// system, you will need to set this path definition appropriately.
#if !defined(RAWWAVE_PATH)
  #define RAWWAVE_PATH "../../rawwaves/"
#endif

const StkFloat PI           = 3.14159265358979;
const StkFloat TWO_PI       = 2 * PI;
const StkFloat ONE_OVER_128 = 0.0078125;

#if defined(__WINDOWS_DS__) || defined(__WINDOWS_ASIO__) || defined(__WINDOWS_MM__)
  #define __OS_WINDOWS__
  #define __STK_REALTIME__
#elif defined(__LINUX_OSS__) || defined(__LINUX_ALSA__) || defined(__UNIX_JACK__)
  #define __OS_LINUX__
  #define __STK_REALTIME__
#elif defined(__IRIX_AL__)
  #define __OS_IRIX__
#elif defined(__MACOSX_CORE__) || defined(__UNIX_JACK__)
  #define __OS_MACOSX__
  #define __STK_REALTIME__
#endif

//#define _STK_DEBUG_

#endif
