// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__ESOUND_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__ESOUND_OUT__INCLUDED
#pragma once

#if defined PSYCLE__ESOUND_AVAILABLE
#include "audiodriver.h"
#include <pthread.h>
#include <exception>

namespace psycle { namespace audiodrivers {

class ESoundOut : public AudioDriver {
	public:
		ESoundOut();
		~ESoundOut();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open();
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close();

	private:
		unsigned int channels_;
		int channelsFlag();

		unsigned int bits_;
		int bitsFlag();

		unsigned int rate_;

		std::string host_;
		int port_;
		std::string hostPort();
		int output_;
		int fd_;

		pthread_t threadId_;
		static void audioOutThreadStatic(void*);
		void audioOutThread();
		bool threadRunning_;
		bool killThread_;

		int writeBuffer(char * buffer, long size);
		long deviceBuffer_;
};

}}
#endif
#endif
