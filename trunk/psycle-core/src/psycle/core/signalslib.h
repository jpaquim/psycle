// For the raison d'etre of this file,
// see the boost signals faq question number 3.
// http://www.boost.org/doc/html/signals/s04.html
#ifndef boost_signalslib
#define boost_signalslib

#ifdef signals
#error "signalslib.hpp must be included before any qt header"
#endif

#include <boost/signal.hpp>
namespace boost
{
	namespace signalslib = signals;
}

#endif
