
/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#include "audiodriver.h"
#include <diversalis/cpu.hpp>
#include <boost/bind.hpp>
#include <universalis/stdlib/thread.hpp>
#include <psycle/helpers/math/clip.hpp>
#include <psycle/helpers/math/rint.hpp>
namespace psycle { namespace core {

/*******************************************************************************************/
// AudioDriverInfo

AudioDriverInfo::AudioDriverInfo(std::string const & name, std::string const & header, std::string const & description, bool show) 
: name_(name), header_(header), description_(description), show_(show)
{}

/*******************************************************************************************/
// AudioDriverSettings

AudioDriverSettings::AudioDriverSettings()
:
	deviceName_("unnamed"),
	samplesPerSec_(44100),
	bitDepth_(16),
	channelMode_(3),
	bufferSize_(131072),
	blockSize_(4096),
	blockCount_(8)
{}


/*******************************************************************************************/
// AudioDriver

AudioDriverInfo AudioDriver::info() const {
	return AudioDriverInfo("silent", "null output driver", "no sound output", true);
}

double AudioDriver::frand() {
	static int32_t stat = 0x16BA2118;
	stat = (stat * 1103515245 + 12345) & 0x7fffffff;
	return static_cast<double>(stat) * (1.0 / 0x7fffffff);
}

using helpers::math::clipped_lrint;

///\todo: all these methods assume a stereo signal. It's allright for now, but...
void AudioDriver::Quantize16WithDither(float const * pin, int16_t * piout, int c) {
	do {
		*piout++ = clipped_lrint<int16_t>(pin[0] + frand());
		*piout++ = clipped_lrint<int16_t>(pin[1] + frand());
		pin += 2;
	} while(--c);
}

void AudioDriver::Quantize16(float const * pin, int16_t * piout, int c) {
	do {
		*piout++ = clipped_lrint<int16_t>(pin[0]);
		*piout++ = clipped_lrint<int16_t>(pin[1]);
		pin += 2;
	} while(--c);
}

void AudioDriver::Quantize16AndDeinterlace(float const * pin, int16_t * pileft, int strideleft, int16_t * piright, int strideright, int c) {
	do {
		*pileft = clipped_lrint<int16_t>(pin[0]);
		*piright = clipped_lrint<int16_t>(pin[1]);
		pileft += strideleft;
		piright += strideright;
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
		*piout++ = clipped_lrint<int32_t, 24>(pin[0] + frand());
		*piout++ = clipped_lrint<int32_t, 24>(pin[1] + frand());
		pin += 2;
	} while(--c);
}

void AudioDriver::Quantize24(float const * pin, int32_t * piout, int c) {
	do {
		*piout++ = clipped_lrint<int32_t, 24>(pin[0]);
		*piout++ = clipped_lrint<int32_t, 24>(pin[1]);
		pin += 2;
	} while(--c);
}

///\todo: not verified. copied from ASIO implementation
void AudioDriver::Quantize24AndDeinterlace(float const * pin, int32_t * pileft, int32_t * piright, int c) {
	char* outl = (char*)pileft;
	char* outr = (char*)piright;
	int t;
	char* pt = (char*)&t;
	do {
		t = clipped_lrint<int, 24>((*pin++) * 256.0f);
		*outl++ = pt[0];
		*outl++ = pt[1];
		*outl++ = pt[2];

		t = clipped_lrint<int, 24>((*pin++) * 256.0f);
		*outr++ = pt[0];
		*outr++ = pt[1];
		*outr++ = pt[2];
	} while(--c);
}
///\todo: not verified. copied from ASIO implementation
void AudioDriver::DeQuantize24AndDeinterlace(int const * pin, float * poutleft, float * poutright, int c) {
	char* inl;
	char* inr;
	inl = (char*)poutleft;
	inr = (char*)poutright;
	int t;
	char* pt = (char*)&t;
	do {
		pt[0] = *inl++;
		pt[1] = *inl++;
		pt[2] = *inl++;
		*(poutleft++) = t >> 8;

		pt[0] = *inr++;
		pt[1] = *inr++;
		pt[2] = *inr++;
		*(poutright++) = t >> 8;

	} while(--c);
}

/*******************************************************************************************/
// DummyDriver

DummyDriver::DummyDriver()
:
	AudioDriver(),
	callback_function_(),
	callback_context_(),
	initialized_(),
	running_(),
	stop_requested_()
{}
	
DummyDriver::~DummyDriver() {
	stop();
}

AudioDriverInfo DummyDriver::info() const {
	return AudioDriverInfo("dummy", "Dummy Driver", "Dummy silent driver", true);
}

void DummyDriver::Initialize(AUDIODRIVERWORKFN callback_function, void * callback_context) {
	callback_function_ = callback_function;
	callback_context_ = callback_context;
	initialized_ = true;
}

void DummyDriver::start() {
	// return immediatly if the thread is already running
	if(running_) return;
	
	thread t(boost::bind(&DummyDriver::thread_function, this));
	// wait for the thread to be running
	{ scoped_lock lock(mutex_);
		while(!running_) condition_.wait(lock);
	}
}

void DummyDriver::thread_function() {
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
		int samples(256);
		callback_function_(callback_context_, samples);
	}

	// notify that the thread is not running anymore
	notify_termination:
		{ scoped_lock lock(mutex_);
			running_ = false;
		}
		condition_.notify_one();
}

void DummyDriver::stop() {
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

}}
