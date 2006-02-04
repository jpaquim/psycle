// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <universalis/detail/project.private.hpp>
#include "allocators.hpp"
#include <diversalis/compiler.hpp>
#include <memory>
namespace universalis
{
	namespace standard_library
	{
		namespace allocators
		{
			namespace process
			{
				namespace detail
				{
					namespace
					{
						static std::allocator<char> allocator;
					}

					#if 0 && !defined DIVERSALIS__COMPILER__SHARED
						#error "This translation unit must be linked into a shared library (linux/*bsd/darwin), or bundle (darwin), or DLL (mswindows)."
					#endif

					void * allocate(std::size_t const & size)
					{
						return allocator.allocate(size);
					}

					void deallocate(void * pointer, std::size_t const & size)
					{
						allocator.deallocate(reinterpret_cast<char*>(pointer), size);
					}
				}
			}
		}
	}
}
