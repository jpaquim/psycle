// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::logger
#include <psycle/detail/project.hpp>
#include <universalis/operation_system/loggers.hpp>
#include <fstream>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__LOGGER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK logger : public universalis::operating_system::loggers::stream_logger {
	public:
		logger(std::ostream & ostream) : universalis::operating_system::loggers::stream_logger(ostream) {}
	protected:
		void do_log(int const level, std::string const & string) throw() /*override*/;
};

}
#include <universalis/compiler/dynamic_link/end.hpp>

