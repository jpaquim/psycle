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
#include "patterndata.h"
#include <algorithm>

namespace psycle
{
	namespace host
	{

		PatternData::PatternData()
		{
			lock = false;
		}

		PatternData::~PatternData()
		{
			lock = true;
			for (std::vector<SinglePattern*>::iterator it = begin(); it < end(); it++) {
				delete *it;
			}
		}


		SinglePattern* PatternData::createNewPattern( const std::string & name )
		{
			SinglePattern* pattern = new SinglePattern();
			pattern->setName(name);
			push_back(pattern);
			return pattern;
		}

		SinglePattern * PatternData::findByPtr( SinglePattern * ptr )
		{
			std::vector<SinglePattern*>::iterator it = find(begin(), end(), ptr);
			if (it != end() ) return *it;
			return 0;
		}

		void PatternData::onDeletePattern( SinglePattern* ptr )
		{
			if (!lock) {
				std::vector<SinglePattern*>::iterator it = find(begin(), end(), ptr);
				if (it != end() ) erase(it);
			}
		}

	}
}
