// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::jack
#ifndef PSYCLE__PLUGINS__OUTPUTS__JACK__INCLUDED
#define PSYCLE__PLUGINS__OUTPUTS__JACK__INCLUDED
#pragma once
#include "../resource.hpp"
#include <psycle/helpers/ring_buffer.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <jack/jack.h>
#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUTS__JACK
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins { namespace outputs {

/// outputs to a soundcard device via jackd
class PSYCLE__DECL jack : public resource {
	public:
		jack(class plugin_library_reference &, name_type const &);
		virtual ~jack() throw();
		engine::ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
		bool opened()  const /*override*/;
		bool started() const /*override*/;
	protected:
		void do_open() /*override*/;
		void do_start() /*override*/;
		void do_process() /*override*/;
		void do_stop() /*override*/;
		void do_close() /*override*/;
		void channel_change_notification_from_port(port const &) /*override*/;
	private:
		::jack_client_t * client_;
		std::vector< ::jack_port_t*> output_ports_;

		int static process_callback_static(::jack_nframes_t, void*);
		int        process_callback(::jack_nframes_t);

		int static set_sample_rate_callback_static(::jack_nframes_t, void*);
		int        set_sample_rate_callback(::jack_nframes_t);

		void static thread_init_callback_static(void*);
		void        thread_init_callback();
		universalis::os::thread_name thread_name_;
		
		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;

		bool started_;
		bool stop_requested_;

		/// intermediate buffer between do_process() and the routine that writes to the gstreamer buffer
		char * intermediate_buffer_;

		typedef helpers::ring_buffer<> ring_buffer_type;
		/// read and write positions within the intermediate buffer
		ring_buffer_type ring_buffer_;

		std::vector< ::jack_default_audio_sample_t> last_samples_;
};

}}}
#include <psycle/detail/decl.hpp>
#endif
