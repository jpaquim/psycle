// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief handling of shared dynamic-link libraries for microsoft's dll format
#include <universalis/compiler/compiler.hpp>
#if defined UNIVERSALIS__COMPILER__DYNAMIC_LINK__DETAIL__DEFINED
	#error missing #include <universalis/compiler/dynamic_link/end.hpp>
#elif !defined UNIVERSALIS__COMPILER__DYNAMIC_LINK
	#error missing #define UNIVERSALIS__COMPILER__DYNAMIC_LINK
#endif
#if defined DIVERSALIS__COMPILER__DOXYGEN
	#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
	/// handling of shared dynamic-link libraries in microsoft's dll format.
	/// either UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT or UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT.
	/// Note that the gcc/mingw compiler handles dll transparently thanks to automatic import and runtime pseudo-relocations :-)
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK
#else
	#if UNIVERSALIS__COMPILER__DYNAMIC_LINK > 0 || UNIVERSALIS__COMPILER__CONCATENATED(PACKAGENERIC__MODULE__SOURCE__, UNIVERSALIS__COMPILER__DYNAMIC_LINK) > 0
		#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT
	#elif UNIVERSALIS__COMPILER__DYNAMIC_LINK < 0 || UNIVERSALIS__COMPILER__CONCATENATED(PACKAGENERIC__MODULE__SOURCE__, UNIVERSALIS__COMPILER__DYNAMIC_LINK) < 0
		#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK /* static link */
	#else
		#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT
	#endif
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__DETAIL__DEFINED

#if 0 // alternative
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK(x) a(b, x)
	#define a(b, x) b##x
	#define b0 <universalis/compiler/dynamic_link/begin/import.hpp>
	#define b1 <universalis/compiler/dynamic_link/begin/export.hpp>
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__END <universalis/compiler/dynamic_link/end.hpp>
	// usage:
	#include UNIVERSALIS__COMPILER__DYNAMIC_LINK(FOO__BAR)
		// some declarations in between
	#include UNIVERSALIS__COMPILER__DYNAMIC_LINK__END
#endif
