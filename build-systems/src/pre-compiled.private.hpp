// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief inclusions of headers to be pre-compiled.

#pragma once

#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION // if the compiler supports pre-compilation

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("pre-compiling " __FILE__ " ...")
	#endif

	#include <diversalis/diversalis.hpp>
	#include <universalis/universalis.hpp>
	#include "pre-compiled/microsoft.private.hpp"
	#include "pre-compiled/standard.private.hpp"
	#include "pre-compiled/posix.private.hpp"
	#include "pre-compiled/boost.private.hpp"

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("pre-compiling " __FILE__ " ... done")
	#endif
#endif
