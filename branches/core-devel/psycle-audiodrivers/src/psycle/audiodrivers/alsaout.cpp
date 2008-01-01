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
#if defined PSYCLE__ALSA_AVAILABLE
#include "alsaout.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <thread>
#include <boost/bind.hpp>
namespace psy { namespace core {

AlsaOut::AlsaOut()
	:
		AudioDriver(),
		_callbackContext(),
		_pCallback(),
		_initialized(),
		rate(),
		channels(),
		buffer_time(),
		period_time(),
		buffer_size(),
		period_size(),
		output(),
		handle(),
		method(),
		samples(),
		areas(),
		id(),
		left(),
		right(),
		enablePlayer(-3)
	{
		std::memset(&format, 0, sizeof format);
		setDefault();
	}
	
	AlsaOut::~AlsaOut() {
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			if(enablePlayer > 0) {
				enablePlayer = -2; condition_.notify_one();
				while(enablePlayer != -3) condition_.wait(lock);
			}
		}
		/// \todo free memory here
	}
	
	AlsaOut * AlsaOut::clone() const {
		// we need a hand-written version because not everything can/must be cloned:
		// e.g., we don't clone the mutex, the condition, nor the thread state.
		AlsaOut & r(*new AlsaOut);
		r._callbackContext = _callbackContext;
		r._pCallback = _pCallback;
		r._initialized = _initialized;
		r.rate = rate;
		r.format = format;
		r.channels = channels;
		r.buffer_time = buffer_time;
		r.period_time = period_time;
		r.buffer_size = buffer_size;
		r.period_size = period_size;
		r.output = output;
		r.handle = handle;
		r.method = method;
		r.samples = samples;
		r.areas = areas;
		r.id = id;
		r.left = left;
		r.right = right;
		r.enablePlayer = -3; // doesn't make sense to copy the thread state since we don't clone the thread!
		return &r;
	}
	
	AudioDriverInfo AlsaOut::info() const {
		return AudioDriverInfo("alsa", "Alsa Driver", "Low Latency audio driver", true);
	}
	
