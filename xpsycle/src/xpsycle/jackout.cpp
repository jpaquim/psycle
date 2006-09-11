/***************************************************************************
  *   Copyright (C) 2006 by Stefan Nattkemper  *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/

#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/jack_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_JACK
#include "jackout.h"
#include <iostream>

namespace psycle
{
	namespace host
	{

		JackOut::JackOut() :
			_initialized( false )
		{
			clientName_ = "xpsycle";
			serverName_ = ""; // maybe not needed
		}

		JackOut::~JackOut() {

		}

		AudioDriverInfo JackOut::info( ) const
		{
			return AudioDriverInfo("jack");
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
			if ( e ) {
				return registerToJackServer();
			} else {
				return jack_client_close (client);
			}
		}

		// Jack special functions

		bool JackOut::registerToJackServer() {
 			// try to become a client of the JACK server
			const char* registerCPtr = std::string( clientName_ +" "+serverName_  ).c_str();

			if ( (client = jack_client_new ( registerCPtr )) == 0) {
 				std::cerr << "jack server not running?\n" << std::endl;
				return 0;
 			}


			// tell the JACK server to call `process()' whenever
			// there is work to be done.
 
			jack_set_process_callback (client, process, (void*) this);

			 // display the current sample rate. 

			std::cout << "engine sample rate: %"  << jack_get_sample_rate (client) << std::endl;

			setSamplesPerSec( jack_get_sample_rate (client) );
			setBitDepth( 16 ); // hardcoded so far

			// create output port

			output_port_1 = jack_port_register (client, "output_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			output_port_2 = jack_port_register (client, "output_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			std::cout << "jo" << std::endl;

			 // tell the JACK server that we are ready to roll

 			if (jack_activate (client)) {
 				std::cout << "cannot activate client" << std::endl;
				return 0;
 			}

			if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
				std::cout << "Cannot find any physical playback ports" << std::endl;
				return 0;
			}

      std::cout << "jo1" << std::endl;

			if (jack_connect (client, jack_port_name (output_port_1), ports[0])) {
				std::cout << "cannot connect output ports" << std::endl;
			}

			std::cout << "jo2" << std::endl;

			if (jack_connect (client, jack_port_name (output_port_2), ports[1])) {
		 		std::cout << "cannot connect output ports" << std::endl;
			}

			std::cout << "jo3" << std::endl;

			free (ports);

			std::cout << "jack enabled" << std::endl;

			return 1;
		}

		int JackOut::process (jack_nframes_t nframes, void *arg)
 		{
			JackOut* driver = static_cast<JackOut*> (arg);
			driver->fillBuffer( nframes );
			return 0;      
 		}

		int JackOut::fillBuffer( jack_nframes_t nframes ) {
			jack_default_audio_sample_t *out_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_1, nframes);

			jack_default_audio_sample_t *out_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_2, nframes);
 				
			jack_default_audio_sample_t out1_buf[ nframes * sizeof(jack_default_audio_sample_t)];

			jack_default_audio_sample_t out2_buf[ nframes * sizeof(jack_default_audio_sample_t)];

			int count = nframes;
			
 			float const * input(_pCallback(_callbackContext, count));
			
			// convert stero into two buffers;

			// sounds trashy sth wrong here or above
			while ( count--  > 0) {
				out_1[ count ] = *input++  / 32768.0f;
				out_2[ count ] = *input++  / 32768.0f;				
			}

			return 0;
		}

	}
}





#endif // !defined XPSYCLE__NO_JACK
