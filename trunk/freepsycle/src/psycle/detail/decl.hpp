// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#include "project.hpp"

#ifndef DIVERSALIS__COMPILER__RESOURCE
	#if defined PSYCLE__DECL__DEFINED
		#undef PSYCLE__DECL__DEFINED
		#undef PSYCLE__DECL
	#else
		#define PSYCLE__DECL__DEFINED
		#if PSYCLE__DETAIL__SHARED(PSYCLE__DECL) > 0
			#if PSYCLE__DETAIL__SOURCE(PSYCLE__DECL) > 0
				#undef  PSYCLE__DECL
				#define PSYCLE__DECL UNIVERSALIS__COMPILER__DYN_LINK__EXPORT
			#else
				#undef  PSYCLE__DECL
				#define PSYCLE__DECL UNIVERSALIS__COMPILER__DYN_LINK__IMPORT
			#endif
		#else
			#undef  PSYCLE__DECL
			#define PSYCLE__DECL //UNIVERSALIS__COMPILER__DYN_LINK__HIDDEN
		#endif
	#endif
#endif
