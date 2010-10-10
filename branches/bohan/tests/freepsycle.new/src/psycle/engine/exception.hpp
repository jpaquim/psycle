// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::engine::exception
#ifndef PSYCLE__ENGINE__EXCEPTION__INCLUDED
#define PSYCLE__ENGINE__EXCEPTION__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include <string>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

/// generic exception thrown by functions of the namespace psycle::engine.
class exception : public universalis::exception {
	public:
		exception(
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
#include <psycle/detail/decl.hpp>
#endif
