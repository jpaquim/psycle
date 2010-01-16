// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__WAVE_FILE_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__WAVE_FILE_OUT__INCLUDED
#pragma once

#include "audiodriver.h"
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

class WaveFileOut : public AudioDriver {
	public:
		/*override*/ AudioDriverInfo info() const;
		WaveFileOut();
		~WaveFileOut() throw();

	protected:
		/*override*/ void do_open() {}
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close() {}

	private:
		void thread_function();
		void writeBuffer();
		thread * thread_;
		typedef class scoped_lock<mutex> scoped_lock;
		mutex mutable mutex_;
		bool stop_requested_;
};

}}
#endif
