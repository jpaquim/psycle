// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::threads::sleep
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/numeric.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace threads
		{
			/// Makes the calling thread sleep for seconds.
			///\param seconds the duration to make the thread sleep for.
			///\throw exception on any error from the operating system. \todo elaborate on this... there are interruption signals
			void sleep(compiler::numeric<>::floating_point const & seconds) throw(exception);
			
			//class interrupted : public std::exception {};
			//void sleep_until_interrupted(compiler::numeric<>::floating_point const & seconds) throw(exception, interrupted);
		}
	}
}

// arch-tag: 3b5de4c4-1f9f-4e2e-a453-9337c9d9f48d
