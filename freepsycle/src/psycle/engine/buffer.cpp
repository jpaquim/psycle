// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation psycle::engine::buffer
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "buffer.hpp"
namespace psycle { namespace engine {

buffer::buffer(std::size_t channels, std::size_t events) throw(std::exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "new buffer " << this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	resize(channels, events);
}

buffer::~buffer() throw() {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "delete buffer " << this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void buffer::resize(std::size_t channels, std::size_t events) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "buffer " << this << " resizing to " << channels << " channels of " << events << " events each";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	//assert(channels != 0 || events == 0 && "channels == 0 implies events == 0"); // why enforcing this ?
	std::vector<channel>::resize(channels);
	events_ = events;
	for(iterator i(begin()) ; i != end() ; ++i) i->resize(events);
	clear(size());
}

}}

