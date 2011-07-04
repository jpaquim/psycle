// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file

#ifndef UNIVERSALIS__DETAIL__PROJECT__INCLUDED
#define UNIVERSALIS__DETAIL__PROJECT__INCLUDED
#pragma once

#include <universalis/detail/config.hpp>
#include <diversalis.hpp>

#ifndef DIVERSALIS__COMPILER__RESOURCE
	#include <universalis/compiler/dyn_link.hpp>

	#ifdef UNIVERSALIS__SHARED
		#ifdef UNIVERSALIS__SOURCE
			#define UNIVERSALIS__DECL UNIVERSALIS__COMPILER__DYN_LINK__EXPORT
		#else
			#define UNIVERSALIS__DECL UNIVERSALIS__COMPILER__DYN_LINK__IMPORT
		#endif
	#else
		#define UNIVERSALIS__DECL //UNIVERSALIS__COMPILER__DYN_LINK__HIDDEN
	#endif

	#if !defined UNIVERSALIS__SOURCE && defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "universalis")
	#endif
#endif

#endif
