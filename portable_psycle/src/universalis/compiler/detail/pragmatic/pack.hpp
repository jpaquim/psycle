// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#define UNIVERSALIS__COMPILER__PACK__PUSH(x) <pshpack#x.h>
	#define UNIVERSALIS__COMPILER__PACK__POP <poppack.h>
#endif

// arch-tag: bd51fa83-3a62-462e-8ea6-d8e52671a45b
