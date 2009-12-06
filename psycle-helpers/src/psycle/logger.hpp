// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 members of the psycle project http://psycle.pastnotecut.org

#ifndef PSYCLE__LOGGER__INCLUDED
#define PSYCLE__LOGGER__INCLUDED
#pragma once

// uncomment the following define to enable a very verbose logger
//#define PSYCLE_LOG_BE_VERBOSE

#ifdef PSYCLE_LOG_BE_VERBOSE
	#include <universalis/os/loggers.hpp>
#else
	#include <iostream>
#endif

namespace psycle { namespace logger {

void inline init() {
	#ifdef PSYCLE_LOG_BE_VERBOSE
		using namespace universalis::os::loggers;
		multiplex_logger::singleton().add(stream_logger::default_logger());
	#endif
}

#define PSYCLE_LOG_TRACE(args) PSYCLE_LOG_COUT_(universalis::os::loggers::trace, args)
#define PSYCLE_LOG_INFO(args)  PSYCLE_LOG_COUT_(universalis::os::loggers::information, args)
#define PSYCLE_LOG_WARN(args)  PSYCLE_LOG_CERR_(universalis::os::loggers::warning, args)
#define PSYCLE_LOG_ERROR(args) PSYCLE_LOG_CERR_(universalis::os::loggers::exception, args)

#ifdef PSYCLE_LOG_BE_VERBOSE

	#define PSYCLE_LOG_COUT_(logger, args) PSYCLE_LOG_(logger, args)
	#define PSYCLE_LOG_CERR_(logger, args) PSYCLE_LOG_(logger, args)
	#define PSYCLE_LOG_(logger, args) \
		do { \
			if(logger()()) { \
				std::ostringstream s; \
				s << args; \
				logger()(s.str(), UNIVERSALIS__COMPILER__LOCATION); \
			} \
		} while(false)

#else

	#define PSYCLE_LOG_COUT_(logger, args) PSYCLE_LOG_(std::cout, logger, args)
	#define PSYCLE_LOG_CERR_(logger, args) PSYCLE_LOG_(std::cerr, logger, args)
	#define PSYCLE_LOG_(stream, logger, args) \
		do { \
			stream << args << '\n'; \
		} while(false)

#endif

}}

#endif