	void AlsaOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context ) {
		_pCallback = pCallback;
		_callbackContext = context;
		_initialized = true;
		//audioStart();
	}
	
	bool AlsaOut::Start() {
		// start thread (might already be running, but that is ok.)
		if(!audioStart()) return false;

		// make thread start using the callback
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			enablePlayer = 1; condition_.notify_one();
			while(enablePlayer != 2) condition_.wait(lock);
		}

		return true;
	}
	
	bool AlsaOut::Stop() {
		audioStop();
		return true;
	}
	
	void AlsaOut::setDefault() {
		rate = 44100; // stream rate
		format = SND_PCM_FORMAT_S16; // sample format
		channels = 2; // count of channels
		// safe values, and usual ones too on other programs
		buffer_time = 200000;
		period_time = 20000;
		
		buffer_size=0;
		period_size=0;
		output = NULL;
		
		AudioDriverSettings settings(this->settings());
		{
			char const * const env(std::getenv("ALSA_CARD"));
			if(env) settings.setDeviceName(env);
			else settings.setDeviceName("default");
		}
		this->setSettings(settings);
	}
	
	int AlsaOut::audioStop() {
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			if(enablePlayer < 0) return 1;
			enablePlayer = 0; condition_.notify_one();
			while(enablePlayer >= 0) condition_.wait(lock);
		}
		return 1;
	}
	
	/// starts the alsa thread
	int AlsaOut::audioStart() {
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			if(enablePlayer >= -1) return 1;
			enablePlayer = -3; condition_.notify_one();
		}

		int err;
		snd_pcm_hw_params_t *hwparams;
		snd_pcm_sw_params_t *swparams;
		
		snd_pcm_hw_params_alloca(&hwparams);
		snd_pcm_sw_params_alloca(&swparams);
		
		err = snd_output_stdio_attach(&output, stdout, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: attaching output failed: " << snd_strerror(err) << '\n';
			return 0;
		}
		
		std::cout << "psycle: alsa: playback device is: " << settings().deviceName() << '\n';
		std::cout << "psycle: alsa: stream parameters are; " << rate << "Hz, " << snd_pcm_format_name(format) << ", " << channels << " channels\n";
		std::cout << "psycle: alsa: using transfer method: " << "write" << '\n'; ///\todo parametrable?
		
		if((err = snd_pcm_open(&handle, settings().deviceName().c_str() , SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			std::cerr << "psycle: alsa: playback open error: " << snd_strerror(err) << '\n';
			return 0;
		}
		
		if((err = set_hwparams(hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
			std::cerr << "psycle: alsa: setting of hwparams failed: " << snd_strerror(err) << '\n';
			return 0;
		}
		
		if((err = set_swparams(swparams)) < 0) {
			std::cerr << "psycle: alsa: setting of swparams failed: " << snd_strerror(err) << '\n';
			return 0;
		}
		
		samples = (short int*) malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
		if(!samples) {
			std::cerr << "out of memory\n";
			return 0;
		}
		
		areas = (snd_pcm_channel_area_t*) calloc(channels, sizeof(snd_pcm_channel_area_t));
		if(!areas) {
			std::cerr << "out of memory\n";
			return 0;
		}
		
		for(unsigned int chn(0); chn < channels; ++chn) {
			areas[chn].addr = samples;
			areas[chn].first = chn * 16;
			areas[chn].step = channels * 16;
		}
		
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			enablePlayer = 1; condition_.notify_one();
		}

		std::thread t(boost::bind(&AlsaOut::thread_function, this));
		return 1;
	}
	
	void AlsaOut::FillBuffer(snd_pcm_uframes_t offset, int count) {
		signed short *samples[channels];
		int steps[channels];
		// verify and prepare the contents of areas
		for (unsigned int chn(0); chn < channels; ++chn) {
			if ((areas[chn].first % 8) != 0) {
				std::ostringstream s;
				s << "psycle: alsa: areas[" << chn << "].first == " << areas[chn].first << ", aborting.";
				throw std::runtime_error(s.str().c_str());
			}
			samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
			if ((areas[chn].step % 16) != 0) {
				std::ostringstream s;
				s <<  "psycle: alsa: areas[" << chn << "].step == " << areas[chn].step << ", aborting.";
				throw std::runtime_error(s.str().c_str());
			}
			steps[chn] = areas[chn].step / 16;
			samples[chn] += offset * steps[chn];
		}
		// fill the channel areas
		float const * input(_pCallback(_callbackContext, count));
		
		Quantize16AndDeinterlace(input,samples[0],steps[0],samples[1],steps[1],count);
		samples[0]+=steps[0]*count;
		samples[1]+=steps[1]*count;
	}
	
	int AlsaOut::set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access) {
		snd_pcm_uframes_t size;
		int err;
		
		// choose all parameters
		err = snd_pcm_hw_params_any(handle, params);
		if (err < 0) {
			std::cerr << "psycle: alsa: broken configuration for playback: no configurations available: " << snd_strerror(err) << '\n';
			return err;
		}
		#if 0
			// set hardware resampling
			err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
			if (err < 0) {
				std::cerr << "psycle: alsa: resampling setup failed for playback: " << snd_strerror(err) << '\n';
				return err;
			}
		#endif
		// set the interleaved read/write format
		err = snd_pcm_hw_params_set_access(handle, params, access);
		if (err < 0) {
			std::cerr << "psycle: alsa: access type not available for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// set the sample format
		err = snd_pcm_hw_params_set_format(handle, params, format);
		if (err < 0) {
			std::cerr << "psycle: alsa: sample format not available for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// set the count of channels
		err = snd_pcm_hw_params_set_channels(handle, params, channels);
		if (err < 0) {
			std::cerr << "psycle: alsa: channels count (" << channels << ") not available for playback: " << snd_strerror(err) << '\n';
			return err;
		}

		// set the stream rate
		{
			unsigned int rrate(rate);
			err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
			if (err < 0) {
				std::cerr << "psycle: alsa: rate "<< rate << "Hz not available for playback: " << snd_strerror(err) << '\n';
				return err;
			}
			if(rrate != rate) {
				std::cerr << "psycle: alsa: rate does not match (requested " << rate << "Hz, got " << err << "Hz)\n";
				return -EINVAL;
			}
		}
		
		// set the buffer time
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set buffer time " << buffer_time << " for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		
		err = snd_pcm_hw_params_get_buffer_size(params, &size);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to get buffer size for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		
		std::cout << "step2.9, buffer size:" << size << std::endl;
		buffer_size = size;

		// set the period time
		err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set period time " << period_time << " for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		
		err = snd_pcm_hw_params_get_period_size(params, &size, 0);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to get period size for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		
		period_size = size;

		// write the parameters to device
		err = snd_pcm_hw_params(handle, params);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set hw params for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		return 0;
	}
	
	int AlsaOut::set_swparams(snd_pcm_sw_params_t *swparams) {
		int err;
		// get the current swparams
		err = snd_pcm_sw_params_current(handle, swparams);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to determine current swparams for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// start the transfer when the buffer is almost full:
		// (buffer_size / avail_min) * avail_min
		err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set start threshold mode for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// allow the transfer when at least period_size samples can be processed
		err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set avail min for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// align all transfers to 1 sample
		err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set transfer align for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		// write the parameters to the playback device
		err = snd_pcm_sw_params(handle, swparams);
		if (err < 0) {
			std::cerr << "psycle: alsa: unable to set sw params for playback: " << snd_strerror(err) << '\n';
			return err;
		}
		return 0;
	}
	
	///   Underrun and suspend recovery
	int AlsaOut::xrun_recovery(int err) {
		if (err == -EPIPE) { // under-run
			err = snd_pcm_prepare(handle);
			if(err < 0) std::cerr << "psycle: alsa: cannot recover from under-run, prepare failed: " << snd_strerror(err) << '\n';
			return 0;
		} else if(err == -ESTRPIPE) {
			while((err = snd_pcm_resume(handle)) == -EAGAIN)
				sleep(1); /// wait until the suspend flag is released
			if(err < 0) {
				err = snd_pcm_prepare(handle);
				if(err < 0) std::cerr << "psycle: alsa: cannot recover from suspend, prepare failed: " << snd_strerror(err) << '\n';
			}
			return 0;
		}
		return err;
	}
	
	int AlsaOut::write_loop() {
		signed short *ptr;
		int err, cptr;
		while(true) {
			int enablePlayerCopy;
			{
				std::scoped_lock<std::mutex> lock(mutex_);
				enablePlayerCopy = enablePlayer;
			}
			if(enablePlayerCopy > 0) {
				{
					std::scoped_lock<std::mutex> lock(mutex_);
					enablePlayer = 2; condition_.notify_one();
				}
				FillBuffer(0, period_size);
				ptr = samples;
				cptr = period_size;
				while(cptr > 0) {
					err = snd_pcm_writei(handle, ptr, cptr);
					if(err == -EAGAIN) continue;
					if(err < 0) {
						if (xrun_recovery(err) < 0) {
							std::cerr << "psycle: alsa: write error: " << snd_strerror(err) << '\n';
							return -1;
						}
						break; // skip one period
					}
					ptr += err * channels;
					cptr -= err;
				}
			} else if(enablePlayerCopy == -2) return 0;
			else {
				{
					std::scoped_lock<std::mutex> lock(mutex_);
					enablePlayer = -1; condition_.notify_one();
				}
				return 0;
			}
			std::this_thread::yield(); ///\todo sleeping is bad
		}
	}

	#if 0 ///\todo remove? This appears to not be used  
		struct transfer_method {
			const char *name;
			snd_pcm_access_t access;
			int (*transfer_loop)(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
		};
	#endif

	void AlsaOut::thread_function() {
		int err = write_loop();
		if(err < 0) std::cerr << "psycle: alsa: transfer failed: " << snd_strerror(err) << '\n';
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			enablePlayer = -3; condition_.notify_one();
		}
	}
	
}}
#endif // defined PSYCLE__ALSA_AVAILABLE

