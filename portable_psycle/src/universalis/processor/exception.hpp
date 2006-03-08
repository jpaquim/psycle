// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::processor::exception
#pragma once
#include <universalis/exception.hpp>
namespace universalis
{
	namespace processor
	{
		/// external cpu/os exception translated into a c++ one.
		class exception : public universalis::exception
		{
			public:
				exception(unsigned int const & code, compiler::location const & location) throw();
				/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
				/// [bohan] This is needed for microsoft, i don't know about linux/*bsd/gcc.
				void static new_thread(std::string const & = "");
		};
	}
}
