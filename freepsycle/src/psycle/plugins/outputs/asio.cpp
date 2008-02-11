/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::asio
#include <psycle/detail/project.private.hpp>
#include "asio.hpp"
namespace psycle { namespace plugins { namespace outputs {

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(asio)

	asio::asio(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
	:
		resource(plugin_library_reference, graph, name)
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void asio::do_open() throw(engine::exception) {
		resource::do_open();
	}

	bool asio::opened() const {
		return true;
	}

	void asio::do_start() throw(engine::exception) {
		resource::do_start();
	}

	bool asio::started() const {
		if(!opened()) return false;
		return true;
	}

	/// this is called from within psycle's host's processing thread.
	void asio::do_process() throw(engine::exception) {
		//boost::notify(condition);
		//boost::wait(condition);
	}
	
	namespace {
		/// this is called from within asio's processing thread.
		int process() {
			//boost::wait(condition);
			//boost::notify(condition);
			return 0;
		}
	}

	void asio::do_stop() throw(engine::exception) {
		resource::do_stop();
	}

	void asio::do_close() throw(engine::exception) {
		resource::do_close();
	}
}}}

