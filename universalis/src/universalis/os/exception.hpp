// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::os::exception

#ifndef UNIVERSALIS__OS__EXCEPTION__INCLUDED
#define UNIVERSALIS__OS__EXCEPTION__INCLUDED
#pragma once

#include <universalis/exception.hpp>
#include <universalis/compiler/compiler.hpp>
#include <cerrno>
#if defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace os {

/// generic exception thrown by functions of the namespace universalis::os.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK exception : public universalis::exception {
	public:
		exception(compiler::location const & location) throw()
			: universalis::exception(location), code_(
					#if !defined DIVERSALIS__OS__MICROSOFT
						errno
					#else
						::GetLastError()
					#endif
				), what_() {}
		exception(int const & code, compiler::location const & location) throw() : universalis::exception(location), code_(code), what_() {}
		~exception() throw() { delete what_; }

	public:
		int code() const throw() { return code_; }
	private:
		int const code_;
		#if defined DIVERSALIS__OS__MICROSOFT
			// indicates that code() is not a winapi error one, but a posix one.
			// This makes the what() function use the standard lib strerror() function.
			bool const code_is_posix_;
		#endif

	public:
		char const * what() const throw() /*override*/;
	protected:
		std::string const mutable * what_;
};

namespace exceptions {

	///\internal
	namespace detail {
		#if defined DIVERSALIS__OS__MICROSOFT
			/// exceptions for which code() is a standard posix errno one and not a winapi GetLastError() one.
			/// The what() function then uses the standard lib strerror() function instead of windows ntdll FormatMessage().
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK posix : public exception {
				public:
					posix(compiler::location const & location) throw() : exception(errno, location) {}
					posix(int const & code, compiler::location const & location) throw() : exception(code, location) {}
					char const * what() const throw() /*override*/;
			};
		#else
			// nothing special to do on posix systems
			typedef exception posix;
		#endif
	}

	class UNIVERSALIS__COMPILER__DYNAMIC_LINK operation_not_permitted : public detail::posix {
		public:
			operation_not_permitted(compiler::location const & location) throw() : detail::posix(EPERM, location) {}
	};

	class UNIVERSALIS__COMPILER__DYNAMIC_LINK runtime_error : public universalis::exceptions::runtime_error {
		public:
			runtime_error(std::string const & what, compiler::location const & location, void const * cause = 0) throw()
			: universalis::exceptions::runtime_error(what, location, cause)
			{}
	};
}

}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
