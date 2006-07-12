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
#include "singlepattern.h"

namespace psycle
{
	namespace host
	{

		SinglePattern::SinglePattern()
		{
			beats_ = 4;
			beatZoom_ = 4;

			lastLine = 0;
		}


		SinglePattern::~SinglePattern()
		{
			beforeDelete.emit(this);
		}

		void SinglePattern::setBeatZoom( int zoom )
		{
			beatZoom_ = zoom;
		}

		int SinglePattern::beatZoom( ) const
		{
			return beatZoom_;
		}

		void SinglePattern::setBeats( int beats )
		{
			beats_ = beats;
		}

		int SinglePattern::beats( ) const
		{
			return beats_;
		}

		const PatternEvent & SinglePattern::dataAt( float position, int track )
		{
			std::list<PatternLine>::iterator it = begin();

			for (; it != end(); it++) {
				PatternLine & line = *it;
				lastLine = &line;
				if (line.tickPosition() == position) {
					return line.trackAt(track);
				}
			}
			return zeroTrack;
		}

		void SinglePattern::setData( float position, int track, const PatternEvent & data )
		{
			if (lastLine && lastLine->tickPosition() == position ) {
				lastLine->setPatternEvent(data, track);
			} else {
			bool inserted = false;
				std::list<PatternLine>::iterator it = begin();
				for (; it != end(); it++) {
					PatternLine & line = *it;
					if (line.tickPosition() == position) {
						line.setPatternEvent(data,track);
					} else
					if (line.tickPosition() > position) {
						PatternLine line(position);
						line.setPatternEvent(data,track);
						insert(it, line);
						inserted = true;
					}
				}
				if (!inserted) {
					PatternLine line(position);
					line.setPatternEvent(data,track);
					push_back(line);
				}
			}
		}

		std::list< PatternLine >::iterator SinglePattern::startItr( float position )
		{
			std::list<PatternLine>::iterator it = begin();
			for (; it != end(); it++) {
				PatternLine & line = *it;
				if (line.tickPosition() == position) {
					return it;
				} else
			if (line.tickPosition() > position) {
				return it;
			}
		}
		return begin();
		}

		void SinglePattern::setName( const std::string & name )
		{
			name_ = name;
		}

		const std::string & SinglePattern::name( ) const
		{
			return name_;
		}

	}
}



