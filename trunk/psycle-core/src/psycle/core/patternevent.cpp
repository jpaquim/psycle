// -*- mode:c++; indent-tabs-mode:t -*-
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

#include "patternevent.h"
#include <sstream>

namespace psy { namespace core {
		
PatternEvent::PatternEvent() :
	note_(255),
	inst_(255),
	mach_(255),
	cmd_(0),
	param_(0),
	volume_(255)
{
	for(int i = 0; i < 10; i++) paraCmdList_.push_back(PcmType());
}

std::string PatternEvent::toXml( int track ) const
{
	std::ostringstream xml;
	xml
		<< "<patevent track='" << track
		<< std::hex << "' note='" << (int) note_
		<< std::hex << "' mac='" << (int) mach_
		<< std::hex << "' inst='" << (int) inst_
		<< std::hex << "' cmd='" << (int) cmd_
		<< std::hex << "' param='" << (int) param_
		<<"' />\n";
	return xml.str();
}

}}
