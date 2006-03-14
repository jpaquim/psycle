// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief handling of shared dynamic-link libraries for microsoft's dll format
#undef LIBRARY
#if defined OPERATING_SYSTEM__MICROSOFT && !defined COMPILER__GNU // gcc handles dll transparently :-)
	// the microsoft dll horror show system begins...
#	define LIBRARY EXPORT
#else
	// everything is nice and simple
#	define LIBRARY
#endif

// arch-tag: 522574a5-ca77-4aab-a125-eb13f2406630
