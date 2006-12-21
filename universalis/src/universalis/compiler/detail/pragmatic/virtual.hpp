// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
/// stuff related with virtual member functions
#pragma once
#include "attribute.hpp"

/// pure virtual classes
#if \
	defined DIVERSALIS__COMPILER__GNU || \
	defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__VIRTUAL__PURE UNIVERSALIS__COMPILER__ATTRIBUTE(novtable)
#else
	#define UNIVERSALIS__COMPILER__VIRTUAL__PURE
#endif

/// implementation of pure virtual member function
#define UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE /* implements pure */ virtual
//#define UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE
#define implements_pure_virtual UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE

/// overridding of virtual member function
#define UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES /* overrides */ virtual
//#define UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES
#define overrides_virtual UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES
