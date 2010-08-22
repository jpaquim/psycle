// -*- mode:c++; indent-tabs-mode:t -*-

#define VERSION "0.2.4"

#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_IO_H 1
#define HAVE_WINDOWS_H 1

#define DSOUND_SUPPORT 0
#define WINMIDI_SUPPORT 0
#define WITH_FLOAT 1

#define snprintf _snprintf
#if !defined _MSC_VER || _MSC_VER < 1500 // [bohan] build fix for msvc 9 (2008)
#define vsnprintf _vsnprintf
#endif
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#ifndef WIN32
#define WIN32  1
#endif

#define WITH_PROFILING 0

#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#pragma warning(disable : 4305)

#define WITHOUT_SERVER 1
