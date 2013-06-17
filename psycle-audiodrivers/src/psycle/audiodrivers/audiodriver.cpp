// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "audiodriver.h"
#include <psycle/helpers/math/clip.hpp>
#include <boost/bind.hpp>

namespace psycle { namespace audiodrivers {

namespace loggers = universalis::os::loggers;
using namespace helpers::math;

/// number of samples that the dummy driver reads at a time
int const DUMMYDRIVERWORKFN_MAX_BUFFER_LENGTH = 8192;

/*******************************************************************************************/
// AudioDriverInfo

AudioDriverInfo::AudioDriverInfo(std::string const & name, std::string const & header, std::string const & description, bool show) 
: name_(name), header_(header), description_(description), show_(show)
{}

/*******************************************************************************************/
// AudioDriverSettings

AudioDriverSettings::AudioDriverSettings()
:
	deviceName_(),
	samplesPerSec_(44100),
	bitDepth_(16),
	channelMode_(3),
	blockFrames_(2048),
	blockCount_(4)
{
	frameBytes_ = channelMode_ == 3 ? bitDepth_ / 4 : bitDepth_ / 8;
	blockBytes_ = blockFrames_ * frameBytes_;
	totalBufferBytes_ = blockBytes_ * blockCount_;
	assert(blockFrames_ < MAX_SAMPLES_WORKFN);
}

bool AudioDriverSettings::operator!=(AudioDriverSettings const & other) {
	return
		deviceName_ != other.deviceName_ ||
		samplesPerSec_ != other.samplesPerSec_ ||
		bitDepth_ != other.bitDepth_ ||
		channelMode_ != other.channelMode_ ||
		blockFrames_ != other.blockFrames_ ||
		blockCount_ != other.blockCount_;
}

/*******************************************************************************************/
// AudioDriver

AudioDriver::AudioDriver()
:
	callback_(),
	callback_context_(),
	opened_(),
	started_()
{}

void AudioDriver::before_destruction() throw() {
	try {
		set_opened(false);
	} catch(std::exception e) {
		std::ostringstream s;
		s << "could not close driver: " << e.what();
		loggers::exception()(s.str());
	} catch(...) {
		std::ostringstream s;
		s << "could not close driver: " << universalis::compiler::exceptions::ellipsis_desc();
		loggers::exception()(s.str());
	}
}

void AudioDriver::set_callback(AUDIODRIVERWORKFN callback, void * context) {
	callback_ = callback;
	callback_context_ = context;
}

void AudioDriver::set_opened(bool b) throw(std::exception) {
	if(b) {
		if(!opened()) do_open();
	} else if(opened()) {
		set_started(false);
		do_close();
	}
	opened_ = b;
}

void AudioDriver::set_started(bool b) throw(std::exception) {
	if(b) {
		if(!started()) {
			set_opened(true);
			do_start();
		}
	} else if(started()) do_stop();
	started_ = b;
}

void AudioDriver::setPlaybackSettings(AudioDriverSettings const & settings ) {
	if(!opened_) playbackSettings_ = settings;
	else if(playbackSettings_ != settings) {
		do_close();
		playbackSettings_ = settings;
		do_open();
		if(started_) do_start();
	}
}

void AudioDriver::setCaptureSettings(AudioDriverSettings const & settings ) {
	if(!opened_) captureSettings_ = settings;
	else if(captureSettings_ != settings) {
		do_close();
		captureSettings_ = settings;
		do_open();
		if(started_) do_start();
	}
}

double AudioDriver::frand() {
	static int32_t stat = 0x16BA2118;
	stat = (stat * 1103515245 + 12345) & 0x7fffffff;
	return static_cast<double>(stat) * (1.0 / 0x7fffffff);
}

///\TODO all these methods assume a stereo signal. It's alright for now, but...
void AudioDriver::Quantize16WithDither(float const * pin, int16_t * piout, int c) {
	do {
		*piout++ = rint_clip<int16_t>(pin[0] + frand());
		*piout++ = rint_clip<int16_t>(pin[1] + frand());
		pin += 2;
	} while(--c);
}

void AudioDriver::Quantize16(float const * pin, int16_t * piout, int c) {
	rint_clip_array(pin, piout, c * 2);
}

void AudioDriver::Quantize16AndDeinterlace(float const * pin, int16_t * poleft, int strideleft, int16_t * poright, int strideright, int c) {
	do {
		*poleft = rint_clip<int16_t>(pin[0]);
		*poright = rint_clip<int16_t>(pin[1]);
		poleft += strideleft;
		poright += strideright;
		pin += 2;
	} while(--c);
}

void AudioDriver::DeQuantize16AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c) {
	do {
		*poutleft++ = static_cast<int16_t>(*pin & 0xFFFF);
		*poutright++ = static_cast<int16_t>((*pin & 0xFFFF0000) >> 16);
		++pin;
	} while(--c);
}

void AudioDriver::Quantize24WithDither(float const * pin, int32_t * piout, int c) {
	do {
		*piout++ = rint_clip<int32_t, 24>(pin[0] + frand());
		*piout++ = rint_clip<int32_t, 24>(pin[1] + frand());
		pin += 2;
	} while(--c);
}

void AudioDriver::Quantize24(float const * pin, int32_t * piout, int c) {
	do {
		*piout++ = rint_clip<int32_t, 24>(pin[0]);
		*piout++ = rint_clip<int32_t, 24>(pin[1]);
		pin += 2;
	} while(--c);
}

///\TODO not verified. copied from ASIO implementation
void AudioDriver::Quantize24AndDeinterlace(float const * pin, int32_t * pileft, int32_t * piright, int c) {
	char* outl = (char*)pileft;
	char* outr = (char*)piright;
	// TODO packed 24-bit integers ... ouch! Does anyone use this?
	union u {
		int32_t i;
		char c[3];
	} t;
	do {
		t.i = rint_clip<int32_t, 24>((*pin++) * 256.0f);
		UNIVERSALIS__COMPILER__WARNING("big-endianess assumed ?")
		*outl++ = t.c[0];
		*outl++ = t.c[1];
		*outl++ = t.c[2];

		t.i = rint_clip<int32_t, 24>((*pin++) * 256.0f);
		UNIVERSALIS__COMPILER__WARNING("big-endianess assumed ?")
		*outr++ = t.c[0];
		*outr++ = t.c[1];
		*outr++ = t.c[2];
	} while(--c);
}

///\TODO not verified. copied from ASIO implementation
void AudioDriver::DeQuantize24AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c) {
	char* inl;
	char* inr;
	inl = (char*)poutleft;
	inr = (char*)poutright;
	// TODO packed 24-bit integers ... ouch! Does anyone use this?
	union u {
		int32_t i;
		char c[3];
	} t;
	do {
		UNIVERSALIS__COMPILER__WARNING("big-endianess assumed ?")
		t.c[0] = *inl++;
		t.c[1] = *inl++;
		t.c[2] = *inl++;
		*(poutleft++) = t.i >> 8;

		UNIVERSALIS__COMPILER__WARNING("big-endianess assumed ?")
		t.c[0] = *inr++;
		t.c[1] = *inr++;
		t.c[2] = *inr++;
		*(poutright++) = t.i >> 8;

	} while(--c);
}

/*******************************************************************************************/
// DummyDriver

AudioDriverInfo DummyDriver::info() const {
	return AudioDriverInfo("dummy", "Dummy Driver", "Dummy silent driver", true);
}

DummyDriver::~DummyDriver() throw() {
	before_destruction();
}
	
void DummyDriver::do_start() throw(std::exception) {
	stop_requested_ = false;
	thread_ = new thread(boost::bind(&DummyDriver::thread_function, this));
}

void DummyDriver::thread_function() {
	while(true) {
		{ scoped_lock lock(mutex_);
			// check whether the thread has been asked to terminate
			if(stop_requested_) break;
		}
		callback(DUMMYDRIVERWORKFN_MAX_BUFFER_LENGTH);
	}
}

void DummyDriver::do_stop() throw(std::exception) {
	{ scoped_lock lock(mutex_);
		stop_requested_ = true; // ask the thread to terminate
	}
	thread_->join();
	delete thread_;
}

}}
