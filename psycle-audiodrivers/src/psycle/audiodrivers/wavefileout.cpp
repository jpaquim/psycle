// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "wavefileout.h"
#include <boost/bind.hpp>
#include <iostream>

namespace psycle { namespace audiodrivers {

using namespace psycle::helpers;

AudioDriverInfo WaveFileOut::info( ) const {
	return AudioDriverInfo("wavefileout", "Wave to File Driver", "Recording a wav to a file", false);
}

WaveFileOut::WaveFileOut()
	: dither_enabled_(true) {
}

void WaveFileOut::do_start() throw(std::exception) {
	stop_requested_ = false;
	thread_ = new thread(boost::bind(&WaveFileOut::thread_function, this));
}

void WaveFileOut::thread_function() {
	while(true) {
		scoped_lock lock(mutex_);
		if(stop_requested_)
			return;
		Write(callback(256),256);
	}
}

void WaveFileOut::Write(float* data, unsigned int frames)
{
	for (unsigned i = 0; i < frames; ++i) {
		left[i] = *data++;
		right[i] = *data++;
	}

	if(dither_enabled_) {
		dither_.Process(left, frames);
		dither_.Process(right, frames);
	}
	switch(playbackSettings().channelMode()) {
		case 0: // mono mix
			for(int i(0); i < frames; ++i)
				//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
				if(wav_file_.WriteMonoSample((left[i] + right[i]) / 2) != DDC_SUCCESS) {
					set_opened(false);
				}
			break;
		case 1: // mono L
			for(int i(0); i < frames; ++i)
				if(wav_file_.WriteMonoSample(left[i]) != DDC_SUCCESS) {
					set_opened(false);
				}
			break;
		case 2: // mono R
			for(int i(0); i < frames; ++i)
				if(wav_file_.WriteMonoSample(right[i]) != DDC_SUCCESS) {
					set_opened(false);
				}
			break;
		default: // stereo
			for(int i(0); i < frames; ++i)
				if(wav_file_.WriteStereoSample(left[i], right[i]) != DDC_SUCCESS) {
					set_opened(false);
				}
			break;
	}
}

void WaveFileOut::do_open() throw(std::exception)
{
	int channel_num ;
	if (playbackSettings().channelMode() < 3)
		channel_num = 1;
	else
		channel_num = 2;
	wav_file_.OpenForWrite(
		playbackSettings().deviceName().c_str(),
		playbackSettings().samplesPerSec(),
		playbackSettings().bitDepth(),
		channel_num);
}

void WaveFileOut::do_close() throw(std::exception)
{
	wav_file_.Close();
}

void WaveFileOut::do_stop() throw(std::exception) {
	scoped_lock lock(mutex_);
	stop_requested_ = true;
	thread_->join();
	delete thread_;
}

WaveFileOut::~WaveFileOut() throw() {
	before_destruction();
}

}}
