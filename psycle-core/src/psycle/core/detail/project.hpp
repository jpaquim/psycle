// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2010 members of the psycle project http://psycle.sourceforge.net

///\file

#ifndef PSYCLE__CORE__DETAIL__PROJECT__INCLUDED
#define PSYCLE__CORE__DETAIL__PROJECT__INCLUDED
#pragma once

#include <psycle/core/detail/config.hpp>
#include <universalis.hpp>

namespace psycle { namespace core {

using namespace universalis::stdlib;

#ifndef DIVERSALIS__COMPILER__RESOURCE
	#ifdef PSYCLE__CORE__SHARED
		#ifdef PSYCLE__CORE__SOURCE
			#define PSYCLE__CORE__DECL UNIVERSALIS__COMPILER__DYN_LINK__EXPORT
		#else
			#define PSYCLE__CORE__DECL UNIVERSALIS__COMPILER__DYN_LINK__IMPORT
		#endif
	#else
		#define PSYCLE__CORE__DECL //UNIVERSALIS__COMPILER__DYN_LINK__HIDDEN
	#endif

	#if !defined PSYCLE__CORE__SOURCE && defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "psycle-core")
	#endif
#endif

}}

#endif
