// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::threads::sleep

/**************************** note ****************************
the C++ standardisation commitee is working on a threading api that will provide an equivalent interface:
#include <thread>
template<typename Elapsed_Time> std::this_thread::sleep(Elapsed_Time &);
***************************************************************/

#pragma once
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/numeric.hpp>
#include <date_time>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  UNIVERSALIS__OPERATING_SYSTEM__THREADS__SLEEP
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system { namespace threads {

/// Makes the calling thread sleep for nanoseconds.
///\throw exception on any error from the operating system. \todo elaborate on this... there are interruption signals
UNIVERSALIS__COMPILER__DYNAMIC_LINK void sleep(std::nanoseconds const &) throw(exception);

//class UNIVERSALIS__COMPILER__DYNAMIC_LINK interrupted : public std::exception {};
//void sleep_until_interrupted(std::nanoseconds const &) throw(exception, interrupted);

}}}
#include <universalis/compiler/dynamic_link/end.hpp>
