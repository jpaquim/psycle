// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

// For the raison d'être of this file,
// see the boost signals faq question number 3.
// http://www.boost.org/doc/html/signals/s04.html

#ifndef PSYCLE__CORE__SIGNALSLIB__INCLUDED
#define PSYCLE__CORE__SIGNALSLIB__INCLUDED
#pragma once

#ifdef signals
	#error "<psycle/core/signalslib.hpp> must be included before any qt header"
#endif

#include <boost/signal.hpp>
namespace boost {
	namespace signalslib = signals;
}

#endif
