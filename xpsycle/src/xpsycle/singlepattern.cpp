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
#include "patterndata.h"

namespace psycle
{
	namespace host
	{

		SinglePattern::SinglePattern()
		{
			timeSignatures.push_back( TimeSignature() );
			beatZoom_ = 4;
			category_ = 0;
		}


		SinglePattern::~SinglePattern()
		{
		}

		void SinglePattern::setBeatZoom( int zoom )
		{
			beatZoom_ = zoom;
		}

		int SinglePattern::beatZoom( ) const
		{
			return beatZoom_;
		}

		void SinglePattern::addBar( const TimeSignature & signature )
		{
			if ( timeSignatures.size() > 0 ) {
				TimeSignature & last = timeSignatures.back();
				if (last.numerator()   != signature.numerator() ||
						last.denominator() != signature.denominator() )
				{
					timeSignatures.push_back(signature);
				} else
				timeSignatures.back().incCount();
			}
		}

		const TimeSignature & SinglePattern::playPosTimeSignature(double pos) const
		{
				int bts = 0;
				std::vector<TimeSignature>::const_iterator it = timeSignatures.begin();
				for (; it < timeSignatures.end(); it++)
				{
					const TimeSignature & timeSignature = *it;
					bts += timeSignature.beats();
					if (pos < bts) return timeSignature;
				}
				return zeroTime;
		}

		bool SinglePattern::barStart( double pos , TimeSignature & signature ) const
		{
			if (pos - ((int) pos) != 0) return false;
			int bts = 0;
			std::vector<TimeSignature>::const_iterator it = timeSignatures.begin();
			for (; it < timeSignatures.end(); it++)
			{
				const TimeSignature & timeSignature = *it;
				for (int count = 0; count < timeSignature.count(); count++) {
					if (bts == pos) {
						signature = timeSignature;
						return true;
					}
					bts += timeSignature.numerator();
				}
			}
			return false;
		}

		int SinglePattern::beats( ) const
		{
			int bts = 0;
			std::vector<TimeSignature>::const_iterator it = timeSignatures.begin();
			for (; it < timeSignatures.end(); it++)
			{
				const TimeSignature & signature = *it;
				bts += signature.beats();
			}
			return bts;
		}

		void SinglePattern::setName( const std::string & name )
		{
			name_ = name;
		}

		const std::string & SinglePattern::name( ) const
		{
			return name_;
		}

		void SinglePattern::setCategory( PatternCategory * category )
		{
			category_ = category;
		}

		PatternCategory * SinglePattern::category( )
		{
			return category_;
		}

		void SinglePattern::clearEmptyLines()
		{
			for( iterator it = begin(); it != end(); )
			{
				if(it->second.empty())
					erase(it++);
				else
					++it;
			}
		}

		void SinglePattern::scaleBlock(int left, int right, double top, double bottom, float factor)
		{
			double length = bottom - top;
			
			if(factor>1) //expanding-- iterate backwards
			{
				reverse_iterator rLineIt = (reverse_iterator)(lower_bound(bottom));
			
				// use > instead of >= -- lines at exactly top don't need to be moved
				for(; rLineIt != rend() && rLineIt->first >top; ++rLineIt )
				{
					PatternLine & line = rLineIt->second;
					double newpos = top + (rLineIt->first-top) * factor;
			
					for( PatternLine::iterator entryIt = line.lower_bound(left)
					   ; entryIt != line.end() && entryIt->first < right
					   ; )
					{
						if( newpos < beats() )
						{
							(*this)[newpos][entryIt->first] = entryIt->second;
						} 
						line.erase(entryIt++);
					}
				}
			}
			else //contracting -- iterate forwards
			{
				//use upper_bound, not lower_bound.. lines at exactly top don't need to be moved
				iterator lineIt = upper_bound(top);
				
				for(; lineIt != end() && lineIt->first < bottom; ++lineIt )
				{
					PatternLine & line = lineIt->second;
					double newpos = top + (lineIt->first-top) * factor;
					
					for( PatternLine::iterator entryIt = line.lower_bound(left)
					   ; entryIt != line.end() && entryIt->first < right
					   ; )
					{
						if( newpos < beats() )
						{
							(*this)[newpos][entryIt->first] = entryIt->second;
						}
						line.erase(entryIt++);
					}
				}
			}
		}

		void SinglePattern::transposeBlock(int left, int right, double top, double bottom, int trp)
		{
			for( iterator lineIt = lower_bound(top)
			   ; lineIt != end() && lineIt->first < bottom
			   ; ++lineIt )
			{
				PatternLine & line = lineIt->second;
				for( PatternLine::iterator entryIt = line.lower_bound(left)
				   ; entryIt != line.end() && entryIt->first < right
				   ; ++entryIt)
				{
					PatternEvent & entry = entryIt->second;
					int note = entry.note();
					if ( note < 120 ) {
						note+=trp;
						if ( note < 0 )   note = 0;
						if (note > 119)   note = 119;
						entry.setNote(note);
					}
				}
			}
		}

		void SinglePattern::deleteBlock(int left, int right, double top, double bottom)
		{
			for( iterator lineIt = lower_bound(top)
			   ; lineIt != end() && lineIt->first < bottom
			   ; ++lineIt )
			{
				PatternLine & line = lineIt->second;
				for( PatternLine::iterator entryIt = line.lower_bound(left)
				   ; entryIt != line.end() && entryIt->first < right
				   ; )
				{
					line.erase(entryIt++);
				}
			}
			clearEmptyLines();
		}

		void SinglePattern::clearPosition(double beatpos, int track, int column)
		{
			if(!count(beatpos)) return;

			PatternLine &line = (*this)[beatpos];

			if(!line.count(track)) return;
			
			if(column==0)
			{
				line.erase(track);
				if(line.empty()) 
					erase(beatpos);
				return;
			}
			
			PatternEntry *pEntry = line[track].entry();
			if (column < 5 )   { *((std::uint8_t*)pEntry+(column+1)/2) = 255; }
			else	                { *((std::uint8_t*)pEntry+(column+1)/2) = 0; }
			
		}



	}
}




