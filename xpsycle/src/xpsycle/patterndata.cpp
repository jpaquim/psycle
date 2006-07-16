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

		PatternCategory::PatternCategory( )
		{
			color_ = 0;
		}

		PatternCategory::PatternCategory( const std::string & name )
		{
			name_ = name;
			color_ = 0;
		}

		PatternCategory::~ PatternCategory( )
		{
			for (std::vector<SinglePattern*>::iterator it = begin(); it < end(); it++) {
				delete *it;
			}
		}

		SinglePattern* PatternCategory::createNewPattern( const std::string & name )
		{
			SinglePattern* pattern = new SinglePattern();
			pattern->setCategory(this);
			pattern->setName(name);
			push_back(pattern);
			return pattern;
		}


		void PatternCategory::setName( const std::string & name )
		{
			name_ = name;
		}

		const std::string & PatternCategory::name( ) const
		{
			return name_;
		}

		void PatternCategory::setColor( long color )
		{
			color_ = color;
		}

		long PatternCategory::color( ) const
		{
			return color_;
		}

		// end of Category class


		// the pattern data class
		PatternData::PatternData()
		{
		}

		PatternData::~PatternData()
		{
			for (std::vector<PatternCategory*>::iterator it = begin(); it < end(); it++) {
				delete *it;
			}
		}


		PatternCategory * PatternData::createNewCategory( const std::string & name )
		{
			PatternCategory* category = new PatternCategory();
			category->setName(name);
			push_back(category);
			return category;
		}

	}
}








