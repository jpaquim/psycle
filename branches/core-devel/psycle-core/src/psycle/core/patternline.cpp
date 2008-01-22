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
#include <sstream>

namespace psy { namespace core {

void PatternLine::setNoteEvent(track_t index,NoteEvent& pevent)
{
	noteiterator it = noteMap.find(index);
	if ( it == noteMap.end() ) 
	{
		noteMap[index]=pevent;
	}
	else
	{
		it->second = pevent;
	}
}
void PatternLine::remNoteEvent(track_t index)
{
	noteiterator it = noteMap.find(index);
	if ( it != noteMap.end() ) 
	{
		noteMap.erase(it);
	}
}
NoteEvent & PatternLine::noteEvent(track_t index)
{
	noteiterator it = noteMap.find(index);
	if ( it == noteMap.end() ) return emptyevent;
	else return it->second;
}
const NoteEvent & PatternLine::noteEvent(track_t index) const
{
	notec_iterator it = noteMap.find(index);
	if ( it == noteMap.end() ) return emptyevent;
	else return it->second;
}

void PatternLine::setTweak(track_t index,TweakEvent& pevent)
{
	tweakiterator it = tweakMap.find(index);
	if ( it == tweakMap.end() ) 
	{
		tweakMap[index]=pevent;
	}
	else
	{
		it->second = pevent;
	}

}
void PatternLine::remTweak(track_t index)
{
	tweakiterator it = tweakMap.find(index);
	if ( it != tweakMap.end() ) 
	{
		tweakMap.erase(it);
	}
}
TweakEvent & PatternLine::tweak(track_t index)
{
	tweakiterator it = tweakMap.find(index);
	if ( it == tweakMap.end() ) return emptytweak;
	else return it->second;
}
const TweakEvent & PatternLine::tweak(track_t index) const
{
	tweakc_iterator it = tweakMap.find(index);
	if ( it == tweakMap.end() ) return emptytweak;
	else return it->second;
}


std::string PatternLine::toXml( float pos ) const
{
	std::ostringstream xml;
	xml << "<patline pos='" << pos << "'>" << std::endl;
	for ( notec_iterator it = noteMap.begin() ; it != noteMap.end() ; it++ ) {
		int trackNumber = it->first;
		const NoteEvent & event = it->second;
		xml << event.toXml( trackNumber );
	}
	for ( tweakc_iterator it = tweakMap.begin() ; it != tweakMap.end() ; it++ ) {
		int trackNumber = it->first;
		const TweakEvent & event = it->second;
		xml << event.toXml( trackNumber );
	}
	xml << "</patline>" << std::endl;
	///\todo: save tweaks.
	return xml.str();
}

}}
