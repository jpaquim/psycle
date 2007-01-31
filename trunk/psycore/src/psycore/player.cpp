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

#ifdef __unix__
#include <dlfcn.h>
#else
#include <windows.h>
#ifdef _MSC_VER
#undef min
#undef max
#endif
#endif

namespace psy {
  namespace core {

    Player::Player() 
      : outputPluginLibHandle_(0)
    {
    }

    Player::~Player()
    {
      unloadOutputPlugin();
    }

    Player* Player::clone() const {
      return new Player(*this);
    }

    void Player::loadAudioOutPlugin( const std::string& path ) {
      unloadOutputPlugin();
#ifdef __unix__
     outputPluginLibHandle_ = dlopen( path.c_str(), RTLD_LAZY);
#else
     // Set error mode to disable system error pop-ups (for LoadLibrary)
      UINT uOldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
     outputPluginLibHandle_ = ::LoadLibrary( path.c_str() );
     // Restore previous error mode
     ::SetErrorMode( uOldErrorMode );
#endif
    }

    void Player::unloadOutputPlugin() {
      #ifdef __unix__
      #else
      if  ( outputPluginLibHandle_ ) {
        ::FreeLibrary( reinterpret_cast<HINSTANCE>(outputPluginLibHandle_) ) ;
      }
      #endif
    }

    int Player::process ( unsigned int nframes, void *arg) {
      return 0;
    }

  }
}
