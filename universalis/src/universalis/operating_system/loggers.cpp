// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::loggers
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "loggers.hpp"
namespace universalis { namespace operating_system {

void logger::log(int const level, std::string const & string) throw() {
	#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION__MAJOR > 7
		//\todo this is crashing here at runtime with msvc7.1
		// some static var doesn't seem to get initialized properly
		boost::mutex::scoped_lock lock(mutex());
		do_log(level, string);
	#endif
}

namespace loggers {
	multiplex_logger multiplex_logger::singleton_;
	
	bool multiplex_logger::add(logger & logger) {
		boost::mutex::scoped_lock lock(mutex());
		iterator i(std::find(begin(), end(), &logger));
		if(i != end()) return false;
		push_back(&logger);
		return true;
	}

	bool multiplex_logger::remove(logger const & logger) {
		boost::mutex::scoped_lock lock(mutex());
		iterator i(std::find(begin(), end(), &logger));
		if(i == end()) return false;
		erase(i);
		return true;
	}
	
	void multiplex_logger::do_log(int const level, std::string const & string) throw() {
		for(iterator i(begin()) ; i != end() ; ++i) (**i).log(level, string);
	}

	stream_logger stream_logger::default_logger_(std::cout /* std::clog is unbuffered by default, this can be changed at runtime tho */);

	stream_logger::stream_logger(std::ostream & ostream) : ostream_(ostream) {}

	///\internal
	#define UNIVERSALIS__OPERATING_SYSTEM__LOGGERS__DETAIL__LOCATION_MARK__START "\033[34m# "
	///\internal
	#define UNIVERSALIS__OPERATING_SYSTEM__LOGGERS__DETAIL__LOCATION_MARK__END   "\033[0m"

	// [bohan] msvc-7.1 crashes if we put this function in the implementation file instead of inlined in the header.
	void stream_logger::do_log(int const level, std::string const & string) throw() {
		// Here we assume we have an ansi terminal if the TERM env var is defined.
		bool const ansi_terminal(std::getenv("TERM"));
		int const static color [] = {0, 2, 6, 1, 5, 3, 4, 7};
		try {
			if(ansi_terminal) ostream() << "\033[1;3" << color[level % sizeof color] << "mlogger: " << level << ": \033[0m" << string << '\n';
			else ostream() << "logger: " << level << ": " << string << '\n';
		} catch(...) {
			// oh dear!
			// report the error to std::cerr ...
			if(ansi_terminal) std::cerr << UNIVERSALIS__COMPILER__LOCATION << "\033[1;31mlogger crashed!\033[0m" << std::endl;
			else std::cerr << UNIVERSALIS__COMPILER__LOCATION << "logger crashed!" << std::endl;
			// ... and fallback to std::clog
			if(ansi_terminal) std::clog << "\033[1;3" << color[level % sizeof color] << "mlogger: " << level << ": \033[0m" << string << '\n';
			else std::clog << "logger: " << level << ": " << string << '\n';
		}
	}
}}}

