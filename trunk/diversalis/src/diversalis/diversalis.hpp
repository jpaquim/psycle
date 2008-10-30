// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief meta header that includes all of diversalis' headers.
#ifndef DIVERSALIS__INCLUDED
#define DIVERSALIS__INCLUDED
	#pragma once
	#include "compiler.hpp"
	#if !defined DIVERSALIS__COMPILER__RESOURCE
		#include "operating_system.hpp"
		#include "processor.hpp"
		#include "standard_library.hpp"
	#endif
#endif
