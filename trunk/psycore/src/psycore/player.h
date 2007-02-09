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
#ifndef PLAYER_H
#define PLAYER_H

#include "machine.h"
#include "psyaudio.h"
#include "pluginload.h"
#include <string>

namespace psy {
  namespace core {


    class Player : public Machine {
    public:

      Player();
      ~Player();

      virtual Player* clone() const;

      virtual ChannelData& input();
      virtual ChannelData& output();

      // starts to play.
      void start( double pos = 0.0 );

      // stops playing.
      void stop();

      // is the player in playmode.
      bool playing() const;

      // set the actualPlaypos
      void setPlayPos( double pos );

      // the current playPos
      double playPos() const;

      // returns false if loading fails
      bool loadAudioOutPlugin( const std::string& path );

    private:

      PsyAudioOut* outputPlugin_;
      PluginLoad pluginLoad;

      ChannelData input_;
      ChannelData output_;

      static int process ( unsigned int nframes, void *arg);
      int process ( unsigned int nframes );

    };

  }
}

#endif
