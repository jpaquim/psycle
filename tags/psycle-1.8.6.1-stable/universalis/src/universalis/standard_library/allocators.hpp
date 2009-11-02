// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
#pragma once
#include <universalis/detail/project.hpp>
#include "detail/allocators.hpp"
#include <cstddef>
namespace universalis { namespace standard_library { namespace allocators { namespace process {

template<typename X>
class allocator {
	public:
		X * allocate(std::size_t const &) { return reinterpret_cast<X*>(detail::allocate(size * sizeof(X))); }
		void deallocate(X *, std::size_t const &) { detail::deallocate(x, size * sizeof(X)); }
};

}}}}
