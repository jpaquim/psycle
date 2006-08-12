/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "esoundout.h"
#include "esd.h"

namespace psycle { namespace host {

ESoundOut::ESoundOut()
 : AudioDriver()
{
  _running     = false;
  _initialized = false;
}


ESoundOut::~ESoundOut()
{
}

void ESoundOut::Initialize( AUDIODRIVERWORKFN pCallback, void * context )
{
  _pCallback = pCallback;
  _callbackContext = context;
  _initialized = true;
  _running = false;
  setDefault();
  open_output();
}

bool ESoundOut::Initialized( )
{
  return _running;
}

void ESoundOut::configure( )
{
}

bool ESoundOut::Enable( bool e )
{
}

int ESoundOut::get_bit_flag( int bits )
{
  switch(bits)
  {
    case 8:
      return ESD_BITS8;
    break;

    case 16:
      return ESD_BITS16;
    break;

   case 24:
      return ESD_BITS16;
   break;
  }
  return 0;
}

// No more than 2 channels in ESD
int ESoundOut::get_channels_flag(int channels)
{
  switch(channels)
  {
    case 1:
      return ESD_MONO;
    break;

    case 2:
      return ESD_STEREO;
    break;

    default:
      return ESD_STEREO;
    break;
  }
  return 0;
}

std::string ESoundOut::translate_device_string(const std::string & server, int port)
{
  char device_string[8000];
// ESD server
  if(port > 0 && strlen(server.c_str()))
    sprintf(device_string, "%s:%d", server.c_str(), port);
  else
    sprintf(device_string, "");
  return std::string(device_string);
}

int ESoundOut::open_output()
{
  esd_format_t format = ESD_STREAM | ESD_PLAY;

  format |= get_channels_flag(channels);
  format |= get_bit_flag(bits);

  if((esd_out = esd_open_sound(translate_device_string(esound_out_server, esound_out_port).c_str())) <= 0)
  {
    fprintf(stderr, "AudioESound::open_output: open failed\n");
    return 1;
  };
  esd_out_fd = esd_play_stream_fallback(format,
      rate,
      translate_device_string(esound_out_server.c_str(),esound_out_port).c_str(),
      "Bcast 2000");
  device_buffer = esd_get_latency(esd_out);
  device_buffer *= bits / 8 * channels;
  return 0;
}

void ESoundOut::setDefault( )
{
  channels = 2;
  bits = 16;
  rate = 44100;
}


int ESoundOut::read_buffer(char *buffer, long size)
{
	if(esd_in_fd > 0)
		return read(esd_in_fd, buffer, size);
	else
		return 1;
return 0;
}

int ESoundOut::write_buffer(char *buffer, long size)
{
	if(esd_out_fd > 0)
		return write(esd_out_fd, buffer, size);
	else
		return 0;
return 0;
}

}}
