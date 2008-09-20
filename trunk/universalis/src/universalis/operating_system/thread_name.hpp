// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::thread_name
#pragma once
#include <universalis/compiler/compiler.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__PROCESSOR__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system {

/// thread name
class UNIVERSALIS__COMPILER__DYNAMIC_LINK thread_name {
	public:
		/// create an instance without setting any name yet
		thread_name() {}

		/// sets a name for the current thread
		///\post the name is copied
		thread_name(std::string const & name) { set(name); }

		/// sets a name for the current thread
		///\post the name is copied
		void set(std::string const &);

		/// gets the name of the current thread
		std::string static get();

		/// unsets the name of the current thread
		~thread_name();
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
