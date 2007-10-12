// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation psycle::engine::logger
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>

#if 0

#include "logger.hpp"
#include "exceptions/code_description.hpp"
#include <fstream>
namespace psycle {

namespace {
	std::ostream & ostream() throw() {
		try {
			class once {
				public:
					// [bohan] hmm, someone told me it is disallowed to have static functions in local classes, although gcc and msvc7.1 seem to allow it.
					static std::ostream & instanciate() {
						// this overwrites the file if it already exists.
						return *new std::ofstream(paths::package::log() / (paths::package::log().leaf() + ".log").c_str());
					}
			};
			static std::ofstream instance = once::instanciate();
			return instance;
		} catch(...) {
			std::cerr << "could not create logger output stream" << std::endl;
			return std::cout;
		}
	}
}

void logger::do_log(int const level, std::string const & string) throw() {
	try {
		// could reuse the default implementation, but here we've redefined it all
		//operating_system::logger::operator()(level, string);
		if((*this)(level)) {
			ostream() << "=== ";
			switch(level) {
				case universalis::operating_system::loggers::levels::trace: ostream() << "trace"; break;
				case universalis::operating_system::loggers::levels::information: ostream() << "information"; break;
				case universalis::operating_system::loggers::levels::warning: ostream() << "warning"; break;
				case universalis::operating_system::loggers::levels::exception: ostream() << "exception"; break;
				case universalis::operating_system::loggers::levels::crash: ostream() << "crash"; break;
				default: ostream() << "default";
			}
			ostream() << " ===" << std::endl;
			ostream() << string << std::endl;
			switch(level) {
				case universalis::operating_system::loggers::levels::trace:
				case universalis::operating_system::loggers::levels::warning:
				case universalis::operating_system::loggers::levels::exception:
				case universalis::operating_system::loggers::levels::crash:
					ostream().flush();
				case universalis::operating_system::loggers::levels::information:
				default:
			}
			//operating_system::terminal::log(level, string);
		}
	} catch(...) {
		// oh dear!
		// fallback to std::cerr
		std::cerr << "logger crashed" << std::endl;
		std::cerr << "logger: " << level << ": " << string;
	}
}

logger & logger::default_logger() throw() {
	try {
		static logger instance(operating_system::loggers::levels::trace, default_logger_ostream());
		return instance;
	} catch(...) {
		static logger instance(operating_system::loggers::levels::trace, std::cout);
		try {
			instance(operating_system::loggers::levels::exception, "could not create logger");
		} catch(...) {
			std::cerr << "could not create fallback logger" << std::endl;
		}
		return instance;
	}
}}
#endif // 0

