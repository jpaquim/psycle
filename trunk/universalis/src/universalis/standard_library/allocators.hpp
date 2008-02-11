// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
#include "detail/allocators.hpp"
#include <cstddef>
namespace universalis
{
	namespace standard_library
	{
		namespace allocators
		{
			namespace process
			{
				template<typename X>
				class allocator
				{
					public:
						X inline * allocate(std::size_t const &);
						void inline deallocate(X *, std::size_t const &);
				};
			}
		}
	}
}
namespace universalis
{
	namespace standard_library
	{
		namespace allocators
		{
			namespace process
			{
				template<typename X>
				X inline * allocator<X>::allocate(std::size_t const & size) { return reinterpret_cast<X*>(detail::allocate(size * sizeof(X))); }

				template<typename X>
				void inline allocator<X>::deallocate(X * x, std::size_t const & size) { detail::deallocate(x, size * sizeof(X)); }
			}
		}
	}
}
