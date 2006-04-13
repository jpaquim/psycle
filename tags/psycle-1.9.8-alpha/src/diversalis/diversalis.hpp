// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief meta header, project-wide compiler, operating system, standard library, and processor specific tweaks.
///\meta generic
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
