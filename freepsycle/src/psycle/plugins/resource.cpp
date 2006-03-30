// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright © 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \implementation psycle::plugins::resource
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "resource.hpp"
namespace psycle
{
	namespace plugins
	{
		resource::resource(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(std::exception)
		:
			node(plugin_library_reference, graph, name)
		{
		}

		void resource::do_open() throw(std::exception)
		{
			node::do_open();
			if(loggers::information()())
			{
				std::ostringstream s;
				s << universalis::compiler::typenameof(*this) << ": opening";
				loggers::information()(s.str());
			}
		}
	
		void resource::do_start() throw(std::exception)
		{
			node::do_start();
			if(loggers::information()())
			{
				std::ostringstream s;
				s << universalis::compiler::typenameof(*this) << ": starting";
				loggers::information()(s.str());
			}
		}
	
		void resource::do_stop() throw(std::exception)
		{
			node::do_stop();
			if(loggers::information()())
			{
				std::ostringstream s;
				s << universalis::compiler::typenameof(*this) << ": stopping";
				loggers::information()(s.str());
			}
		}
	
		void resource::do_close() throw(std::exception)
		{
			node::do_close();
			if(loggers::information()())
			{
				std::ostringstream s;
				s << universalis::compiler::typenameof(*this) << ": closing";
				loggers::information()(s.str());
			}
		}
	}
}
