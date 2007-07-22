// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\implementation asio
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC

#include <asio_build_wrapper/asio.hpp>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4189) // host\ASIOConvertSamples.cpp(656) : warning C4189: 'b' : local variable is initialized but not referenced
#endif
#include <ASIOConvertSamples.cpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif
