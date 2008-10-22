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
	private:
		::jack_client_t * client_;
		::jack_port_t * playback_port_;

		int static process_callback_static(::jack_nframes_t, void*);
		int        process_callback(::jack_nframes_t);

		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;

		bool started_;
		bool process_callback_called_;
		bool stop_requested_;

		/// intermediate buffer between do_process() and the routine that writes to the gstreamer buffer
		char * intermediate_buffer_;
		
		/// pointer to current gstreamer read position in the intermediate buffer
		char * intermediate_buffer_current_read_pointer_;
		
		/// pointer to end of intermediate buffer
		char * intermediate_buffer_end_;
		
		typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
		std::vector<output_sample_type> last_samples_;
};

}}}
#include <universalis/compiler/dynamic_link/end.hpp>

