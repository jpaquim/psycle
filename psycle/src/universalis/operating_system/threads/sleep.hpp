// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface universalis::operating_system::threads::sleep
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/numeric.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  UNIVERSALIS__OPERATING_SYSTEM__THREADS__SLEEP
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace threads
		{
			/// Makes the calling thread sleep for seconds.
			///\param seconds the duration to make the thread sleep for.
			///\throw exception on any error from the operating system. \todo elaborate on this... there are interruption signals
			void UNIVERSALIS__COMPILER__DYNAMIC_LINK sleep(compiler::numeric<>::floating_point const & seconds) throw(exception);
			
			//class UNIVERSALIS__COMPILER__DYNAMIC_LINK interrupted : public std::exception {};
			//void sleep_until_interrupted(compiler::numeric<>::floating_point const & seconds) throw(exception, interrupted);
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
