// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__JACK_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__JACK_OUT__INCLUDED
#pragma once

#if defined PSYCLE__JACK_AVAILABLE
#include "audiodriver.h"
#include <jack/jack.h>
#include <string>

namespace psycle { namespace audiodrivers {

class JackOut : public AudioDriver {
	public:
		JackOut();
		~JackOut();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() {}
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close() {}

	private:
		bool running_;

		// jack variables
		jack_port_t *output_port_1;
		jack_port_t *output_port_2;

		jack_client_t *client;
		const char **ports;

		std::string clientName_;
		std::string serverName_;

		static int process (jack_nframes_t nframes, void *arg);
		int fillBuffer( jack_nframes_t nframes );
};

}}
#endif // defined PSYCLE__JACK_AVAILABLE
#endif
