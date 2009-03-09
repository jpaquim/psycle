// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

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
