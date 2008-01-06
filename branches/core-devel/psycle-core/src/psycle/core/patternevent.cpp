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
#include "patternevent.h"
#include <sstream>

namespace psy { namespace core {
	
NoteEvent::PcmType NoteEvent::emptyCmd_;

NoteEvent::NoteEvent()
	:note_(255)
	,volume_(255)
{}

void NoteEvent::setParamCmd(std::uint8_t index,PcmType value)
{
	PcmListType::iterator it = paraCmdList_.find(index);
	if ( it == paraCmdList_.end() ) 
	{
		paraCmdList_[index]=value;
	}
	else
	{
		it->second = value;
	}
}
void NoteEvent::remParamCmd(std::uint8_t index)
{
	PcmListType::iterator it = paraCmdList_.find(index);
	if ( it != paraCmdList_.end() ) 
	{
		paraCmdList_.erase(it);
	}
}
const NoteEvent::PcmType & NoteEvent::paramCmd(std::uint8_t index) const
{
	PcmListType::const_iterator it = paraCmdList_.find(index);
	if ( it == paraCmdList_.end() ) return emptyCmd_;
	else return it->second;
}

std::string NoteEvent::toXml( int track ) const
{
	std::ostringstream xml;
	xml << "<NoteEvent track='" << track
		<< std::hex << "' note='" << (int) note_
		<< std::hex << "' volume='" << (int) volume_;
		for ( PcmListType::const_iterator it = paraCmdList_.begin(); it != paraCmdList_.end(); ++it)
		{
			xml << std::hex << "' param" << (int) it->first << "='" << (int) it->second.first << (int) (it->second.second) ;
		}
	xml <<"' />\n";
	return xml.str();
}


/************************************************************************/
// TweakEvent
std::string TweakEvent::toXml( int track ) const
{
	std::ostringstream xml;
	xml << "<TrackEvent track='" << track;
		<< std::hex << "' value='" << (int) value_
		<<"' />\n";
	return xml.str();
}


}}
