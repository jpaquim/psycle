// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file
/// stuff related with virtual member functions

#ifndef UNIVERSALIS__COMPILER__DETAIL__PRAGMATIC__VIRTUAL__INCLUDED
#define UNIVERSALIS__COMPILER__DETAIL__PRAGMATIC__VIRTUAL__INCLUDED
#pragma once

#include "attribute.hpp"

/// pure virtual classes
#if \
	defined DIVERSALIS__COMPILER__GNU || \
	defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__PURE_VIRTUAL UNIVERSALIS__COMPILER__ATTRIBUTE(novtable)
#else
	#define UNIVERSALIS__COMPILER__PURE_VIRTUAL
#endif

/// overridding of virtual member function
#if 0 // new keyword in C++0x
	#define UNIVERSALIS__COMPILER__OVERRIDE [[override]]
#else
	#define UNIVERSALIS__COMPILER__OVERRIDE
#endif

#endif
