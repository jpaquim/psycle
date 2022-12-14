// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__ALSA_AVAILABLE

#include "alsaout.h"
#include <boost/bind.hpp>
#include <sstream>
#include <cstring>

namespace psycle { namespace audiodrivers {

namespace loggers = universalis::os::loggers;
using universalis::exceptions::runtime_error;

AudioDriverInfo AlsaOut::info() const {
	return AudioDriverInfo("alsa", "Alsa Driver", "Low Latency audio driver", true);
}

AlsaOut::AlsaOut()
:
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
	running_(),
	stop_requested_()
{
	std::memset(&format, 0, sizeof format);

	rate = 44100; // stream rate
	format = SND_PCM_FORMAT_S16; // sample format
	channels = 2; // count of channels

	// safe default values
	buffer_time = 200000;
	period_time = 60000;

	buffer_size = 0;
	period_size = 0;
	output = 0;
}
	
void AlsaOut::do_start() throw(std::exception) {
	// return immediatly if the thread is already running
	if(running_) return;
	
	int err;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	
	err = snd_output_stdio_attach(&output, stdout, 0);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: attaching output failed: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	std::string device_name = playbackSettings().deviceName();
	if(!device_name.length()) device_name = "default";

	std::cout << "psycle: alsa: playback device is: " << device_name << '\n';
	std::cout << "psycle: alsa: stream parameters are: " << rate << "Hz, " << snd_pcm_format_name(format) << ", " << channels << " channels\n";
	std::cout << "psycle: alsa: using transfer method: " << "write" << '\n'; ///\todo parametrable?
	
	if((err = snd_pcm_open(&handle, device_name.c_str() , SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		std::ostringstream s;
		s << "psycle: alsa: playback open error: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	set_hwparams(hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	set_swparams(swparams);
	
	samples = (int16_t*) std::malloc((period_size * channels * snd_pcm_format_width(format)) / 8);
	if(!samples) {
		throw runtime_error("psycle: alsa: out of memory", UNIVERSALIS__COMPILER__LOCATION);
	}
	
	areas = (snd_pcm_channel_area_t*) std::calloc(channels, sizeof(snd_pcm_channel_area_t));
	if(!areas) {
		throw runtime_error("psycle: alsa: out of memory", UNIVERSALIS__COMPILER__LOCATION);
	}
	
	for(unsigned int chn(0); chn < channels; ++chn) {
		areas[chn].addr = samples;
		areas[chn].first = chn * 16;
		areas[chn].step = channels * 16;
	}
	
	thread t(boost::bind(&AlsaOut::thread_function, this));
	// wait for the thread to be running
	{ scoped_lock lock(mutex_);
		while(!running_) condition_.wait(lock);
	}
}

void AlsaOut::thread_function() {
	// notify that the thread is now running
	{ scoped_lock lock(mutex_);
		running_ = true;
	}
	condition_.notify_one();

	while(true) {
		// check whether the thread has been asked to terminate
		{ scoped_lock lock(mutex_);
			if(stop_requested_) goto notify_termination;
		}
		FillBuffer(0, period_size);
		int16_t * ptr = samples;
		int cptr = period_size;
		while(cptr > 0) {
			int err = snd_pcm_writei(handle, ptr, cptr);
			if(err == -EAGAIN) continue;
			if(err < 0) {
				if(xrun_recovery(err) < 0) {
					if(loggers::exception()) {
						std::ostringstream s;
						s << "psycle: alsa: write error: " << snd_strerror(err);
						loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					goto notify_termination;
				}
				break; // skip one period
			}
			ptr += err * channels;
			cptr -= err;
		}
		this_thread::yield(); ///\todo is this useful?
	}

	// notify that the thread is not running anymore
	notify_termination:
		{ scoped_lock lock(mutex_);
			running_ = false;
		}
		condition_.notify_one();
}

/// Underrun and suspend recovery
int AlsaOut::xrun_recovery(int err) {
	if(err == -EPIPE) { // under-run
		err = snd_pcm_prepare(handle);
		if(err < 0) {
			if(loggers::warning()) {
				std::ostringstream s;
				s << "psycle: alsa: cannot recover from under-run, prepare failed: " << snd_strerror(err) << '\n';
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
		return 0;
	} else if(err == -ESTRPIPE) {
		while((err = snd_pcm_resume(handle)) == -EAGAIN)
			// wait until the suspend flag is released
			this_thread::yield(); ///\todo any other way?
		if(err < 0) {
			err = snd_pcm_prepare(handle);
			if(err < 0) {
				if(loggers::warning()) {
					std::ostringstream s;
					s << "psycle: alsa: cannot recover from suspend, prepare failed: " << snd_strerror(err);
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		}
		return 0;
	}
	return err;
}

void AlsaOut::do_stop() throw(std::exception) {
	// return immediatly if the thread is not running
	if(!running_) return;

	// ask the thread to terminate
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_one();
	
	/// join the thread
	{ scoped_lock lock(mutex_);
		while(running_) condition_.wait(lock);
		stop_requested_ = false;
	}
}

void AlsaOut::FillBuffer(snd_pcm_uframes_t offset, int count) {
	int16_t *samples[channels];
	int steps[channels];
	// verify and prepare the contents of areas
	for (unsigned int chn(0); chn < channels; ++chn) {
		if(areas[chn].first % 8) {
			std::ostringstream s;
			s << "psycle: alsa: areas[" << chn << "].first == " << areas[chn].first << ", aborting.";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		samples[chn] = (int16_t *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
		if(areas[chn].step % 16) {
			std::ostringstream s;
			s <<  "psycle: alsa: areas[" << chn << "].step == " << areas[chn].step << ", aborting.";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		steps[chn] = areas[chn].step / 16;
		samples[chn] += offset * steps[chn];
	}
	// fill the channel areas
	float const * input(callback(count));
	
	Quantize16AndDeinterlace(input,samples[0], steps[0], samples[1], steps[1], count);
	samples[0] += steps[0] * count;
	samples[1] += steps[1] * count;
}

void AlsaOut::set_hwparams(snd_pcm_hw_params_t *params, snd_pcm_access_t access) {
	snd_pcm_uframes_t size;
	int err;
	
	// choose all parameters
	err = snd_pcm_hw_params_any(handle, params);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: broken configuration for playback: no configurations available: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	#if 0
		// set hardware resampling
		err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
		if(err < 0) {
			std::ostringstream s;
			s << "psycle: alsa: resampling setup failed for playback: " << snd_strerror(err);
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	#endif
	// set the interleaved read/write format
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: access type not available for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// set the sample format
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: sample format not available for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// set the count of channels
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: channels count (" << channels << ") not available for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	// set the stream rate
	{
		unsigned int rrate(rate);
		err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
		if(err < 0) {
			std::ostringstream s;
			s << "psycle: alsa: rate "<< rate << "Hz not available for playback: " << snd_strerror(err);
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(rrate != rate) {
			///\todo don't throw an exception, but inform the caller that the actual settings differ.
			std::ostringstream s;
			s << "psycle: alsa: rate does not match (requested " << rate << "Hz, got " << err << "Hz)";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
	
	// set the buffer time
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set buffer time " << buffer_time << " for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to get buffer size for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	if(loggers::trace()) {
		std::ostringstream s;
		s << "psycle: alsa: buffer size: " << size;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	buffer_size = size;

	// set the period time
	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set period time " << period_time << " for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	err = snd_pcm_hw_params_get_period_size(params, &size, 0);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to get period size for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	period_size = size;

	// write the parameters to device
	err = snd_pcm_hw_params(handle, params);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set hw params for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void AlsaOut::set_swparams(snd_pcm_sw_params_t *swparams) {
	int err;
	// get the current swparams
	err = snd_pcm_sw_params_current(handle, swparams);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to determine current swparams for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// start the transfer when the buffer is almost full:
	// (buffer_size / avail_min) * avail_min
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set start threshold mode for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// allow the transfer when at least period_size samples can be processed
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set avail min for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// align all transfers to 1 sample
	#if SND_LIB_VERSION >= 0x10010 // 1.0.16
		// snd_pcm_sw_params_set_xfer_align() is deprecated, alignment is always 1
	#else
		err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
		if(err < 0) {
			std::ostringstream s;
			s << "psycle: alsa: unable to set transfer align for playback: " << snd_strerror(err);
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	#endif
	// write the parameters to the playback device
	err = snd_pcm_sw_params(handle, swparams);
	if(err < 0) {
		std::ostringstream s;
		s << "psycle: alsa: unable to set sw params for playback: " << snd_strerror(err);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

AlsaOut::~AlsaOut() throw() {
	before_destruction();
	///\todo free memory here
}

}}
#endif // defined PSYCLE__ALSA_AVAILABLE

