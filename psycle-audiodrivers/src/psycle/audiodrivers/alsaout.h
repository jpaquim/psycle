// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#ifndef PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED
#if defined PSYCLE__ALSA_AVAILABLE
#include "audiodriver.h"
#include <alsa/asoundlib.h>
#include <mutex>
#include <condition>
#include <cstdint>

/**
@author Psycledelics
*/
namespace psy { namespace core {

class AlsaOut : public AudioDriver
{
	public:
		AlsaOut();
		~AlsaOut();
		/// uses the copy constructor
		virtual AlsaOut* clone() const;
		virtual AudioDriverInfo info() const;

		virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
		virtual bool Initialized() { return _initialized; }
		virtual void configure() {}
		virtual bool Enable(bool e) { return e ? Start() : Stop(); }

	private:
		void* _callbackContext;
		AUDIODRIVERWORKFN _pCallback;

		bool _initialized;
		
		void setDefault();
		bool Start();
		bool Stop();

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
		
		std::int16_t * samples;
		snd_pcm_channel_area_t *areas;

		void FillBuffer(snd_pcm_uframes_t offset, int count);

		int id;

		/// left out (getSample should change this non-stop if audio was started
		std::int16_t left;

		/// right out (getSample should change this non-stop if audio was started)
		std::int16_t right;

		void (*getSample) (void*);

		int set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access);
		int set_swparams(snd_pcm_sw_params_t *swparams);
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
			std::mutex mutex_;
			/// a condition variable to wait until notified that the value of running_ has changed
			std::condition<std::scoped_lock<std::mutex> > condition_;
		///\}
};

}}
#endif // defined PSYCLE__ALSA_AVAILABLE
#endif // !defined PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED

