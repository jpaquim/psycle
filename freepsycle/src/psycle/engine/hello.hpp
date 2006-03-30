// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::hello
#pragma once
#include <psycle/detail/project.hpp>
#include <boost/thread.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__HELLO
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		/// psycle's engine's hello.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK hello
		{
			public:
				/// hello message
				///\return a hello message
				std::string say_hello() const throw();
			private:
				boost::mutex mutex_;
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
