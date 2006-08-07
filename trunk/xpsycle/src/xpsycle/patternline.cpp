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
#include "patternline.h"
#include <sstream>

namespace psycle
{
	namespace host
	{
		
		
		PatternLine::PatternLine()
		{
		}

		PatternLine::~PatternLine()
		{
		}

		std::string PatternLine::toXml( float pos ) const
		{
			std::ostringstream xml;
			xml << "<patline pos='" << pos << "'>" << std::endl;
			for ( const_iterator it = begin() ; it != end() ; it++ ) {
				int trackNumber = it->first;
				const PatternEvent & event = it->second;
				xml << event.toXml( trackNumber );
			}
			xml << "</patline>" << std::endl;
			return xml.str();
		}

	}
}







