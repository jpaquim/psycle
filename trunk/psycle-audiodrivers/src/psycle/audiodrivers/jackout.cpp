// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#if defined PSYCLE__JACK_AVAILABLE
#include "jackout.h"
#include <iostream>

namespace psycle { namespace audiodrivers {

AudioDriverInfo JackOut::info( ) const {
	return AudioDriverInfo("jack","Jack Audio Connection Kit Driver","Low Latency audio driver",true);
}

JackOut::JackOut() {
	clientName_ = "psycle";
	serverName_ = ""; // maybe not needed
	running_ = 0;
}

void JackOut::do_start() {
	//jack_options_t options = JackNullOption;
	//jack_status_t status;
	// try to become a client of the JACK server
	const char* registerCPtr = std::string( clientName_ +" "+serverName_  ).c_str();

	if ( (client = jack_client_new ( registerCPtr )) == 0) {
		std::cerr << "jack server not running?\n" << std::endl;
		return;
	}

	/*if ( (client = jack_client_open( clientName_.c_str(),options,&status,serverName_.c_str())) == NULL ) {
		std::cerr << "jack server not running?\n" << std::endl;
	}*/

	// tell the JACK server to call `process()' whenever
	// there is work to be done.

	jack_set_process_callback (client, process, (void*) this);

	// display the current sample rate. 

	std::cout << "engine sample rate: "  << jack_get_sample_rate (client) << std::endl;

	// create output port

	output_port_1 = jack_port_register (client, "output_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	output_port_2 = jack_port_register (client, "output_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	std::cout << "jo" << std::endl;

		// tell the JACK server that we are ready to roll

		if (jack_activate (client)) {
			std::cerr << "cannot activate client" << std::endl;
			return;
		}

	playbackSettings_.setSamplesPerSec(jack_get_sample_rate(client));
	playbackSettings_.setBitDepth(16); // hardcoded so far
	///\todo inform the player that the sample rate is different

	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		std::cerr << "Cannot find any physical playback ports" << std::endl;
		return;
	}

	std::cout << "jo1" << std::endl;

	if (jack_connect (client, jack_port_name (output_port_1), ports[0])) {
		std::cerr << "cannot connect output ports" << std::endl;
	}

	std::cout << "jo2" << std::endl;

	if (jack_connect (client, jack_port_name (output_port_2), ports[1])) {
		std::cerr << "cannot connect output ports" << std::endl;
	}

	std::cout << "jo3" << std::endl;

	std::free (ports);

	std::cout << "jack enabled" << std::endl;
}

int JackOut::process (jack_nframes_t nframes, void *arg) {
	JackOut* driver = static_cast<JackOut*> (arg);
	driver->fillBuffer( nframes );
	return 0;      
}

int JackOut::fillBuffer( jack_nframes_t nframes ) {
	jack_default_audio_sample_t *out_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_1, nframes);
	jack_default_audio_sample_t *out_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_2, nframes);
	float const * input(callback(nframes));
	int count = 0;
	while(count < nframes) {
		out_1[count] = *input++  / 32768.0f;
		out_2[count] = *input++  / 32768.0f;
		++count;
	}
	return 0;
}

void JackOut::do_stop() {
	jack_client_close(client);
}

JackOut::~JackOut() {
	set_opened(false);
}

}}
#endif // defined PSYCLE__JACK_AVAILABLE
