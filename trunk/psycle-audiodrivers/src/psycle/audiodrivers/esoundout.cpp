// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__ESOUND_AVAILABLE
#include "esoundout.h"
#include <esd.h>
#include <universalis/stdlib/cstdint.hpp>
#include <universalis/compiler/location.hpp>
#include <universalis/os/loggers.hpp>
#include <universalis/exception.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cerrno>
#include <cstring>

namespace psycle { namespace audiodrivers {

namespace loggers = universalis::os::loggers;
using universalis::exceptions::runtime_error;

AudioDriverInfo ESoundOut::info( ) const {
	return AudioDriverInfo("esd", "ESound Driver", "Driver with optional network capabilities (high latency)",true);
}

ESoundOut::ESoundOut()
:
	channels_(2),
	bits_(16),
	rate_(44100),
	threadRunning_(false),
	killThread_(false),
	host_(""),
	port_(0),
	output_(-1)
{}

int ESoundOut::bitsFlag() {
	switch(bits_) {
		case 8: return ESD_BITS8; break;
		case 16: return ESD_BITS16; break;
		default: {
			std::ostringstream s;
			s << "psycle: esound: unsupported audio bit depth: " << bits_ << " (must be 8 or 16)";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
}

int ESoundOut::channelsFlag() {
	switch(channels_) {
		case 1: return ESD_MONO; break;
		case 2: return ESD_STEREO; break;
		default: {
			std::ostringstream s;
			s << "psycle: esound: unsupported audio channel count: " << channels_ << " (must be 1 or 2)";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
}

/// ESD host:port
std::string ESoundOut::hostPort() {
	std::string nrv;
	{
		char * env(std::getenv("ESPEAKER"));
		if(env) {
			nrv = env;
			return nrv;
		}
	}
	if(port_ > 0 && host_.length()) {
		std::ostringstream s;
		s << host_ << ":" << port_;
		nrv = s.str();
	}
	return nrv;
}

void ESoundOut::do_open() {
	esd_format_t format = ESD_STREAM | ESD_PLAY;
	format |= channelsFlag();
	format |= bitsFlag();
	if((output_ = esd_open_sound(hostPort().c_str())) < 0) {
		std::ostringstream s;
		s << "psycle: esound: failed to open output '" << hostPort() << "': " << std::strerror(errno);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	esd_resume(output_);
	esd_print_all_info(esd_get_all_info(output_));
	deviceBuffer_ = esd_get_latency(output_);
	//deviceBuffer_ *= bits_ / 8 * channels_;
	if((fd_ = esd_play_stream_fallback(format, rate_, hostPort().c_str(), "psycle")) < 0) {
		std::ostringstream s;
		s << "psycle: esound: failed to open output play stream '" << hostPort() << "': " << std::strerror(errno);
		throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void ESoundOut::do_start() {
	killThread_ = false;
	pthread_create(&threadId_, NULL, (void*(*)(void*))audioOutThreadStatic, (void*) this);
}

void ESoundOut::audioOutThreadStatic( void * ptr ) {
	reinterpret_cast<ESoundOut*>(ptr)->audioOutThread();
}

void ESoundOut::audioOutThread() {
	threadRunning_ = true;

	int deviceBufferSamples(deviceBuffer_ * (3 - channels_) * 44100 / rate_);

	if(loggers::trace()) {
		std::ostringstream s;
		s << "psycle: esound: device buffer: " << deviceBuffer_ << ", samples: " << deviceBufferSamples;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	switch(bits_) {
		case 16: {
			int bufSize = deviceBuffer_ / 2;
			int16_t buf[bufSize];
			int newCount = bufSize / channels_;
			while(!killThread_) {
				float const * input(callback(newCount));
				#if 0
					for (int i = 0; i < bufSize; i++) {
						buf[i] = *input++;
					}
				#else
					Quantize16(input,buf,newCount);
				#endif
				if(write(fd_, buf, sizeof buf) < 0) std::cerr << "psycle: esound: write failed.\n";
			}
		}
		break;
		case 8: {
			uint8_t buf[deviceBufferSamples];
			int bytes(sizeof buf);
			int samples(bytes);
			while(!killThread_) {
				float const * input(callback(samples));
				for (int i(0); i < samples; ++i) buf[i] = *input++ / 256 + 128;
				if(write(fd_, buf, bytes) < 0) std::cout << "psycle: esound: write failed.\n";
			}
		}
		break;
		default: {
			std::ostringstream s;
			s << "psycle: esound: unsupported audio bit depth: " << bits_ << " (must be 8 or 16)";
			throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	threadRunning_ = false;
	pthread_exit(0);
}

void ESoundOut::do_stop() {
	killThread_ = true;
	while(threadRunning_) usleep(500); ///\todo use proper synchronisation
}

void ESoundOut::do_close() {
	int i = esd_close(output_);
	output_ = -1;
}

ESoundOut::~ESoundOut() throw() {
	before_destruction();
}

}}
#endif // defined PSYCLE__ESOUND_AVAILABLE
