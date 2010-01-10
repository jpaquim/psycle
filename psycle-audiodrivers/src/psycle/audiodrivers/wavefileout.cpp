// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "wavefileout.h"
#include <boost/bind.hpp>
#include <iostream>

namespace psycle { namespace audiodrivers {

AudioDriverInfo WaveFileOut::info( ) const {
	return AudioDriverInfo("wavefileout", "Wave to File Driver", "Recording a wav to a file", false);
}

WaveFileOut::WaveFileOut()
:
	thread_(),
	stop_requested_()
{}

void WaveFileOut::do_start() {
	thread_ = new std::thread(boost::bind(&WaveFileOut::thread_function, this));
}

void WaveFileOut::thread_function() {
	while(true) {
		{ scoped_lock lock(mutex_);
			if(stop_requested_) return;
		}
		float const * input(callback(MAX_SAMPLES_WORKFN));
		///\todo well, the real job, i.e. output to a file
	}
}

void WaveFileOut::do_stop() {
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	thread_->join();
	delete thread_;
}

WaveFileOut::~WaveFileOut() {
	set_opened(false);
}

}}
