// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::resource
#include <psycle/detail/project.private.hpp>
#include "resource.hpp"
namespace psycle { namespace plugins {

resource::resource(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	node(plugin_library_reference, name)
{}

void resource::do_open() {
	node::do_open();
	if(loggers::information()) {
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << ": opening";
		loggers::information()(s.str());
	}
}

void resource::do_start() {
	node::do_start();
	if(loggers::information()) {
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << ": starting";
		loggers::information()(s.str());
	}
}

void resource::do_stop() {
	node::do_stop();
	if(loggers::information()) {
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << ": stopping";
		loggers::information()(s.str());
	}
}

void resource::do_close() {
	node::do_close();
	if(loggers::information()) {
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << ": closing";
		loggers::information()(s.str());
	}
}

}}
