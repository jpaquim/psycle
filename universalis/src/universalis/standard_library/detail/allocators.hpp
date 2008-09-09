// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
#pragma once
#include <universalis/detail/project.hpp>
#include <cstddef>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__STANDARD_LIBRARY__DETAIL__ALLOCATORS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace standard_library { namespace allocators { namespace process {

///\internal
namespace detail {
	UNIVERSALIS__COMPILER__DYNAMIC_LINK void * allocate(std::size_t const &);
	UNIVERSALIS__COMPILER__DYNAMIC_LINK void deallocate(void *, std::size_t const &);
}

}}}}
#include <universalis/compiler/dynamic_link/end.hpp>
