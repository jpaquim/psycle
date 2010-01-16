// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED
#pragma once

#if defined PSYCLE__ALSA_AVAILABLE
#include "audiodriver.h"
#include <alsa/asoundlib.h>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <universalis/stdlib/cstdint.hpp>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

class AlsaOut : public AudioDriver {
	public:
		AlsaOut();
		~AlsaOut() throw();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() throw(std::exception) {}
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception) {}

	private:
		/// stream rate
		unsigned int rate;
		/// sample format
		snd_pcm_format_t format;
		/// count of channels
		unsigned int channels;
		/// ring buffer length in us
		unsigned int buffer_time;
		/// period time in us
		unsigned int period_time;

		snd_pcm_sframes_t buffer_size;
		snd_pcm_sframes_t period_size;
		snd_output_t *output;

		snd_pcm_t *handle;

		/// 0:WRITE 1:WRITE&POLL 2:ASYNC 3:async_direct 4:direct_interleaved
		/// 5:direct_noninterleaved 6:DIRECT_WRITE
		int method;
		
		int16_t * samples;
		snd_pcm_channel_area_t *areas;

		void FillBuffer(snd_pcm_uframes_t offset, int count);

		int id;

		/// left out (getSample should change this non-stop if audio was started
		int16_t left;

		/// right out (getSample should change this non-stop if audio was started)
		int16_t right;

		void (*getSample) (void*);

		void set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access);
		void set_swparams(snd_pcm_sw_params_t *swparams);
		int xrun_recovery(int err);

		///\name thread
		///\{
			/// the function executed by the alsa thread
			void thread_function();
			/// whether the thread is running
			bool running_;
			/// whether the thread is asked to terminate
			bool stop_requested_;
			/// a mutex to synchronise accesses to running_ and stop_requested_
			mutex mutex_;
			typedef class scoped_lock<mutex> scoped_lock;
			/// a condition variable to wait until notified that the value of running_ has changed
			condition<scoped_lock> condition_;
		///\}
};

}}
#endif // defined PSYCLE__ALSA_AVAILABLE
#endif
