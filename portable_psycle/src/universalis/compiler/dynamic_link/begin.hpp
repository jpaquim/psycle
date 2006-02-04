// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

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
	/// handling of shared dynamic-link libraries for microsoft's dll format.
	/// either UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT or UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT.
	/// The microsoft dll horror show system does not apply to other systems, where everything is nice and simple.
	/// Also, the gcc/mingw compiler handles dll transparently thanks to runtime pseudo-relocations :-)
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK
#else
	#if UNIVERSALIS__COMPILER__CONCATENATED(PACKAGENERIC__MODULE__SOURCE__, UNIVERSALIS__COMPILER__DYNAMIC_LINK)
		#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT
	#else
		#undef  UNIVERSALIS__COMPILER__DYNAMIC_LINK
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT
	#endif
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__DETAIL__DEFINED
