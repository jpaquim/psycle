// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief
#pragma once
#include <universalis/detail/project.hpp>
#include <string>
#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <cerrno>
#else
	#include <windows.h>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__EXCEPTIONS__CODE_DESCRIPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace exceptions
		{
			///\internal
			namespace detail
			{
				std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK code_description
					(
						#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
							int const & = errno
						#else
							::DWORD /* or ::HRESULT in some cases */ const & /*= ::GetLastError()*/,
							bool from_processor = false
						#endif
					) throw();
			}

			std::string inline code_description
			(
				#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					int const & code = errno
				#else
					::DWORD /* or ::HRESULT in some cases */ const & code = ::GetLastError()
				#endif
			) throw()
			{
				return detail::code_description(code);
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
