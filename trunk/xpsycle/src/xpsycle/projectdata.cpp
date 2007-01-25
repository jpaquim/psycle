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
#include "projectdata.h"


ProjectData::ProjectData()
{
}

ProjectData::~ProjectData()
{
  for ( std::vector<psy::core::Song*>::iterator it = songs_.begin(); it < songs_.end(); it++ )
    delete *it;
}

psy::core::Song* ProjectData::createSong() {
  psy::core::Song* song = new psy::core::Song();
  songs_.push_back( song );
  return song;
}

psy::core::Song* ProjectData::songByName( const std::string& name ) {
  psy::core::Song* foundSong = 0;
  std::vector<psy::core::Song*>::const_iterator it = songs_.begin();
  for ( ; it < songs_.end(); it++ ) {
    psy::core::Song* song = *it;
    if ( song->info().name() == name ) {
      foundSong = song;
      break;
    }
  }
  return foundSong;
}