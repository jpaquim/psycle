// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief
#pragma once
#include <universalis/detail/project.hpp>
#include <string>
#include <cerrno>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
namespace universalis
{
	namespace operating_system
	{
		namespace exceptions
		{
			std::string code_description(int const & code = errno) throw();

			#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				std::string microsoft_code_description(::DWORD /* or ::HRESULT in some cases */ const & code = ::GetLastError()) throw();
			#endif
		}
	}
}
// arch-tag: 3bafc500-fe01-4cde-9a14-8002180e84f1
