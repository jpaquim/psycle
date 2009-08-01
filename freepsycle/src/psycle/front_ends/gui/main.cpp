// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\implementation psycle::front_ends::gui::main
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include "lock.hpp"
#include "root.hpp"
#include <psycle/paths.hpp>
#include <psycle/engine/engine.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <universalis/os/loggers.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/cpu/exception.hpp>
#include <exception>
#include <glibmm/exception.h>
#include <gtkmm/main.h>
#include <libgnomecanvasmm/init.h> // for Gnome::Canvas::init()
namespace psycle { namespace front_ends { namespace gui {

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[])
{
	try {
		try {
			universalis::os::loggers::multiplex_logger::singleton().add(universalis::os::loggers::stream_logger::default_logger());
			universalis::os::thread_name thread_name("main");
			universalis::processor::exception::install_handler_in_thread();
			if(universalis::os::loggers::information()()) {
				std::ostringstream s;
				s << paths::package::name() << " " << paths::package::version::string();
				universalis::os::loggers::information()(s.str());
			}
			lock::init();
			Gnome::Canvas::init();
			Gtk::Main main(argument_count, arguments);
			engine::graph & graph(engine::graph::create_on_heap("graph"));
			root window(graph);
			{
				lock lock;
				main.run(window);
			}
			graph.free_heap();
		} catch(Glib::Exception const & e) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
			throw;
		} catch(std::exception const & e) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
			throw;
		} catch(...) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "exception: " << universalis::compiler::exceptions::ellipsis();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
			throw;
		}
	} catch(...) {
		std::string s; std::getline(std::cin, s);
		throw;
	}
	return 0;
}

}}}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	psycle::front_ends::gui::main(argument_count, arguments);
}

