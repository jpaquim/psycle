// copyright 2006-2007 psycledelics http://psycle.sourceforge.net
/*************************************************************************
*  This program is covered by the GNU General Public License:            *
*  http://gnu.org/licenses/gpl.html                                      *
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
*  This program is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
*  See the GNU General Public License for more details.                  *
*                                                                        *
*  You should have received a copy of the GNU General Public License     *
*  along with this program; if not, write to the                         *
*  Free Software Foundation, Inc.,                                       *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
**************************************************************************/

#if defined PSYCLE__ALSA_AVAILABLE

#include "alsa_out.h"
#include <cstring>
#include <iostream>

namespace psy {
	namespace core {
	
	AlsaOut::AlsaOut( ) :
		AudioDriver()
		,_callbackContext(NULL)
		,_pCallback(NULL)
		,threadid(0)
		,_initialized(false)
		,rate(0)
		,channels(0)
		,buffer_time(0)
		,period_time(0)
		,buffer_size(0)
		,period_size(0)
		,output(NULL)
		,handle(NULL)
		,enablePlayer(-3)
		,method(0)
		,samples(NULL)
		,areas(NULL)
		,id(0)
		,left(0)
		,right(0)
	{
		std::memset(&format, 0, sizeof format);
		setDefault();
	}
	
	AlsaOut::~AlsaOut()
	{
		enablePlayer = -2;
		while(enablePlayer != -3) usleep(1000); ///\todo use proper synchronisation technic
		/// \todo free memory here
	}
	
	AlsaOut * AlsaOut::clone( ) const
	{
		return new AlsaOut(*this);
	}
	
	AudioDriverInfo AlsaOut::info( ) const
	{
		return AudioDriverInfo("alsa","Alsa Driver","Low Latency audio driver",true);
	}
	
	void AlsaOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
	{
		_pCallback = pCallback;
		_callbackContext = context;
		_initialized = true;
		audioStart();
	}
	
	bool AlsaOut::Enable( bool e )
	{
		return e ? Start() : Stop();
	}
	
	bool AlsaOut::Start( )
	{
		// start thread (might already be running, but that is ok.)
		if (!audioStart())
		return false;

		// make thread start using the callback
		enablePlayer = 1;
		while(enablePlayer != 2) usleep(1000); ///\todo use proper synchronisation technic
		return true;
	}
	
	bool AlsaOut::Stop( )
	{
		audioStop();
		return true;
	}
	
	void AlsaOut::configure( )
	{
	}
	
	////////////// Begin audio.c to c++
	
	void AlsaOut::setDefault( )
	{
		rate = 44100; // stream rate
		format = SND_PCM_FORMAT_S16; // sample format
		channels = 2; // count of channels
		buffer_time = 80000; // ring buffer length in us
		period_time = 20000; // period time in us
		
		buffer_size=0;
		period_size=0;
		output = NULL;
		// AlsaOut::enablePlayer = 1; // has stopped, 0: stop!, 1: play!, 2: is playing
	}
	
	int AlsaOut::audioStop( )
	{
		if (enablePlayer < 0) return 1; ///\todo use proper synchronisation technic
		enablePlayer = 0;
		
		while (enablePlayer >= 0) usleep(1000); ///\todo use proper synchronisation technic
		return 1;
	}

