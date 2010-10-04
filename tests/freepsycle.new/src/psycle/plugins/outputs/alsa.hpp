// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::alsa
#pragma once
#include "../resource.hpp"
#include <alsa/asoundlib.h>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUTS__ALSA
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins { namespace outputs {

using namespace universalis::stdlib;

/// outputs to a soundcard device via alsa output implementation.
class PSYCLE__DECL alsa : public resource {
	public:
		alsa(class plugin_library_reference &, name_type const &) throw(exception);
		virtual ~alsa() throw();
		ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
		bool opened()  const /*override*/;
		bool started() const /*override*/;
	protected:
		void do_open()    throw(engine::exception) /*override*/;
		void do_start()   throw(engine::exception) /*override*/;
		void do_process() throw(engine::exception) /*override*/;
		void do_stop()    throw(engine::exception) /*override*/;
		void do_close()   throw(engine::exception) /*override*/;
	private:
		/// pcm device handle
		::snd_pcm_t * pcm_;

		/// attached to std output
		::snd_output_t * output_;

		/// samples per period (member data because set in do_open and read in do_start)
		::snd_pcm_uframes_t period_frames_;

		/// buffer size in samples (member data because set in do_open and read in do_start)
		::snd_pcm_uframes_t buffer_frames_;

		/// intermediate buffer between do_process() and the routine that writes to the alsa device
		char * intermediate_buffer_;
		
		/// pointers to areas within the buffer in memory-mapped access method
		//::snd_pcm_channel_area_t * areas_;

		/// bits per channel sample
		unsigned int bits_per_channel_sample_;

		class thread * thread_;
		void thread_function();
		void poll_loop() throw(exception);
	
		typedef outputs::scoped_lock<mutex> scoped_lock;
		mutex mutable mutex_;
		condition<scoped_lock> mutable condition_;

		bool stop_requested_;

		ports::inputs::single in_, amp_;
};

}}}
#include <psycle/detail/decl.hpp>
