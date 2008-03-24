// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::exception
#pragma once
#include <universalis/exception.hpp>
#include <universalis/compiler/compiler.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system {

/// generic exception thrown by functions of the namespace universalis::operating_system.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK exception : public universalis::exception {
	public:
		inline exception(int const & code, compiler::location const & location) throw() : universalis::exception(location), code_(code) {}

	public:
		int const code() const throw() { return code_; }
	private:
		int const code_;

	public:
		char const * what() const throw() /*override*/;
	protected:
		std::string const mutable * what_;
};

namespace exceptions {
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK runtime_error : public universalis::exceptions::runtime_error {
		public:
			runtime_error(std::string const & what, compiler::location const &, void const * cause = 0) throw();
	};
}

}}
#include <universalis/compiler/dynamic_link/end.hpp>
