// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\implementation asio
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC

#include <asio_build_wrapper/asio.hpp>

// Note: It's not totally clear whether test makes sense for macOSX/Darwin on the PC architecture.
#if defined DIVERSALIS__OPERATING_SYSTEM__APPLE
	#include <asioshlib.cpp>
	#include <codefragments.cpp>
#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <asiolist.cpp>
#endif
