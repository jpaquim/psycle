// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
#include <cstddef>
namespace universalis
{
	namespace standard_library
	{
		namespace allocators
		{
			namespace process
			{
				///\internal
				namespace detail
				{
					void * allocate(std::size_t const &);
					void deallocate(void *, std::size_t const &);
				}
			}
		}
	}
}
