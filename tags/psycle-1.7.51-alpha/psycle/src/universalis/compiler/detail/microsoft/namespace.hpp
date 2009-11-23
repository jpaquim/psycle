// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
/// microsoft's mfc resource editor
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined _AFXDLL // mfc
	#if defined DIVERSALIS__COMPILER__RESOURCE //&& defined DIVERSALIS__COMPILER__MICROSOFT
		// msvc7.1's resource compiler freaks out: warning RC4011: identifier truncated to 'APPSTUDIO_INVOKED__MSVC_MFC_RES'
	#else
		#if defined APPSTUDIO_INVOKED__MSVC_MFC_RESOURCE_EDITOR_IS_A_SHIT_AND_DOES_NOT_HANDLE_NAMESPACES_AND_WRONGLY_PARSES_PREPROCESSOR_DIRECTIVES
			#define UNIVERSALIS__NAMESPACE__BEGIN(x)
			#define UNIVERSALIS__NAMESPACE__END
		#else
			#define UNIVERSALIS__NAMESPACE__BEGIN(x) namespace x {
			#define UNIVERSALIS__NAMESPACE__END }
		#endif
	#endif
#endif