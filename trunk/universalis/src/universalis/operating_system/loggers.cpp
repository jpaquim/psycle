// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::loggers
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "loggers.hpp"
#include <cstdlib>
#include <algorithm> // for std::min
namespace universalis { namespace operating_system {

/**********************************************************************************************************/
// logger

void logger::log(int const level, std::string const & message, compiler::location const & location) throw() {
	#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION__MAJOR > 7
		///\todo this is crashing here at runtime with msvc7.1 ... some static var doesn't seem to get initialized properly
		///\todo test again on msvc7.1 since the code has been redesigned.
		boost::mutex::scoped_lock lock(mutex());
		do_log(level, message, location);
	#endif
}

void logger::log(int const level, std::string const & string) throw() {
	#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION__MAJOR > 7
		///\todo this is crashing here at runtime with msvc7.1 ... some static var doesn't seem to get initialized properly
		///\todo test again on msvc7.1 since the code has been redesigned.
		boost::mutex::scoped_lock lock(mutex());
		do_log(level, string);
	#endif
}

namespace loggers {

/**********************************************************************************************************/
// multiplex_logger

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

void multiplex_logger::do_log(int const level, std::string const & message, compiler::location const & location) throw() {
	for(iterator i(begin()) ; i != end() ; ++i) (**i).log(level, message, location);
}

void multiplex_logger::do_log(int const level, std::string const & string) throw() {
	for(iterator i(begin()) ; i != end() ; ++i) (**i).log(level, string);
}

/**********************************************************************************************************/
// stream_logger

stream_logger stream_logger::default_logger_(std::cout /* std::clog is unbuffered by default, this can be changed at runtime tho */);

stream_logger::stream_logger(std::ostream & ostream) : ostream_(ostream) {}

namespace {
	// Here we assume we have an ansi terminal if the TERM env var is defined.
	bool const ansi_terminal(std::getenv("TERM"));
	
	void dump_location(compiler::location const & location, std::ostream & out) {
		if(ansi_terminal) out << "\033[34m";
		out
			<< "# "
			<< location.module() << " # "
			<< location.function() << " # "
			<< location.file() << ":"
			<< location.line();
		if(ansi_terminal) out << "\033[0m";
	}
}

void stream_logger::do_log(int const level, std::string const & message, compiler::location const & location) throw() {
	std::ostringstream s;
	dump_location(location, s);
	s << '\n' << message;
	do_log(level, s.str());
}

// [bohan] msvc-7.1 crashes if we put this function in the implementation file instead of inlined in the header.
void stream_logger::do_log(int const level, std::string const & string) throw() {
	int const static levels [] = {'T', 'I', 'W', 'E', 'C'};
	int const static colors [] = {0, 2, 6, 1, 5, 3, 4, 7};
	char const level_char(levels[std::min(static_cast<unsigned int>(level), sizeof levels)]);
	try {
		if(ansi_terminal) ostream() << "\033[1;3" << colors[level % sizeof colors] << "mlogger: " << level_char << ": \033[0m" << string << '\n';
		else ostream() << "logger: " << level_char << ": " << string << '\n';
	} catch(...) {
		// oh dear!
		// report the error to std::cerr ...
		dump_location(UNIVERSALIS__COMPILER__LOCATION, std::cerr);
		if(ansi_terminal) std::cerr << "\033[1;31m";
		std::cerr << "logger crashed!";
		if(ansi_terminal) std::cerr << "\033[0m";
		std::cerr << std::endl;
		
		// ... and fallback to std::clog
		if(ansi_terminal) std::clog << "\033[1;3" << colors[level % sizeof colors] << "mlogger: " << level_char << ": \033[0m";
		else std::clog << "logger: " << level_char << ": ";
		std::clog << string << '\n';
	}
}

}}}
