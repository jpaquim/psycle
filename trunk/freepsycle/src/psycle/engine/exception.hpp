// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::engine::exception
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/exception.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// generic exception thrown by functions of the namespace psycle::engine.
class exception : public universalis::exception {
	public:
		inline exception(
			universalis::compiler::location const & location,
			void const * cause = 0
		) throw() : universalis::exception(location, cause) {}
};

namespace exceptions {
	/// runtime errors thrown by functions of the namespace psycle::engine.
	class runtime_error : public universalis::exceptions::runtime_error {
		public:
			runtime_error(
				std::string const & what,
				universalis::compiler::location const & location,
				void const * cause = 0
			) throw() : universalis::exceptions::runtime_error(what, location, cause) {}
	};
}}}

#include <universalis/compiler/dynamic_link/end.hpp>
