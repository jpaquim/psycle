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
#include <pthread.h>

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
		virtual bool Initialized();
		virtual void configure();
		virtual bool Enable(bool e);

		//bool _timerActive; ///\todo remove?

	private:
		//int iret1; ///\todo remove?
		//static void* pollerThread(void* ptr); ///\todo remove?
		void* _callbackContext;
		AUDIODRIVERWORKFN _pCallback;
		pthread_t threadid;

		bool _initialized;
		
		void setDefault();
		bool Start();
		bool Stop();

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

		/// -3: thread not running, -2: stop thread!, -1: has stopped, 0: stop!, 1: play!, 2: is playing
		///\todo volatile is not meant to handle thread-shared data
		volatile int enablePlayer;
		
		/// 0:WRITE 1:WRITE&POLL 2:ASYNC 3:async_direct 4:direct_interleaved
		/// 5:direct_noninterleaved 6:DIRECT_WRITE
		int method;
		
		signed short *samples;
		snd_pcm_channel_area_t *areas;

		void FillBuffer(snd_pcm_uframes_t offset, int count);

		int id;
		/// left out (getSample should change this non-stop if audio was started
		signed short left;
		/// right out (getSample should change this non-stop if audio was started)
		signed short right;
		void (*getSample) (void*);

		int audioStart( );
		int audioStop();

		int set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access);
		int set_swparams(snd_pcm_sw_params_t *swparams);
		int xrun_recovery(int err);

		int write_loop();

		static void* audioOutThread(void * ptr);
    	//void setValues(); ///\todo remove?
};

}}
#endif // defined PSYCLE__ALSA_AVAILABLE
#endif // !defined PSYCLE__AUDIODRIVERS__ALSA_OUT__INCLUDED

