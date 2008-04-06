// -*- mode:c++; indent-tabs-mode:t -*-
// For the raison d'etre of this file,
// see the boost signals faq question number 3.
// http://www.boost.org/doc/html/signals/s04.html
#ifndef boost_signalslib
#define boost_signalslib

#ifdef signals
#error "signalslib.hpp must be included before any qt header"
#endif

#ifdef _MSC_VER
#pragma warning (disable:4251) // class 'X' needs to have dll-interface to be used by clients of class 'Y'
#pragma warning (disable:4275) // non dll-interface class 'X' used as base for dll-interface class 'Y'
#endif

#include <boost/signal.hpp>
namespace boost
{
	namespace signalslib = signals;
}

#endif
