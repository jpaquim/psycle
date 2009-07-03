// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief microsoft's MAX_PATH has too low value for ntfs

#ifndef UNIVERSALIS__OS__DETAIL__MICROSOFT__MAX_PATH__INCLUDED
#define UNIVERSALIS__OS__DETAIL__MICROSOFT__MAX_PATH__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>

#if defined DIVERSALIS__OS__MICROSOFT
	//#include <windows.h>
	/// microsoft's MAX_PATH has too low value for ntfs
	///\see <cstdio>'s FILENAME_MAX
	#define UNIVERSALIS__OS__MICROSOFT__MAX_PATH (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
#endif

#endif
