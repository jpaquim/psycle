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
			exception(std::string const &) throw();
			/// returns the message describing the cause of the exception.
			/// same as what() but returns a std::string.
			virtual operator std::string const () const throw();
	};

	namespace exceptions
	{
		/// external cpu/os exception translated into a c++ one, with deferred querying of the human-readable message.
		class LIBRARY translated : public std::exception
		{
			public:
				/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
				void static new_thread(std::string const & = "");

			public:
				inline translated(unsigned int const & code) throw() : code_(code), what_(0) {}

			public:
				unsigned int const inline & code() const throw() { return code_; }
			private:
				unsigned int const          code_;

			public:
				       char const virtual /* overrides */ * what() const;
			private:
				std::string const mutable                 * what_;
		};
	}
}
