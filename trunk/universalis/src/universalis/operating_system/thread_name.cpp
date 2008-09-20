// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::thread_name
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "thread_name.hpp"
#include "loggers.hpp"
#include <thread>

namespace universalis { namespace operating_system {

namespace {
	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE
	std::string const * thread_name_(0);
}

void thread_name::set(std::string const & name) {
	if(operating_system::loggers::trace()()) {
		std::ostringstream s;
		s << "setting name for thread: id: " << std::this_thread::id() << ", name: " << name;
		operating_system::loggers::trace()(s.str());
	}

	delete thread_name_;
	thread_name_ = new std::string(name);
}

std::string thread_name::get() {
	std::string nvr(thread_name_ ? *thread_name_ : "<unknown>");
	return nvr;
}

thread_name::~thread_name() {
	delete thread_name_;
}

}}
