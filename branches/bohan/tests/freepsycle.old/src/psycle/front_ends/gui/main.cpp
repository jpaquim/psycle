// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::front_ends::gui::main
#include <psycle/detail/project.private.hpp>
#include "main.hpp"
#include "lock.hpp"
#include "root.hpp"
#include <psycle/engine/engine.hpp>
#include <universalis/os/thread_name.hpp>
#include <exception>
#include <glibmm/exception.h>
#include <gtkmm/main.h>

#include <libgnomecanvasmm/init.h> // for Gnome::Canvas::init()

#include <clutter/clutter-version.h>
#if !CLUTTER_CHECK_VERSION(0, 6, 0)
	#error clutter library is too old
#endif
#include <clutter/clutter-main.h> // for clutter_init()

namespace psycle { namespace front_ends { namespace gui {

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	try {
		try {
			universalis::os::loggers::multiplex_logger::singleton().add(universalis::os::loggers::stream_logger::default_logger());
			universalis::os::thread_name thread_name("main");
			universalis::cpu::exceptions::install_handler_in_thread();
			// gtk_clutter_init(&argument_count, &arguments); replaces gtk_init(&argument_count, &arguments);
			lock::init(); // => threads support init
			Gnome::Canvas::init();
			//Glib::thread_init() or ::g_thread_init(); (then ::gdk_threads_init();)
			//clutter_threads_init();
			clutter_init(&argument_count, &arguments);
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
				s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
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
	return psycle::front_ends::gui::main(argument_count, arguments);
}
