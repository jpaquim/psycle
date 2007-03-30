/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include <sstream>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psy
{
	namespace core
	{

      PatternData::PatternData() {
      }

      PatternData::~PatternData() {
      }

      SinglePattern& PatternData::newPattern() {
        patterns_.push_back( SinglePattern() );
        return patterns_.back(); 
      }

      void PatternData::erase( std::list<SinglePattern>::iterator it ) {
        patterns_.erase( it );
      }

      void PatternData::clear() {
        patterns_.clear();
      }

      const std::list<SinglePattern>::const_iterator PatternData::begin() const {
        return patterns_.begin();
      }

      const std::list<SinglePattern>::const_iterator PatternData::end() const {
        return patterns_.end();
      }

      std::list<SinglePattern>::iterator PatternData::begin() {
        return patterns_.begin();
      }

      std::list<SinglePattern>::iterator PatternData::end() {
        return patterns_.end();
      }

      std::list<SinglePattern>::iterator PatternData::patternByName( const std::string& name) {
        std::list<SinglePattern>::iterator it = patterns_.begin();
        for ( ; it != patterns_.end(); it++ ) {
          const SinglePattern& pattern = *it;
          if ( pattern.name() == name ) return it;
        }
        return patterns_.end();
      }

	}
}
