/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include <psycle/core/psycleCorePch.hpp>
#include "patternline.h"
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {

PatternLine::PatternLine()
{
	sequencerTrack_ = 0;
}

PatternLine::~PatternLine()
{
}

std::string PatternLine::toXml( float pos ) const
{
	std::ostringstream xml;
	xml << "<patline pos='" << pos << "'>" << std::endl;
	for ( std::map<int, PatternEvent>::const_iterator it = noteMap.begin() ; it != noteMap.end() ; it++ ) {
		int trackNumber = it->first;
		const PatternEvent & event = it->second;
		xml << event.toXml( trackNumber );
	}
	xml << "</patline>" << std::endl;
	return xml.str();
}

void PatternLine::setSequenceTrack( int track )
{
		sequencerTrack_ = track;
}

int PatternLine::sequenceTrack( ) const
{
		return sequencerTrack_;
}

std::map< int, PatternEvent > & PatternLine::notes( )
{
	return noteMap;
}

const std::map< int, PatternEvent > & PatternLine::notes( ) const
{
	return noteMap;
}

std::map< int, PatternEvent > & PatternLine::tweaks( )
{
	return tweakMap;
}

const std::map< int, PatternEvent > & PatternLine::tweaks( ) const
{
	return tweakMap;
}

bool PatternLine::empty() const {
	return ( noteMap.empty() && tweakMap.empty() );
}

}}
