// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief handling of shared dynamic-link libraries for microsoft's dll format
#if defined LIBRARY__DEFINED
	#error "missing #include LIBRARY__END, aka #include <operating_system/library/end.hpp>"
#endif
#if defined OPERATING_SYSTEM__MICROSOFT && !defined COMPILER__GNU // gcc handles dll transparently :-)
	// The microsoft dll horror show system begins...
	#if LIBRARY
		#include LIBRARY__EXPORT
	#else
		#include LIBRARY__IMPORT
	#endif
	#define class class LIBRARY
#else
	#undef LIBRARY
	/// handling of shared dynamic-link libraries for microsoft's dll format.
	/// either attribute(dllexport) or attribute(dllimport).
	/// The microsoft dll horror show system does not apply to other systems, where everything is nice and simple.
	/// Also, mingw handles dll transparently :-)
	#define LIBRARY
	#if defined COMPILER__DOXYGEN
		/// for classes, transparent handling of shared dynamic-link libraries for microsoft's dll format
		///\see LIBRARY
		#define class class LIBRARY
	#endif
#endif
#define LIBRARY__DEFINED

// arch-tag: 8423c69b-6223-4ac7-95f6-6d7fafd8d9a4
