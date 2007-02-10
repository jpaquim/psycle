/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#include "module.h"

namespace psy {
  namespace host {

ModuleInfo::ModuleInfo( const std::string& name ) 
  : name_(name) {
}

ModuleInfo::~ModuleInfo() {
}

std::string ModuleInfo::name() const {
  return name_;
}


Module::Module() 
  : song_( 0 )
{
}

Module::~Module()
{
}

void Module::setSong( psy::core::Song* song ) {
  song_ = song;
}

psy::core::Song* Module::song() {
  return song_; 
}


  }
}