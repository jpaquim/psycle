// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file

#ifndef UNIVERSALIS__STDLIB__ALLOCATORS__INCLUDED
#define UNIVERSALIS__STDLIB__ALLOCATORS__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include "detail/allocators.hpp"
#include <cstddef>

namespace universalis { namespace stdlib { namespace allocators { namespace process {

template<typename X>
class allocator {
	public:
		X * allocate(std::size_t const & size) { return reinterpret_cast<X*>(detail::allocate(size * sizeof(X))); }
		void deallocate(X * x, std::size_t const & size) { detail::deallocate(x, size * sizeof(X)); }
};

}}}}

#endif
