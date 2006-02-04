// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::processor::exception
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/compiler.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__PROCESSOR__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace processor
	{
		/// external cpu/os exception translated into a c++ one, with deferred querying of the human-readable message.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK exception : public universalis::operating_system::exception
		{
			public:
				/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
				void static new_thread(std::string const & = "");

			public:
					#if defined NBEBUG
						inline
					#endif
						exception(unsigned int const & code, compiler::location const & location) throw()
							#if defined NDEBUG
								: universalis::operating_system::exception(code, location) {}
							#else
								;
							#endif

			public:
				char const UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES * what() const throw();
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
