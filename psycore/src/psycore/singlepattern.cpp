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
#include "singlepattern.h"
#include "patterndata.h"
#include <sstream>
#include <iostream>
#include <cmath>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psy
{
	namespace core
	{

		TweakTrackInfo::TweakTrackInfo() :
				macIdx_(0xFF),
				paramIdx_(0),
				type_( twk )
		{
		}

		TweakTrackInfo::TweakTrackInfo( int mac, int param, TweakType type) :
				macIdx_( mac),
				paramIdx_( param),
				type_( type )
		{
		}

		TweakTrackInfo::~TweakTrackInfo() {

		}

		int TweakTrackInfo::machineIdx() const {
			return macIdx_;
		}

		int TweakTrackInfo::parameterIdx() const {
			return paramIdx_;
		}

		TweakTrackInfo::TweakType TweakTrackInfo::type() const {
			return type_;
		}

    bool TweakTrackInfo::operator<(const TweakTrackInfo & key) const {
      long key1 = machineIdx() | parameterIdx() << 8;
      long key2 = key.machineIdx() | key.parameterIdx() <<8;
      return key1 < key2;
    };



		int SinglePattern::idCounter = 0;

		SinglePattern::SinglePattern()
		{
			TimeSignature timeSig;
			timeSig.setCount(4);
			timeSignatures_.push_back( timeSig  );
			beatZoom_ = 4;
			id_ = idCounter;
			idCounter++;
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
			if ( timeSignatures_.size() > 0 ) {
				TimeSignature & last = timeSignatures_.back();
				if (last.numerator()   != signature.numerator() ||
						last.denominator() != signature.denominator() )
				{
					timeSignatures_.push_back(signature);
				} else
				timeSignatures_.back().incCount();
			} else
					timeSignatures_.push_back(signature);
		}

		void SinglePattern::removeBar( float pos )
		{
			float searchPos = 0;
			std::vector<TimeSignature>::iterator it = timeSignatures_.begin();
			for (; it < timeSignatures_.end(); it++) {
				TimeSignature & timeSignature = *it;
				float oldPos = searchPos;
				searchPos += timeSignature.beats();
				if (searchPos > pos) {
						// found our bar
						float beginPos = searchPos;
						float endPos   = oldPos;

						std::map<double, PatternLine>::iterator startIt = patternMap.lower_bound(pos);
						std::map<double, PatternLine>::iterator endIt   = patternMap.upper_bound(pos);

						if (startIt != patternMap.end() && endIt != patternMap.end() ) {
								patternMap.erase(startIt, endIt);
						}
						if (timeSignature.count() > 1) {
							timeSignature.setCount(timeSignature.count()-1);
						} else {
							timeSignatures_.erase(it);
						}
						break;
				}
			}
		}

		const TimeSignature & SinglePattern::playPosTimeSignature(double pos) const
		{
				double bts = 0;
				std::vector<TimeSignature>::const_iterator it = timeSignatures_.begin();
				for (; it < timeSignatures_.end(); it++)
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
			std::vector<TimeSignature>::const_iterator it = timeSignatures_.begin();
			for (; it < timeSignatures_.end(); it++)
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

		void SinglePattern::clearBars( )
		{
			timeSignatures_.clear();
		}

		float SinglePattern::beats( ) const
		{
			float bts = 0;
			std::vector<TimeSignature>::const_iterator it = timeSignatures_.begin();
			for (; it < timeSignatures_.end(); it++)
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

		float SinglePattern::beatsPerLine() const {
			return 1 / (float) beatZoom();
		}

		void SinglePattern::clearEmptyLines()
		{
          std::map<double, PatternLine>::iterator it = patternMap.begin();
		  for( ; it != patternMap.end(); )
		  {
		    if( it->second.empty() )
				patternMap.erase( it++ );
			else
				++it;
			}
		}

		void SinglePattern::scaleBlock(int left, int right, double top, double bottom, float factor)
		{
			double length = bottom - top;
			
			if(factor>1) //expanding-- iterate backwards
			{
			    std::map<double, PatternLine>::reverse_iterator rLineIt = (const std::map<double, PatternLine>::reverse_iterator)(patternMap.lower_bound(bottom));
			
				// use > instead of >= -- lines at exactly top don't need to be moved
				for(; rLineIt != patternMap.rend() && rLineIt->first >top; ++rLineIt )
				{
					PatternLine & line = rLineIt->second;
					double newpos = top + (rLineIt->first-top) * factor;
			
					for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
					   ; entryIt != line.notes().end() && entryIt->first < right
					   ; )
					{
						if( newpos < beats() )
						{
                          patternMap[newpos].notes()[entryIt->first] = entryIt->second;
						} 
						line.notes().erase(entryIt++);
					}
				}
			}
			else //contracting -- iterate forwards
			{
				//use upper_bound, not lower_bound.. lines at exactly top don't need to be moved
				std::map<double, PatternLine>::iterator lineIt = patternMap.upper_bound(top);
				
				for(; lineIt != patternMap.end() && lineIt->first < bottom; ++lineIt )
				{
					PatternLine & line = lineIt->second;
					double newpos = top + (lineIt->first-top) * factor;
					
					for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
					   ; entryIt != line.notes().end() && entryIt->first < right
					   ; )
					{
						if( newpos < beats() )
						{
						  patternMap[newpos].notes()[entryIt->first] = entryIt->second;
						}
						line.notes().erase(entryIt++);
					}
				}
			}
		}

		void SinglePattern::transposeBlock(int left, int right, double top, double bottom, int trp)
		{          
			for( std::map<double, PatternLine>::iterator lineIt = patternMap.lower_bound(top)
			   ; lineIt != patternMap.end() && lineIt->first < bottom
			   ; ++lineIt )
			{
				PatternLine& line = lineIt->second;
				for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
				   ; entryIt != line.notes().end() && entryIt->first < right
				   ; ++entryIt)
				{
					PatternEvent& entry = entryIt->second;
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
			for( std::map<double, PatternLine>::iterator lineIt = patternMap.lower_bound(top)
			   ; lineIt != patternMap.end() && lineIt->first < bottom
			   ; ++lineIt )
			{
				PatternLine & line = lineIt->second;
				for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
				   ; entryIt != line.notes().end() && entryIt->first < right
				   ; )
				{
					line.notes().erase(entryIt++);
				}
			}
			clearEmptyLines();
		}

		bool SinglePattern::lineIsEmpty( int linenr ) const {			
			return find_nearest(linenr) == patternMap.end();
		}

		void SinglePattern::clearTrack( int linenr , int tracknr ) {
			std::map<double, PatternLine>::iterator it = find_nearest( linenr );
			PatternLine& line = it->second;
			if ( it == patternMap.end() ) return;
			line.notes().erase(tracknr);
			if ( line.notes().empty() ) patternMap.erase( it );
		}

		void SinglePattern::clearTweakTrack( int linenr , int tracknr ) {
			std::map<double, PatternLine>::iterator it = find_nearest(linenr);
			PatternLine& line = it->second;
			if ( it == patternMap.end() ) return;
			line.tweaks().erase( tracknr );
			if ( line.empty() ) patternMap.erase( it );
		}

		std::vector< TimeSignature >& SinglePattern::timeSignatures( )
		{
  		return timeSignatures_;
		}

		const std::vector< TimeSignature >& SinglePattern::timeSignatures( ) const
		{
  		return timeSignatures_;
		}

		void SinglePattern::setID( int id )
		{
			id_ = id;
			idCounter = std::max(id_,idCounter)+1;
		}

		int SinglePattern::id( ) const
		{
			return id_;
		}

		std::string SinglePattern::toXml( ) const
		{
			std::ostringstream xml;
//			xml << "<pattern name='" << PsyFilter::replaceIllegalXmlChr(name()) << "' zoom='" << beatZoom() << std::hex << "' id='" << id() << std::hex << "'>" << std::endl;
			std::vector<TimeSignature>::const_iterator it = timeSignatures_.begin();
			for ( ; it < timeSignatures_.end(); it++) {
				const TimeSignature & sign = *it;
				xml << "<sign ";
				if (sign.ownerDefined()) {
					xml << "free='" << sign.beats() <<"'";
				} else {
					xml << "num='" << sign.numerator() << "' ";
					xml << "denom='" << sign.numerator() << "' ";
					xml << "count='" << sign.count() << "' ";
				}
				xml << "/>" << std::endl;
			}

			for ( std::map<double, PatternLine>::const_iterator it = patternMap.begin() ; it != patternMap.end() ; ++it ) {
				float beatPos = it->first;
				const PatternLine& line = it->second;
				xml << line.toXml( beatPos );
			}
			xml << "</pattern>" << std::endl;
			return xml.str();
		}

        std::map<double, PatternLine>::const_iterator SinglePattern::lower_bound( double key ) const {
          return patternMap.lower_bound( key);
        }

		std::map<double, PatternLine>::iterator SinglePattern::find_nearest( int line )
		{
			std::map<double, PatternLine>::iterator result;
			
			double low = ( (line - 0.5) / (float) beatZoom() );
			double up  = (line + 0.5) / (float) beatZoom();

			result = patternMap.lower_bound( low );

			if ( result != patternMap.end() && result->first >=low && result->first < up ) {
				return result;
			}
			return patternMap.end();
		}

		std::map<double, PatternLine>::const_iterator SinglePattern::find_nearest( int line ) const
		{
			std::map<double, PatternLine>::iterator::const_iterator result;

			double low = ( (line - 0.499999) / (float) beatZoom() );
			double up  = (line + 0.49999999) / (float) beatZoom();

			result = patternMap.lower_bound( low );

			if ( result != patternMap.end() && result->first >=low && result->first < up ) {
				return result;
			}
			return patternMap.end();
		}

		std::map<double, PatternLine>::iterator SinglePattern::find_lower_nearest( int linenr ) {
			std::map<double, PatternLine>::iterator result;

			double low = (linenr - 0.5) / (float) beatZoom();
			double up  = (linenr + 0.5) / (float) beatZoom();

			result = patternMap.lower_bound( low );

			if ( result != patternMap.end() && result->first >=low ) {
				return result;
			}
			return patternMap.end();
		};

		std::map<double, PatternLine>::const_iterator SinglePattern::find_lower_nearest( int linenr ) const
		{
			std::map<double, PatternLine>::const_iterator result;

			double low = (linenr - 0.5) / (float) beatZoom();
			double up  = (linenr + 0.5) / (float) beatZoom();

			result = patternMap.lower_bound( low );

			if ( result != patternMap.end() && result->first >=low ) {
				return result;
			}
			return patternMap.end();
		}

		void SinglePattern::setEvent( int line, int track, const PatternEvent & event ) {
			std::map<double, PatternLine>::iterator it = find_nearest( line );
			if ( it != patternMap.end())
			{
				it->second.notes()[track] = event;
			} else {
				float position = line / (float) beatZoom();
				patternMap[position].notes()[track] = event;
			}
		}

		PatternEvent SinglePattern::event( int line, int track ) {
			std::map<double, PatternLine>::iterator it = find_nearest( line );
			if ( it != patternMap.end())
				return it->second.notes()[track];
			else
				return PatternEvent();
		}

		void SinglePattern::setTweakEvent( int line, int track, const PatternEvent & event ) {
			std::map<double, PatternLine>::iterator it = find_nearest( line );
			if ( it != patternMap.end())
			{
				it->second.tweaks()[track] = event;
			} else {
				float position = line / (float) beatZoom();
				patternMap[position].tweaks()[track] = event;
			}
		}

		PatternEvent SinglePattern::tweakEvent( int line, int track ) {
			std::map<double, PatternLine>::iterator it = find_nearest( line );
			if ( it != patternMap.end())
				return it->second.tweaks()[track];
			else
				return PatternEvent();
		}


	
		SinglePattern SinglePattern::block( int left, int right, int top, int bottom )
		{
			// copies a given block into a new Pattern
			// the range is:
			// startTrack >= tracks < endTracks ; startLine >= lines < endLines

			float topBeat = top / (float) beatZoom();

			SinglePattern newPattern;
            std::map<double, PatternLine>::iterator lineIt;
			for( lineIt = find_lower_nearest( top )
					; lineIt != patternMap.end() ; ++lineIt )
			{
				PatternLine newLine;
				PatternLine& line = lineIt->second;
				int y = (int) ( lineIt->first * beatZoom() + 0.5 );
				if ( y >= bottom ) break;
		
				for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound( left )
	          ; entryIt != line.notes().end() && entryIt->first < right
	          ; ++entryIt)
				{
    	      newLine.notes().insert(std::map<int, PatternEvent>::value_type( entryIt->first-left, entryIt->second));
  	  	}   
				newPattern.insert( lineIt->first-topBeat, newLine );
			}
			return newPattern;
		}

        void SinglePattern::insert( double pos, const PatternLine& pattern ) {
          patternMap.insert( std::map<double, PatternLine>::value_type( pos, pattern ));
        }

		void SinglePattern::copyBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats) {			
			float pasteStartPos = top / (float) beatZoom();
			deleteBlock(left,left+tracks, pasteStartPos, pasteStartPos+ maxBeats);
            std::map<double, PatternLine>::const_iterator lineIt;
			for( lineIt = pattern.begin(); lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
			{
				const PatternLine & line = lineIt->second;
				for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
	          ; entryIt != line.notes().end() && entryIt->first <= tracks
	          ; entryIt++ )
				{
				patternMap[pasteStartPos+lineIt->first].notes()[left+entryIt->first]=entryIt->second;
				}
			}
		}

		void SinglePattern::mixBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats) {
			float pasteStartPos =  top / (float) beatZoom() ;
            std::map<double, PatternLine>::const_iterator lineIt;
			for( lineIt = pattern.begin(); lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
			{
				const PatternLine& line = lineIt->second;
				for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
	          ; entryIt != line.notes().end() && entryIt->first <= tracks
	          ; entryIt++ )
				{
				patternMap[pasteStartPos+lineIt->first].notes()[left+entryIt->first]=entryIt->second;
				}
			}
		}

		void SinglePattern::deleteBlock( int left, int right, int top, int bottom )
		{
			// deletes a given block into a new Pattern
			// the range is:
			// startTrack >= tracks < endTracks ; startLine >= lines < endLines

			SinglePattern newPattern;

            std::map<double, PatternLine>::iterator lineIt;
			for( lineIt = find_lower_nearest( top ); lineIt != patternMap.end() ; )
			{
				PatternLine& line = lineIt->second;
				int y = (int) ( lineIt->first * beatZoom() + 0.5 );
				if ( y >= bottom ) break;
		
				for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound( left )
	          ; entryIt != line.notes().end() && entryIt->first < right
	          ; )
				{
    	                line.notes().erase(entryIt++);
                }   
				if (line.notes().size() == 0) 
					patternMap.erase(lineIt++);
				else 
					++lineIt;
			}
		}

		TweakTrackInfo SinglePattern::tweakInfo( int track ) const
		{
			std::map<TweakTrackInfo, int>::const_iterator it = tweakInfoMap.begin();
			for ( ; it != tweakInfoMap.end(); it++ ) {
				if ( it->second == track ) return it->first;
			}
			return TweakTrackInfo();
		}

		int SinglePattern::tweakTrack( const TweakTrackInfo & info )
		{
			std::map<TweakTrackInfo, int>::const_iterator it = tweakInfoMap.begin();
			if ( (it = tweakInfoMap.find(info)) != tweakInfoMap.end() ) {
				return it->second;
			} else {
				int maxTrack = 0;
				for ( it = tweakInfoMap.begin(); it != tweakInfoMap.begin(); it++) {
					maxTrack = std::max( maxTrack, it->second + 1 );
				}
				tweakInfoMap[info] = maxTrack;
				return maxTrack;
			}
		}


        std::map<double, PatternLine>::const_iterator SinglePattern::begin() const {
          return patternMap.begin();
        }
        
        std::map<double, PatternLine>::const_iterator SinglePattern::end() const {
          return patternMap.end();
        }

        void SinglePattern::clear() {
          patternMap.clear();
        }

	}	// end of core namespace
}// end of psy namespace
