// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface universalis::operating_system::terminal
#pragma once
#include <universalis/detail/project.hpp>
#include "exception.hpp"
#include <boost/thread/mutex.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__TERMINAL
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		/// terminal.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK terminal
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
#include <universalis/compiler/dynamic_link/end.hpp>
