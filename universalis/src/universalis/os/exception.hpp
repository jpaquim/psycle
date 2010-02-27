// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::os::exception

#ifndef UNIVERSALIS__OS__EXCEPTION__INCLUDED
#define UNIVERSALIS__OS__EXCEPTION__INCLUDED
#pragma once

#include <universalis/exception.hpp>
#include <string>
#include <cerrno>
#if defined DIVERSALIS__OS__MICROSOFT
	#include "include_windows_without_crap.hpp"
#endif

namespace universalis { namespace os {

/// generic exception thrown by functions of the namespace universalis::os.
class UNIVERSALIS__DECL exception : public universalis::exception {
	public:
		typedef
			#if !defined DIVERSALIS__OS__MICROSOFT
				int
			#else
				DWORD
			#endif
			code_type;

		exception(compiler::location const & location) throw()
			: universalis::exception(location), code_(
					#if !defined DIVERSALIS__OS__MICROSOFT
						errno
					#else
						::GetLastError()
					#endif
				), what_() {}
		exception(code_type code, compiler::location const & location) throw() : universalis::exception(location), code_(code), what_() {}
		~exception() throw() { delete what_; }

	public:
		code_type code() const throw() { return code_; }
	private:
		code_type const code_;

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
			class UNIVERSALIS__DECL posix : public exception {
				public:
					posix(compiler::location const & location) throw() : exception(errno, location) {}
					posix(int code, compiler::location const & location) throw() : exception(code, location) {}
					char const * what() const throw() /*override*/;
			};
		#else
			// nothing special to do on posix systems
			typedef exception posix;
		#endif
	}

	class UNIVERSALIS__DECL operation_not_permitted : public detail::posix {
		public:
			operation_not_permitted(compiler::location const & location) throw() : detail::posix(EPERM, location) {}
	};

	class UNIVERSALIS__DECL runtime_error : public universalis::exceptions::runtime_error {
		public:
			runtime_error(std::string const & what, compiler::location const & location, void const * cause = 0) throw()
			: universalis::exceptions::runtime_error(what, location, cause)
			{}
	};

	///\internal
	namespace detail {
		std::string UNIVERSALIS__DECL desc(
			#if !defined DIVERSALIS__OS__MICROSOFT
				int const code = errno
			#else
				::DWORD /* or ::HRESULT in some cases */ const /*= ::GetLastError()*/,
				bool from_processor = false
			#endif
		);
	}

	std::string inline desc(
		#if !defined DIVERSALIS__OS__MICROSOFT
			int const code = errno
		#else
			::DWORD /* or ::HRESULT in some cases */ const code = ::GetLastError()
		#endif
	) { std::string nvr = detail::desc(code); return nvr; }

}

}}

#endif
