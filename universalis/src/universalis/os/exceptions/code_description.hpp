// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface universalis::os::code_description

#ifndef UNIVERSALIS__OS__EXCEPTIONS__CODE_DESCRIPTION__INCLUDED
#define UNIVERSALIS__OS__EXCEPTIONS__CODE_DESCRIPTION__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <string>
#if !defined DIVERSALIS__OS__MICROSOFT
	#include <cerrno>
#else
	#include <windows.h>
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace os { namespace exceptions {

///\internal
namespace detail {
	std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK code_description(
		#if !defined DIVERSALIS__OS__MICROSOFT
			int const code = errno
		#else
			::DWORD /* or ::HRESULT in some cases */ const /*= ::GetLastError()*/,
			bool from_processor = false
		#endif
	);
}

std::string inline code_description(
	#if !defined DIVERSALIS__OS__MICROSOFT
		int const code = errno
	#else
		::DWORD /* or ::HRESULT in some cases */ const code = ::GetLastError()
	#endif
) { return detail::code_description(code); }

}}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
