// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::jack
#include <psycle/detail/project.private.hpp>
#include "jack.hpp"
namespace psycle { namespace plugins { namespace outputs {

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(jack)

	jack::jack(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
	:
		resource(plugin_library_reference, graph, name),
		client_(0),
		output_port_(0)
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	namespace {
		int process(jack_nframes_t, jack & jack);
	}

	void jack::do_open() throw(engine::exception) {
		resource::do_open();
		if(!(client_ = ::jack_client_new(PACKAGENERIC__MODULE__NAME))) throw engine::exception("could not create client to jack daemon", UNIVERSALIS__COMPILER__LOCATION);
		::jack_set_process_callback(client_, process, this);
		if(!(output_port = ::jack_port_register(client_, name().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) throw engine::exception("could not create output port in jack client", UNIVERSALIS__COMPILER__LOCATION);
	}

	bool jack::opened() const {
		return client_;
	}

	void jack::do_start() throw(engine::exception) {
		resource::do_start();
		if(::jack_activate(client_)) throw engine::exception("cannot activate client", UNIVERSALIS__COMPILER__LOCATION);
		char const ** ports(0);
		if(!(ports = ::jack_get_ports(client_, 0, 0, ::JackPortIsPhysical | ::JackPortIsInput))) throw engine::exception("could not find any physical playback ports", UNIVERSALIS__COMPILER__LOCATION);
		if(::jack_connect(client_, ::jack_port_name(output_port_), ports)) throw engine::exception("could not connect output port", UNIVERSALIS__COMPILER__LOCATION);
		std::free(ports);
	}

	bool jack::started() const {
		if(!opened()) return false;
		return true;
	}

	/// this is called from within psycle's host's processing thread.
	void jack::do_process() throw(engine::exception) {
		//boost::notify(condition);
		//boost::wait(condition);
	}
	
	namespace {
		/// this is called from within jack's processing thread.
		int process(jack_nframes_t nframes, jack & jack) {
			//jack.do_process_jack();
			//boost::wait(condition);
			//boost::notify(condition);
			return 0;
		}
	}

	void jack::do_stop() throw(engine::exception) {
		device::do_stop();
	}

	void jack::do_close() throw(engine::exception) {
		::jack_client_close(client_);
		client_ = 0;
		resource::do_close();
	}
}}}

