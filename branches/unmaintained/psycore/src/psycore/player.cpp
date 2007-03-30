/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "player.h"
#include <iostream>
#include <cmath>

namespace psy {
  namespace core {

    Player::Player() 
      : outputPlugin_(0)
    {
    }

    Player::~Player()
    {
    }

    Player* Player::clone() const {
      return new Player(*this);
    }


    ChannelData& Player::input() {
      return input_;
    }

    ChannelData& Player::output() {
      return output_;
    }

    bool Player::loadAudioOutPlugin( const std::string& path ) {
      if ( !pluginLoad.open( path ) ) {
	goto handle_error;
      }
      else {
	void* (*gpi) (void);
	typedef void* (*gpi_t)()
		#if defined __MSC_VER
			__cdecl
		#elif defined __GNUG__
			__attribute__((cdecl))
		#endif
		;               
	gpi = reinterpret_cast<gpi_t>(pluginLoad.loadProcAdress("getPsyAudioOutPlugin"));
	if ( gpi == NULL) {
	  goto handle_error;
	}
	outputPlugin_ = reinterpret_cast<PsyAudioOut*>(gpi());
	if ( outputPlugin_ == NULL) {
	  goto handle_error;
	}

	std::cout << "Outputplugin is : " << outputPlugin_->name << std::endl;
	outputPlugin_->setCallback( Player::process, this );
	std::cout << outputPlugin_->settings()->samplesPerSec << std::endl;
	std::cout << outputPlugin_->settings()->channelSize << std::endl;
	if ( !outputPlugin_->open() ) {
	  goto handle_error;
	}
      
	std::cout << "device started" << std::endl;
	std::cout << "device has " << outputPlugin_->channelSize() << " channels" << std::endl;
      
	output_.addNewChannel( outputPlugin_->channelSize() );
	std::list< Channel >::iterator it =  output_.begin();
	for ( int i = 0; it != output_.end(); it++, i++ ) {
	  Channel& channel = *it;
	  channel.setBuffer( outputPlugin_->buffer(i), 256 );
	}
	return true;
      }

    handle_error:
      output_.removeAllChannels();
      outputPlugin_ = NULL;
      if (pluginLoad.isopen()) {
	pluginLoad.close();
      }
      return false;
    }

    int Player::process( unsigned int nframes, void *arg ) {
      Player* player = reinterpret_cast<Player*>(arg);
      return player->process( nframes );
    }

    int Player::process( unsigned int nframes ) {
      // produce a sin test wave

      std::list< Channel >::iterator it =  output_.begin();
      for ( ; it != output_.end(); it++ ) {
        Channel& channel = *it;
        for ( unsigned int frame = 0; frame < nframes; frame++ ) {
          channel.buffer()[frame] = sin( 2* 3.14/ 180* frame );
        }
      }          
      
      return 0;
    }

    // starts to play.
    void Player::start( double pos ) {
    }

    // stops playing.
    void Player::stop() {
    }

    // is the player in playmode.
    bool Player::playing() const {
      return true;
    }

    // set the actualPlaypos
    void Player::setPlayPos( double pos ) {
    }

    // the current playPos
    double Player::playPos() const {
      return 0;
    }

  }
}
