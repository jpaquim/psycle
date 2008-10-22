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
	playback_port_(),
	intermediate_buffer_()
{
	engine::ports::inputs::single::create_on_heap(*this, "in");
	engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
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
	if(status & ::JackServerStarted) loggers::information()("jack server started", UNIVERSALIS__COMPILER__LOCATION);
	if(status & ::JackNameNotUnique) {
		client_name = ::jack_get_client_name(client_);
		std::ostringstream s; s << "unique client name assigned: " << client_name;
		loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	{ // allocate the intermediate buffer
		std::size_t const bytes_per_sample(32); ///\todo corresponds to JACK_DEFAULT_AUDIO_TYPE, which normally is 32-bit float.
		std::size_t const bytes(static_cast<std::size_t>(parent().events_per_buffer() * bytes_per_sample));
		if(!(intermediate_buffer_ = new char[bytes])) {
			std::ostringstream s; s << "not enough memory to allocate " << bytes << " bytes on heap";
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		intermediate_buffer_end_ = intermediate_buffer_ + bytes;
	}

	// set to false so that started() returns false
	wait_for_state_to_become_playing_ = false;

	intermediate_buffer_current_read_pointer_ = intermediate_buffer_;
	stop_requested_ = process_callback_called_ = false;
	wait_for_state_to_become_playing_ = true;

	::jack_set_process_callback(client_, process_callback_static, (void*)this);
	if(!(playback_port_ = ::jack_port_register(client_, (client_name + "-in").c_str(), JACK_DEFAULT_AUDIO_TYPE, ::JackPortIsInput, 0))) throw engine::exceptions::runtime_error("could not create output port in jack client", UNIVERSALIS__COMPILER__LOCATION);

	{ scoped_lock lock(mutex_);
		while(!process_callback_called_) condition_.wait(lock);
		wait_for_state_to_become_playing_ = false;
	}
	condition_.notify_one();
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
	if(!(ports = ::jack_get_ports(client_, 0, 0, ::JackPortIsPhysical | ::JackPortIsInput))) throw engine::exceptions::runtime_error("could not find any physical playback ports", UNIVERSALIS__COMPILER__LOCATION);
	try {
		if(::jack_connect(client_, ports[0], ::jack_port_name(playback_port_))) throw engine::exceptions::runtime_error("could not connect output port", UNIVERSALIS__COMPILER__LOCATION);
	} catch(...) {
		std::free(ports);
		throw;
	}
	std::free(ports);
}

bool jack::started() const {
	if(!opened()) return false;
	return true;
}

/// this is called from within jack's processing thread.
int jack::process_callback_static(::jack_nframes_t frames, void * data) {
	return reinterpret_cast<jack*>(data)->process_callback(frames);
}

/// this is called from within jack's processing thread.
int jack::process_callback(::jack_nframes_t frames) {
	if(false && loggers::trace()) {
		std::ostringstream s; s << "process_callback";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	{ scoped_lock lock(mutex_);
		while(io_ready() && !stop_requested_ && !wait_for_state_to_become_playing_) condition_.wait(lock);
		if(stop_requested_) return 0;
		// process_callback is called before state is changed to playing.
		if(wait_for_state_to_become_playing_) {
			if(loggers::trace()()) loggers::trace()("process_callback called", UNIVERSALIS__COMPILER__LOCATION);
			process_callback_called_ = true;
			condition_.notify_one();
			while(wait_for_state_to_become_playing_) condition_.wait(lock);
		}
	}
	#if 1
	#else
	::guint8 * out(GST_BUFFER_DATA(&buffer));
	std::size_t out_size(GST_BUFFER_SIZE(&buffer));
	while(true) { // copy the intermediate buffer to the gstreamer buffer
		std::size_t const in_size(intermediate_buffer_end_ - intermediate_buffer_current_read_pointer_);
		std::size_t const copy_size(std::min(in_size, out_size));
		std::memcpy(out, intermediate_buffer_current_read_pointer_, copy_size);
		intermediate_buffer_current_read_pointer_ += copy_size;
		out_size -= copy_size;
		if(!out_size) break;
		{ scoped_lock lock(mutex_);
			io_ready(true);
		}
		condition_.notify_one();
		{ scoped_lock lock(mutex_);
			while(io_ready() && !stop_requested_) condition_.wait(lock);
		}
		if(stop_requested_) return;
		out += copy_size;
	}
	#endif
	if(intermediate_buffer_current_read_pointer_ == intermediate_buffer_end_) {
		{ scoped_lock lock(mutex_);
			io_ready(true);
		}
		condition_.notify_one();
	}
	return 0;
}

/// this is called from within psycle's host's processing thread.
void jack::do_process() throw(engine::exception) {
	if(false && loggers::trace()) {
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
			output_sample_type * out(reinterpret_cast<output_sample_type*>(intermediate_buffer_));
			unsigned int spread(0);
			for(std::size_t e(0), s(in.size()); e < s; ++e) {
				real s(in[e].sample());
				{
					s *= std::numeric_limits<output_sample_type>::max();
					if     (s < std::numeric_limits<output_sample_type>::min()) s = std::numeric_limits<output_sample_type>::min();
					else if(s > std::numeric_limits<output_sample_type>::max()) s = std::numeric_limits<output_sample_type>::max();
				}
				last_samples_[c] = static_cast<output_sample_type>(s);
				for( ; spread <= in[e].index() ; ++spread, ++out) out[c] = last_samples_[c];
			}
			for( ; spread < samples_per_buffer ; ++spread, ++out) out[c] = last_samples_[c];
		}
	}
	{ scoped_lock lock(mutex_);
		intermediate_buffer_current_read_pointer_ = intermediate_buffer_;
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
}

void jack::do_close() throw(engine::exception) {
	::jack_client_close(client_); client_ = 0;
	resource::do_close();
}

jack::~jack() throw() {
	close();
}

}}}

