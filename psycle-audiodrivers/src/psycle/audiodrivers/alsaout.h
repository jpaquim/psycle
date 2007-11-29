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
#ifndef ALSAOUT_H
#define ALSAOUT_H
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

		virtual AlsaOut* clone()  const;   // Uses the copy constructor

		virtual AudioDriverInfo info() const;

		virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
		virtual bool Initialized();
		virtual void configure();
		virtual bool Enable(bool e);

//    bool _timerActive;

private:

//    int iret1;
//    static void* pollerThread(void* ptr);
		void* _callbackContext;
		AUDIODRIVERWORKFN _pCallback;
		pthread_t threadid;


		bool _initialized;

		
		void setDefault();
		bool Start();
		bool Stop();


		/////// ALSA Stuff

		public :

		unsigned int rate;        // stream rate
		snd_pcm_format_t format;  // sample format
		unsigned int channels;    // count of channels
		unsigned int buffer_time; // ring buffer length in us
		unsigned int period_time; // period time in us

		snd_pcm_sframes_t buffer_size;
		snd_pcm_sframes_t period_size;
		snd_output_t *output;

		snd_pcm_t *handle;

		volatile int enablePlayer; // -3: thread not running, -2: stop thread!, -1: has stopped, 0: stop!, 1: play!, 2: is playing
		int method;       // 0:WRITE 1:WRITE&POLL 2:ASYNC 3:async_direct 4:direct_interleaved
											// 5:direct_noninterleaved 6:DIRECT_WRITE
		signed short *samples;
		snd_pcm_channel_area_t *areas;

		void FillBuffer(snd_pcm_uframes_t offset, int count);


		int id;
		signed short left;  // left out (getSample should change this non-stop if audio was started
		signed short right; // right out (getSample should change this non-stop if audio was started)
		void (*getSample) (void*);

		int audioStart( );
		int audioStop();

		int set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access);
		int set_swparams(snd_pcm_sw_params_t *swparams);
		int xrun_recovery(int err);
	int write_loop();

		static void* audioOutThread(void * ptr);
//    void setValues();

};

}}
#endif // defined PSYCLE__ALSA_AVAILABLE
#endif   
