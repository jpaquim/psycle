#pragma once
#include <exception>
#include <string>
#if defined OPERATING_SYSTEM__EXCEPTION
#	include LIBRARY__EXPORT
#else
#	include LIBRARY__IMPORT
#endif
///\file
///\brief interface file for operating_system::exception

/// provides a unified interface for common functionalities of several operating systems, abstracting the specificities of each.
///\see namespace exceptions
///\see namespace library
namespace operating_system
{
	/// generic exception thrown by functions of the namespace operating_system.
	class LIBRARY exception : public std::runtime_error
	{
	public:
		exception(const std::string & what) : std::runtime_error(what) {}
		/// returns the message describing the cause of the exception.
		/// same as what() but returns a std::string.
		virtual operator const std::string() const throw();
	};

	/// some subclasses of operating_system::exception.
	namespace exceptions
	{
		/// external cpu/os exception translated into a c++ one.
		class LIBRARY translated : public exception
		{
		public:
			translated(const unsigned int & code);
			/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
			/// <bohan> This is needed for microsoft, i don't know about linux/gcc.
			static void new_thread(const std::string & = "");
		};
	}
}

#include <sstream>
inline std::ostringstream & operator<<(std::ostringstream & out, const operating_system::exception & e) { out << e.what(); return out; }
