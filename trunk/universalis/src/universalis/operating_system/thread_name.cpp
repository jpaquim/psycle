// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::thread_name
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "thread_name.hpp"
#include "loggers.hpp"
#include <thread>
#include <cassert>

namespace universalis { namespace operating_system {

namespace {
	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE
	std::string const * tls_thread_name_(0);
}

std::string thread_name::get() {
	std::string nvr(tls_thread_name_ ? *tls_thread_name_ : "<unknown>");
	return nvr;
}

void thread_name::set(std::string const & name) {
	assert(!thread_name_.length() || &thread_name_ == tls_thread_name_);
	thread_name_ = name;
	set_tls();
}

void thread_name::set_tls() {
	if(operating_system::loggers::trace()) {
		std::ostringstream s;
		s << "setting name for thread: id: " << std::this_thread::id() << ", name: " << thread_name_;
		operating_system::loggers::trace()(s.str());
	}
	tls_thread_name_ = &thread_name_;
}

thread_name::~thread_name() {
	if(&thread_name_ == tls_thread_name_) tls_thread_name_ = 0;
}

}}
