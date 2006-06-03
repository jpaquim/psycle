// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright � 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\implementation psycle::front_ends::gui::main
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include "lock.hpp"
#include "root.hpp"
#include <psycle/paths.hpp>
#include <psycle/engine/engine.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <universalis/processor/exception.hpp>
#include <exception>
#include <glibmm/exception.h>
#include <gtkmm/main.h>
#include <libgnomecanvasmm/init.h> // for Gnome::Canvas::init()
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[])
			{
				try
				{
					try
					{
						universalis::operating_system::loggers::multiplex_logger::singleton().add(universalis::operating_system::loggers::stream_logger::default_logger());
						universalis::processor::exception::new_thread("psycle.gui");
						{
							if(universalis::operating_system::loggers::information()())
							{
								std::ostringstream s;
								s << paths::package::name() << " " << paths::package::version::string();
								universalis::operating_system::loggers::information()(s.str());
							}
						}
						lock::init();
						Gnome::Canvas::init();
						Gtk::Main main(argument_count, arguments);
						engine::hello hello;
						engine::graph & graph(engine::graph::create("graph"));
						root window(graph, hello);
						{
							lock lock;
							main.run(window);
						}
						graph.destroy();
					}
					catch(Glib::Exception const & e)
					{
						if(loggers::exception()())
						{
							std::ostringstream s;
							s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
							loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						throw;
					}
					catch(std::exception const & e)
					{
						if(loggers::exception()())
						{
							std::ostringstream s;
							s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
							loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						throw;
					}
					catch(...)
					{
						if(loggers::exception()())
						{
							std::ostringstream s;
							s << "exception: " << universalis::compiler::exceptions::ellipsis();
							loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						throw;
					}
				}
				catch(...)
				{
					std::string s; std::getline(std::cin, s);
					throw;
				}
				return 0;
			}
		}
	}
}