	// starts the alsa thread
	int AlsaOut::audioStart(  )
	{
		if (enablePlayer >= -1) return 1; ///\todo use proper synchronisation technic
		enablePlayer = -3;
		int err;
		snd_pcm_hw_params_t *hwparams;
		snd_pcm_sw_params_t *swparams;
		unsigned int chn;
		
		snd_pcm_hw_params_alloca(&hwparams);
		snd_pcm_sw_params_alloca(&swparams);
		
		err = snd_output_stdio_attach(&output, stdout, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: output failed: << snd_strerror(err) << "\n";
			return 0;
		}
		
		std::cout << "psycle: alsa: playback device is: " << settings().deviceName() << "\n";
		std::cout << "psycle: alsa: stream parameters are: "
			<< "rate: " << rate << "Hz, "
			<< "format: " << snd_pcm_format_name(format) << ", "
			<< "channels: " << channels << "\n";
		std::cout << "psycle: alsa: using transfer method: " << "write" << "\n";

		#if !defined NDEBUG		
			std::cout << "psycle: alsa: step 1\n";
		#endif
		
		if ((err = snd_pcm_open(&handle, settings().deviceName().c_str() , SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			std::cerr << "psycle: alsa: playback open error: " << snd_strerror(err) << "\n";
			return 0;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2\n";
		#endif
		
		if ((err = set_hwparams(hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
			std::cerr << "psycle: alsa: setting of hwparams failed: " << snd_strerror(err) << "\n";
			return 0;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 3\n";
		#endif
		
		if ((err = set_swparams(swparams)) < 0) {
			std::cerr << "psycle: alsa: setting of swparams failed: " << snd_strerror(err)) << "\n";
			return 0;
		}

		#if !defined NDEBUG		
			std::cout << "psycle: alsa: step 4\n";
		#endif
		
		samples = (short int*) malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
		if (samples == NULL) {
			std::cerr << "psycle: alsa: not enough memory\n";
			return 0;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 5\n";
		#endif
		
		areas = (snd_pcm_channel_area_t*) calloc(channels, sizeof(snd_pcm_channel_area_t));
		if (areas == NULL) {
			std::cerr << "psycle: alsa: not enough memory\n";
			return 0;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 6\n";
		#endif
		
		for (chn = 0; chn < channels; chn++) {
			areas[chn].addr = samples;
			areas[chn].first = chn * 16;
			areas[chn].step = channels * 16;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 7";
		#endif
		
		enablePlayer = 0;
		if (0 != pthread_create(&threadid, NULL,&AlsaOut::audioOutThread, (void*) this))
		{
			enablePlayer = -3;
			std::cerr << "psycle: alsa: failed to create thread.\n";
			return 0;
		}
		return 1;
	}
	
	void AlsaOut::FillBuffer(snd_pcm_uframes_t offset, int count )
	{
		signed short *samples[channels];
		int steps[channels];
		unsigned int chn;
		
		// verify and prepare the contents of areas */
		for (chn = 0; chn < channels; chn++) {
			if ((areas[chn].first % 8) != 0) {
				std::cerr << "areas[" << chn << "].first == " << areas[chn].first << ", aborting...\n";
				return;
			}
			samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
			if ((areas[chn].step % 16) != 0) {
				std::cerr << "areas[" << chn << "].step == " << areas[chn].step << ", aborting...\n";
				return;
			}
			steps[chn] = areas[chn].step / 16;
			samples[chn] += offset * steps[chn];
		}
		
		// fill the channel areas
		
		float const * input(_pCallback(_callbackContext, count));
		
		while (count-- > 0) {
			*samples[0] = static_cast<short int>( *input++ );
			samples[0] += steps[0];
			*samples[1] = static_cast<short int>( *input++ );
			samples[1] += steps[1];
		}
	}
	
	int AlsaOut::set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access)
	{
		unsigned int rrate;
		snd_pcm_uframes_t size;
		int err, dir;
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.1\n";
		#endif
		
		// choose all parameters
		err = snd_pcm_hw_params_any(handle, params);
		if (err < 0) {
			std::cerr << "psycle: alsa: broken configuration for playback: no configurations available: " << snd_strerror(err) << "\n";
			return err;
		}
		
		#if 0 // set hardware resampling
			err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
			if (err < 0) {
				std::cerr << "psycle: alsa: resampling setup failed for playback: " << snd_strerror(err) << "\n";
				return err;
			}
		#endif
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.2\n";
		#endif
		
		// set the interleaved read/write format
		err = snd_pcm_hw_params_set_access(handle, params, access);
		if (err < 0) {
			std::cerr << "psycle: alsa: access type not available for playback: %s" << snd_strerror(err) << "\n";
			return err;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.3\n";
		#endif
		
		// set the sample format
		err = snd_pcm_hw_params_set_format(handle, params, format);
		if (err < 0) {
			std::cerr << "psycle: alsa: sample format not available for playback: "<< snd_strerror(err) << "\n";
			return err;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.4\n";
		#endif
		
		// set the count of channels
		err = snd_pcm_hw_params_set_channels(handle, params, channels);
		if (err < 0) {
			std::cerr << "Channels count (" << channels << ") not available for playbacks: " << snd_strerror(err) << "\n";
			return err;
		}
		
		// set the stream rate
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.5\n";
		#endif
		
		rrate = rate;
		err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: rate " << rate << "Hz not available for playback: " << snd_strerror(err) << "\n";
			return err;
		}

		#if !defined NDEBUG		
			std::cout << "psycle: alsa: step 2.6\n";
		#endif
		
		if (rrate != rate) {
			std::cerr << "psycle: alsa: rate doesn't match (requested " << rate << "Hz, got " << err << "Hz)\n";
			return -EINVAL;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.7\n";
		#endif

		// set the buffer time
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set buffer time " << buffer_time << " for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.8\n";
		#endif
		
		err = snd_pcm_hw_params_get_buffer_size(params, &size);
		if (err < 0) {
			std::cerr << ("psycle: alsa: unable to get buffer size for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.9\n";
		#endif
		
		buffer_size = size;
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.9.1\n";
		#endif
		
		// set the period time
		err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set period time " << period_time << " for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		
		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.10\n";
		#endif
		
		err = snd_pcm_hw_params_get_period_size(params, &size, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to get period size for playback: " <<  snd_strerror(err) << "\n";
			return err;
		}
		
		period_size = size;

		#if !defined NDEBUG
			std::cout << "psycle: alsa: step 2.10.1\n";
		#endif
		
		// write the parameters to device
		err = snd_pcm_hw_params(handle, params);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set hw params for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		return 0;
	}
	
	int AlsaOut::set_swparams(snd_pcm_sw_params_t *swparams)
	{
		int err;
		// get the current swparams
		err = snd_pcm_sw_params_current(handle, swparams);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to determine current swparams for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		// start the transfer when the buffer is almost full:
		// (buffer_size / avail_min) * avail_min
		err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set start threshold mode for playback: " <<  snd_strerror(err) << "\n";
			return err;
		}
		// allow the transfer when at least period_size samples can be processed
		err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set avail min for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		// align all transfers to 1 sample
		err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set transfer align for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		// write the parameters to the playback device
		err = snd_pcm_sw_params(handle, swparams);
		if (err < 0) {
			std::cerr << "psycle: unable to set sw params for playback: " << snd_strerror(err) << "\n";
			return err;
		}
		return 0;
	}
	
	/// underrun and suspend recovery
	int AlsaOut::xrun_recovery(int err)
	{
		if (err == -EPIPE) {    /* under-run */
			err = snd_pcm_prepare(handle);
			if (err < 0) std::cerr << "psycle: alsa: cannot recover from underrun, prepare failed: " << snd_strerror(err) << "\n";
			return 0;
		} else if (err == -ESTRPIPE) {
			while ((err = snd_pcm_resume(handle)) == -EAGAIN)
				sleep(1);       /* wait until the suspend flag is released */
				///\todo check where this sleep function comes from, also can't we use proper synchronisation in this case?
			if (err < 0) {
				err = snd_pcm_prepare(handle);
				if (err < 0) std::cerr << "psycle: alsa: cannot recover from suspend, prepare failed: " << snd_strerror(err) << "\n";
			}
			return 0;
		}
		return err;
	}
	
	int AlsaOut::write_loop()
	{
		signed short *ptr;
		int err, cptr;
		while (1) {
			if (enablePlayer >0) {
				enablePlayer = 2;
				FillBuffer(0, period_size);
				ptr = samples;
				cptr = period_size;
				while (cptr > 0) {
					err = snd_pcm_writei(handle, ptr, cptr);
					if (err == -EAGAIN) continue;
					if (err < 0) {
						if (xrun_recovery(err) < 0) {
							std::cerr << "psycle: alsa: write error: " << snd_strerror(err) << "\n";
							return -1;
						}
						break;  /* skip one period */
					}
					ptr += err * channels;
					cptr -= err;
				}
			}
			else if (enablePlayer == -2) return 0;
			else enablePlayer = -1;
			usleep(1000); ///\todo use proper synchronisation technic
		}
	}

	#if 0 // This appears to not be used:
		struct transfer_method {
			const char *name;
			snd_pcm_access_t access;
			int (*transfer_loop)(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
		};
	#endif
	
	void* AlsaOut::audioOutThread( void * ptr )
	{
		AlsaOut* pDriver = (AlsaOut*) ptr;
		if (pDriver->areas) pDriver->areas[0] = pDriver->areas[0];
		int err = pDriver->write_loop();
		if (err < 0) std::cerr << "psycle: alsa: transfer failed: " << snd_strerror(err) << "\n";
		pDriver->enablePlayer = -3;
		return NULL;
	}
	
	bool AlsaOut::Initialized( )
	{
		return _initialized;
	}
	
}
}

#endif
