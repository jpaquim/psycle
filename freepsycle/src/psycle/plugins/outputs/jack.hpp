// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::jack
#pragma once
#include <psycle/detail/project.hpp>
#include "../resource.hpp"
#include <jack/jack.h>
#include <condition>
#include <mutex>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__JACK
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

/// outputs to a soundcard device via jackd
class UNIVERSALIS__COMPILER__DYNAMIC_LINK jack : public resource {
	protected: friend class virtual_factory_access;
		jack(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
		virtual ~jack() throw();
	public:
		engine::ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
		bool opened()  const /*override*/;
		bool started() const /*override*/;
	protected:
		void do_open()    throw(engine::exception) /*override*/;
		void do_start()   throw(engine::exception) /*override*/;
		void do_process() throw(engine::exception) /*override*/;
		void do_stop()    throw(engine::exception) /*override*/;
		void do_close()   throw(engine::exception) /*override*/;
		void channel_change_notification_from_port(engine::port const &) throw(engine::exception) /*override*/;
	private:
		::jack_client_t * client_;
		std::vector< ::jack_port_t*> output_ports_;

		int static process_callback_static(::jack_nframes_t, void*);
		int        process_callback(::jack_nframes_t);

		int static set_sample_rate_callback_static(::jack_nframes_t, void*);
		int        set_sample_rate_callback(::jack_nframes_t);

		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;

		bool started_;
		bool stop_requested_;

		/// intermediate buffer between do_process() and the routine that writes to the jack buffer
		std::vector< ::jack_default_audio_sample_t> intermediate_buffer_;
		
		/// pointer to current jack read position in the intermediate buffer
		unsigned int intermediate_buffer_current_read_pointer_;
		
		std::vector< ::jack_default_audio_sample_t> last_samples_;
};

}}}
#include <universalis/compiler/dynamic_link/end.hpp>

