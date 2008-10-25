// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::jack
#include <psycle/detail/project.private.hpp>
#include "jack.hpp"
namespace psycle { namespace plugins { namespace outputs {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(jack)

jack::jack(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
:
	resource(plugin_library_reference, graph, name),
	client_(),
	started_(),
	intermediate_buffer_()
{
	engine::ports::inputs::single::create_on_heap(*this, "in");
	engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
}

void jack::channel_change_notification_from_port(engine::port const & port) throw(engine::exception) {
	if(&port == &in_port()) {
		last_samples_.resize(port.channels());
		for(std::size_t i(0); i < last_samples_.size(); ++i) last_samples_[i] = 0;
	}
	resource::channel_change_notification_from_port(port);
}

void jack::do_open() throw(engine::exception) {
	resource::do_open();
	char const * server_name(0);
	std::string client_name(
		#if defined PACKAGENERIC__MODULE__NAME
			PACKAGENERIC__MODULE__NAME
		#else
			"psycle"
		#endif
	);
	std::size_t const client_name_max_lengh(::jack_client_name_size());
	if(client_name.length() > client_name_max_lengh) client_name = client_name.substr(0, client_name_max_lengh);
	::JackStatus status;
	if(!(client_ = ::jack_client_open(client_name.c_str(), ::JackNullOption, &status, server_name
	))) {
		std::ostringstream s; s << "could not open client: status: " << status;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(status & ::JackServerStarted) loggers::information()("jack server was not running, so it has been started", UNIVERSALIS__COMPILER__LOCATION);
	if(status & ::JackNameNotUnique) {
		client_name = ::jack_get_client_name(client_);
		std::ostringstream s; s << "unique client name assigned: " << client_name;
		loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	// allocate the intermediate buffer
	intermediate_buffer_.resize(parent().events_per_buffer() * in_port().channels());
	//intermediate_buffer_current_read_pointer_ = intermediate_buffer_;

	stop_requested_ = process_callback_called_ = false;
	::jack_set_process_callback(client_, process_callback_static, (void*)this);

	output_ports_.resize(in_port().channels());
	for(unsigned int i(0); i < in_port().channels(); ++i) {
		std::ostringstream port_name; port_name << client_name << '-' << i;
		if(!(output_ports_[i] = ::jack_port_register(client_, port_name.str().c_str(), JACK_DEFAULT_AUDIO_TYPE, ::JackPortIsOutput, 0)))
			throw engine::exceptions::runtime_error("could not register output port", UNIVERSALIS__COMPILER__LOCATION);
	}
}

bool jack::opened() const {
	return client_;
}

void jack::do_start() throw(engine::exception) {
	resource::do_start();
	if(int err = ::jack_activate(client_)) {
		std::ostringstream s; s << "cannot activate client: " << err;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	char const ** ports;
	if(!(ports = ::jack_get_ports(client_, 0, 0, ::JackPortIsPhysical | ::JackPortIsInput)))
		throw engine::exceptions::runtime_error("could not find any physical playback ports", UNIVERSALIS__COMPILER__LOCATION);
	try {
		if(loggers::trace()()) {
			std::ostringstream s; s << "input ports:";
			for(unsigned int i(0); ports[i]; ++i) s << ' ' << ports[i];
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		for(unsigned int i(0); i < in_port().channels(); ++i) {
			if(!ports[i]) {
				loggers::warning()("cannot connect every port: less input ports than output ports");
				break;
			}
			if(::jack_connect(client_, ::jack_port_name(output_ports_[i]), ports[i]))
				throw engine::exceptions::runtime_error("could not connect ports", UNIVERSALIS__COMPILER__LOCATION);
		}
	} catch(...) {
		std::free(ports);
		throw;
	}
	std::free(ports);
	started_ = true;
}

bool jack::started() const {
	return started_;
}

/// this is called from within jack's processing thread.
int jack::process_callback_static(::jack_nframes_t frames, void * data) {
	return reinterpret_cast<jack*>(data)->process_callback(frames);
}

/// this is called from within jack's processing thread.
int jack::process_callback(::jack_nframes_t frames) {
	if(loggers::trace()) {
		std::ostringstream s; s << "process_callback";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	{ scoped_lock lock(mutex_);
		while(io_ready() && !stop_requested_) condition_.wait(lock);
		if(stop_requested_) return 0;
	}
	::jack_transport_state_t ts = jack_transport_query(client_, 0);
	if(ts == ::JackTransportRolling) {
		if(!process_callback_called_) {
			process_callback_called_= true;
			condition_.notify_one();
		}
		// copy the intermediate buffer to the jack buffer
		for(unsigned int c(0); c < in_port().channels(); ++c) {
			::jack_default_audio_sample_t * const out(reinterpret_cast< ::jack_default_audio_sample_t*>(jack_port_get_buffer(output_ports_[c], frames)));
			for(std::size_t i(0); i << frames; ++i) {
				out[i] = intermediate_buffer_[i + c];
			}
		}
	} else if(ts == ::JackTransportStopped) {
		if(process_callback_called_) {
			process_callback_called_= false;
			condition_.notify_one();
		}
	}
	return 0;
}

/// this is called from within psycle's host's processing thread.
void jack::do_process() throw(engine::exception) {
	if(loggers::trace()) {
		std::ostringstream s; s << "process";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!in_port()) return;
	{ scoped_lock lock(mutex_);
		if(false && loggers::warning()() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
		while(!io_ready()) condition_.wait(lock);
	}
	{ // fill the intermediate buffer
		unsigned int const samples_per_buffer(parent().events_per_buffer());
		assert(last_samples_.size() == in_port().channels());
		for(unsigned int c(0); c < in_port().channels(); ++c) {
			engine::buffer::channel & in(in_port().buffer()[c]);
			unsigned int spread(0);
			for(std::size_t e(0), s(in.size()); e < s; ++e) {
				last_samples_[c] = in[e].sample();
				for( ; spread <= in[e].index() ; ++spread) intermediate_buffer_[spread + c] = last_samples_[c];
			}
			for( ; spread < samples_per_buffer ; ++spread) intermediate_buffer_[spread + c] = last_samples_[c];
		}
	}
	{ scoped_lock lock(mutex_);
		//intermediate_buffer_current_read_pointer_ = intermediate_buffer_;
		io_ready(false);
	}
	condition_.notify_one();
}

void jack::do_stop() throw(engine::exception) {
	if(int err = ::jack_deactivate(client_)) {
		std::ostringstream s; s << "cannot deactivate client: " << err;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	resource::do_stop();
	started_ = false;
}

void jack::do_close() throw(engine::exception) {
	::jack_client_close(client_); client_ = 0;
	resource::do_close();
}

jack::~jack() throw() {
	close();
}

}}}
