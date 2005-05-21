// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::terminal
#pragma once
#include <universalis/detail/project.hpp>
#include "exception.hpp"
#include <boost/thread/mutex.hpp>
#include <string>
namespace universalis
{
	namespace operating_system
	{
		/// terminal.
		class terminal
		{
			public:
				terminal() throw(exception);
				virtual ~terminal() throw();
				void output(int const & logger_level, std::string const & string);
			private:
				boost::mutex mutex_;
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					bool allocated_;
				#endif
		};
	}
}
