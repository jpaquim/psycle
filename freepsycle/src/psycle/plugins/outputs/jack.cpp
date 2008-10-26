// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::jack
#include <psycle/detail/project.private.hpp>
#include "jack.hpp"
#include <universalis/processor/exception.hpp>
namespace psycle { namespace plugins { namespace outputs {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(jack)

jack::jack(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
:
	resource(plugin_library_reference, graph, name),
	client_(),
	started_()
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

int jack::set_sample_rate_callback_static(::jack_nframes_t sample_rate, void * data) {
	return reinterpret_cast<jack*>(data)->set_sample_rate_callback(sample_rate);
}

int jack::set_sample_rate_callback(::jack_nframes_t sample_rate) {
	in_port().events_per_second(sample_rate);
	return 0;
}

void jack::do_open() throw(engine::exception) {
	resource::do_open();
	
	// open a client
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
	if(!(client_ = ::jack_client_open(client_name.c_str(), ::JackNullOption, &status, server_name))) {
		std::ostringstream s; s << "could not open client: status: " << status;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(status & ::JackServerStarted) loggers::information()("jack server was not running, so it has been started", UNIVERSALIS__COMPILER__LOCATION);
	if(status & ::JackNameNotUnique) {
		client_name = ::jack_get_client_name(client_);
		std::ostringstream s; s << "unique client name assigned: " << client_name;
		loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	// get the sample rate
	{
		::jack_nframes_t const sample_rate(::jack_get_sample_rate(client_));
		if(sample_rate != static_cast< ::jack_nframes_t>(in_port().events_per_second())) {
			if(loggers::information()) {
				std::ostringstream s;
				s << "samples rate: " << sample_rate << "Hz";
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			in_port().events_per_second(sample_rate);
		}
	}
	// set the sample rate change callback
	if(int error = ::jack_set_sample_rate_callback(client_, set_sample_rate_callback_static, (void*)this)) {
		std::ostringstream s; s << "could not set sample rate callback: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	// register some output ports
	output_ports_.resize(in_port().channels());
	for(unsigned int i(0); i < in_port().channels(); ++i) {
		std::ostringstream port_name; port_name << "output_" << i + 1;
		if(!(output_ports_[i] = ::jack_port_register(client_, port_name.str().c_str(), JACK_DEFAULT_AUDIO_TYPE, ::JackPortIsOutput, /* buffer size ignored for built-in types */ 0)))
			throw engine::exceptions::runtime_error("could not register output port", UNIVERSALIS__COMPILER__LOCATION);
	}

	// allocate the intermediate buffer
	intermediate_buffer_.resize(parent().events_per_buffer() * in_port().channels());
	//intermediate_buffer_current_read_pointer_ = intermediate_buffer_;

	// set the thread init callback
	if(int error = ::jack_set_thread_init_callback(client_, thread_init_callback_static, (void*)this)) {
		std::ostringstream s; s << "could not set thread init callback: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	// set the process callback
	stop_requested_ = false;
	if(int error = ::jack_set_process_callback(client_, process_callback_static, (void*)this)) {
		std::ostringstream s; s << "could not set process callback: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

}

bool jack::opened() const {
	return client_;
}

void jack::do_start() throw(engine::exception) {
	resource::do_start();
	// activate the client
	if(int error = ::jack_activate(client_)) {
		std::ostringstream s; s << "could not activate client: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	#if 1 // automatically connect the ports
		char const ** ports;
		if(!(ports = ::jack_get_ports(client_, /* port name regexp */ 0, /* type name regexp */ 0, ::JackPortIsPhysical | ::JackPortIsInput)))
			throw engine::exceptions::runtime_error("could not find any physical playback/input ports", UNIVERSALIS__COMPILER__LOCATION);
		try {
			if(loggers::trace()) {
				std::ostringstream s; s << "physical playback/input ports:";
				for(unsigned int i(0); ports[i]; ++i) s << ' ' << ports[i];
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(in_port().channels() == 1 && ports[0] && ports[1]) {
				// connect mono to stereo
				if(::jack_connect(client_, ::jack_port_name(output_ports_[0]), ports[0]))
					throw engine::exceptions::runtime_error("could not connect ports", UNIVERSALIS__COMPILER__LOCATION);
				if(::jack_connect(client_, ::jack_port_name(output_ports_[0]), ports[1]))
					throw engine::exceptions::runtime_error("could not connect ports", UNIVERSALIS__COMPILER__LOCATION);
			} else if(in_port().channels() > 1 && ports[0] && !ports[1])
				// connect many to mono
				for(unsigned int i(0); i < in_port().channels(); ++i) {
					if(::jack_connect(client_, ::jack_port_name(output_ports_[i]), ports[0]))
						throw engine::exceptions::runtime_error("could not connect ports", UNIVERSALIS__COMPILER__LOCATION);
				}
			else // connect many to many
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
	#endif
	started_ = true;
}

bool jack::started() const {
	return started_;
}

/// this is called from within jack's processing thread.
void jack::thread_init_callback_static(void * data) {
	reinterpret_cast<jack*>(data)->thread_init_callback();
}

/// this is called from within jack's processing thread.
void jack::thread_init_callback() {
	if(loggers::information()) loggers::information()("jack thread started", UNIVERSALIS__COMPILER__LOCATION);

	// set thread name
	thread_name_.set(universalis::compiler::typenameof(*this) + "#" + qualified_name());

	// install cpu/os exception handler/translator
	universalis::processor::exception::install_handler_in_thread();
}

/// this is called from within jack's processing thread.
int jack::process_callback_static(::jack_nframes_t frames, void * data) {
	return reinterpret_cast<jack*>(data)->process_callback(frames);
}

/// this is called from within jack's processing thread.
int jack::process_callback(::jack_nframes_t frames) {
	if(false && loggers::trace()) loggers::trace()("process_callback", UNIVERSALIS__COMPILER__LOCATION);
	::jack_transport_state_t ts = ::jack_transport_query(client_, /* ::jack_position_t* */ 0);
	if(false /* always plays, doesn't support rolling */ && ts != ::JackTransportRolling) {
		// emit silence
		if(loggers::trace()) loggers::trace()("transport not rolling => emitting silence", UNIVERSALIS__COMPILER__LOCATION);
		for(unsigned int c(0); c < in_port().channels(); ++c) {
			::jack_default_audio_sample_t * const out(reinterpret_cast< ::jack_default_audio_sample_t*>(jack_port_get_buffer(output_ports_[c], frames)));
			std::memset(out, 0, sizeof *out * frames);
		}
	} else // copy the intermediate buffer to the jack buffer
		for(unsigned int c(0); c < in_port().channels(); ++c) {
			::jack_default_audio_sample_t * const out(reinterpret_cast< ::jack_default_audio_sample_t*>(jack_port_get_buffer(output_ports_[c], frames)));
			for(std::size_t i(0); i < frames; ++i) {
				#if 1 // basic 440Hz sine wave test
					double static x(0);
					out[i] = 0.1 * std::sin(x);
					x += 2 * 3.14 * 440 / 44100;
				#else
					out[i] = intermediate_buffer_[i + c];
				#endif
			}
		}
	return 0;
}

/// this is called from within psycle's host's processing thread.
void jack::do_process() throw(engine::exception) {
	if(loggers::trace()) loggers::trace()("process", UNIVERSALIS__COMPILER__LOCATION);
	if(!in_port()) return;
	{ scoped_lock lock(mutex_);
		if(false && loggers::warning() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
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
	if(int error = ::jack_deactivate(client_)) {
		std::ostringstream s; s << "could not deactivate client: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	resource::do_stop();
	started_ = false;
}

void jack::do_close() throw(engine::exception) {
	if(int error = ::jack_client_close(client_)) {
		std::ostringstream s; s << "could not close client: " << error;
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	client_ = 0;
	resource::do_close();
}

jack::~jack() throw() {
	close();
}

}}}
