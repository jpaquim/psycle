// copyright 2006-2007 psycledelics http://psycle.sourceforge.net
/*************************************************************************
*  This program is covered by the GNU General Public License:            *
*  http://gnu.org/licenses/gpl.html                                      *
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
*  This program is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
*  See the GNU General Public License for more details.                  *
*                                                                        *
*  You should have received a copy of the GNU General Public License     *
*  along with this program; if not, write to the                         *
*  Free Software Foundation, Inc.,                                       *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
**************************************************************************/
#if defined PSYCLE__JACK_AVAILABLE
#include "jack_out.h"
#include <iostream>

namespace psy {
	namespace core {

JackOut::JackOut() :
	_initialized( false )
{
	clientName_ = "psycle";
	serverName_ = ""; // maybe not needed
	running_ = 0;
}

JackOut::~JackOut() {

}

AudioDriverInfo JackOut::info( ) const
{
	return AudioDriverInfo("jack","Jack Audio Connection Kit Driver","Low Latency audio driver",true);
}

JackOut * JackOut::clone( ) const
{
	return new JackOut(*this);
}

void JackOut::Initialize( AUDIODRIVERWORKFN pCallback, void * context )
{
	_pCallback = pCallback;
	_callbackContext = context;
	_initialized = true;
}

void JackOut::configure() {

}

bool JackOut::Initialized( )
{
	return _initialized;
}

bool	JackOut::Enable( bool e )
{
	if ( e && !running_ ) {
		running_ = registerToJackServer();
	} else 
	if ( running_ ) {
		jack_client_close (client);
		running_ = false;
	}
	return running_;
}

// Jack special functions

bool JackOut::registerToJackServer() {
	//jack_options_t options = JackNullOption;
	//jack_status_t status;
	
	// try to become a client of the JACK server
	{
		std::string s(clientName_ + " " + serverName_);
		if ( (client = jack_client_new ( s.c_str() )) == 0) {
			std::cerr << "psycle: jack: jack server not running?\n";
			return 0;
		}
	}

	#if 0
	if ( (client = jack_client_open( clientName_.c_str(),options,&status,serverName_.c_str())) == NULL )
	{
		std::cerr << "psycle: jack: jack server not running?\n";
		return 0;
	}
	#endif

	// tell the JACK server to call `process()' whenever there is work to be done.
	jack_set_process_callback (client, process, (void*) this);

	// display the current sample rate. 
	std::cout << "psycle: jack: engine sample rate: " << jack_get_sample_rate (client) << "\n";

	// create output port
	output_port_1 = jack_port_register (client, "output_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	output_port_2 = jack_port_register (client, "output_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	// tell the JACK server that we are ready to roll
	if (jack_activate (client)) {
		std::cerr << "psycle: jack: cannot activate client\n";
		return 0;
	}

	AudioDriverSettings settings_ = settings();
	settings_.setSamplesPerSec( jack_get_sample_rate (client) );
	settings_.setBitDepth( 16 ); ///\todo hardcoded for now
	setSettings( settings_ );

	if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		std::cerr << "psycle: jack: cannot find any physical playback port\n";
		return 0;
	}

	if (jack_connect (client, jack_port_name (output_port_1), ports[0])) {
		std::cerr << "psycle: jack: cannot connect left output port\n";
		return 0;
	}

	if (jack_connect (client, jack_port_name (output_port_2), ports[1])) {
		std::cerr << "psycle: jack: cannot connect right output port\n";
		return 0;
	}

	std::free (ports);

	std::cout << "psycle: jack: enabled\n";
	return 1;
}

int JackOut::process (jack_nframes_t nframes, void *arg)
{
	JackOut* driver = static_cast<JackOut*> (arg);
	driver->fillBuffer( nframes );
	return 0;      
}

int JackOut::fillBuffer( jack_nframes_t nframes )
{
	jack_default_audio_sample_t *out_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_1, nframes);
	jack_default_audio_sample_t *out_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_2, nframes);

	int nframesint = nframes;
	float const * input(_pCallback(_callbackContext, nframesint));

	int count=0;
	while ( count < nframesint) {
		out_1[ count ] = *input++  / 32768.0f;
		out_2[ count ] = *input++  / 32768.0f;
		count++;
	}
	return 0;
}

	} // namespace audio_drivers
} // namespace psycle

#endif // defined PSYCLE__JACK_AVAILABLE
