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
#include "singlepattern.h"
#include "patterndata.h"
#include "helpers/xml.h"
#include <sstream>

namespace psy { namespace core {

/**************************************************************************/
// TweakTrackInfo

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


/**************************************************************************/
// SinglePattern

int SinglePattern::idCounter = 0;

int SinglePattern::genId() {
  return idCounter++;
}

SinglePattern::SinglePattern()
{
	TimeSignature timeSig;
	timeSig.setCount(4);
	timeSignatures_.push_back( timeSig  );
	beatZoom_ = 4;
	category_ = 0;
	id_ = genId();
}

// Explicit copy constructor needed because boost::signal is noncopyable
SinglePattern::SinglePattern(SinglePattern const& other)
: std::map<double,PatternLine>(other)
, beatZoom_(other.beatZoom_)
, name_(other.name_)
, category_(other.category_)
, timeSignatures_(other.timeSignatures_)
, zeroTime(other.zeroTime)
, id_(genId())
, tweakInfoMap(other.tweakInfoMap)
{
}

SinglePattern::~SinglePattern()
{
wasDeleted(this);
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

				SinglePattern::iterator startIt = lower_bound(pos);
				SinglePattern::iterator endIt   = upper_bound(pos);

				if (startIt != end() && endIt != end() ) {
						erase(startIt, endIt);
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

void SinglePattern::setCategory( PatternCategory * category )
{
	category_ = category;
}

PatternCategory * SinglePattern::category( )
{
	return category_;
}


float SinglePattern::beatsPerLine() const {
	return 1 / (float) beatZoom();
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
	
			for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
				; entryIt != line.notes().end() && entryIt->first < right
				; )
			{
				if( newpos < beats() )
				{
					(*this)[newpos].notes()[entryIt->first] = entryIt->second;
				} 
				line.notes().erase(entryIt++);
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
			
			for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
				; entryIt != line.notes().end() && entryIt->first < right
				; )
			{
				if( newpos < beats() )
				{
					(*this)[newpos].notes()[entryIt->first] = entryIt->second;
				}
				line.notes().erase(entryIt++);
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
		for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound(left)
			; entryIt != line.notes().end() && entryIt->first < right
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
	return ( find_nearest(linenr) == end() );
}

void SinglePattern::clearTrack( int linenr , int tracknr ) {
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.notes().erase(tracknr);
	if ( line.notes().empty() ) erase(it);
}

void SinglePattern::clearTweakTrack( int linenr , int tracknr ) {
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.tweaks().erase(tracknr);
	if ( line.empty() ) erase(it);
}

std::vector< TimeSignature > & SinglePattern::timeSignatures( )
{
	return timeSignatures_;
}

const std::vector< TimeSignature > & SinglePattern::timeSignatures( ) const
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
	xml << "<pattern name='" << replaceIllegalXmlChr(name()) << "' zoom='" << beatZoom() << std::hex << "' id='" << id() << std::hex << "'>" << std::endl;
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

	for ( const_iterator it = begin() ; it != end() ; it++ ) {
		double beatPos = it->first;
		const PatternLine & line = it->second;
		xml << line.toXml( static_cast<float>(beatPos) );
	}
	xml << "</pattern>" << std::endl;
	return xml.str();
}

SinglePattern::iterator SinglePattern::find_nearest( int line )
{
	SinglePattern::iterator result;
	
	double low = ( (line - 0.5) / (float) beatZoom() );
	double up  = (line + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

SinglePattern::const_iterator SinglePattern::find_nearest( int line ) const
{
	SinglePattern::const_iterator result;

	double low = ( (line - 0.499999) / (float) beatZoom() );
	double up  = (line + 0.49999999) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

SinglePattern::iterator SinglePattern::find_lower_nearest( int linenr ) {
	SinglePattern::iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
};

SinglePattern::const_iterator SinglePattern::find_lower_nearest( int linenr ) const
{
	SinglePattern::const_iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
}

void SinglePattern::setEvent( int line, int track, const PatternEvent & event ) {
	iterator it = find_nearest( line );
	if ( it != end())
	{
		it->second.notes()[track] = event;
	} else {
		float position = line / (float) beatZoom();
		(*this)[position].notes()[track] = event;
	}
}

PatternEvent SinglePattern::event( int line, int track ) {
	iterator it = find_nearest( line );
	if ( it != end())
		return it->second.notes()[track];
	else
		return PatternEvent();
}

void SinglePattern::setTweakEvent( int line, int track, const PatternEvent & event ) {
	iterator it = find_nearest( line );
	if ( it != end())
	{
		it->second.tweaks()[track] = event;
	} else {
		float position = line / (float) beatZoom();
		(*this)[position].tweaks()[track] = event;
	}
}

PatternEvent SinglePattern::tweakEvent( int line, int track ) {
	iterator it = find_nearest( line );
	if ( it != end())
		return it->second.tweaks()[track];
	else
		return PatternEvent();
}



std::auto_ptr<SinglePattern> SinglePattern::block( int left, int right, int top, int bottom )
{
	// copies a given block into a new Pattern
	// the range is:
	// startTrack >= tracks < endTracks ; startLine >= lines < endLines

	float topBeat = top / (float) beatZoom();

std::auto_ptr<SinglePattern> newPattern(new SinglePattern());

	for( SinglePattern::iterator lineIt = find_lower_nearest( top )
			; lineIt != end() ; ++lineIt )
	{
		PatternLine newLine;
		PatternLine & line = lineIt->second;
		int y = (int) ( lineIt->first * beatZoom() + 0.5 );
		if ( y >= bottom ) break;

		for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound( left )
		; entryIt != line.notes().end() && entryIt->first < right
		; ++entryIt)
		{
	newLine.notes().insert(std::map<int, PatternEvent>::value_type( entryIt->first-left, entryIt->second));
		}   
		newPattern->insert( SinglePattern::value_type( lineIt->first-topBeat, newLine ) );
	}
	return newPattern;
}

void SinglePattern::copyBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats) {
	float pasteStartPos = top / (float) beatZoom();
	deleteBlock(left,left+tracks, pasteStartPos, pasteStartPos+ maxBeats);
	for( SinglePattern::const_iterator lineIt = pattern.begin()
			; lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
	{
		const PatternLine & line = lineIt->second;
		for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
		; entryIt != line.notes().end() && entryIt->first <= tracks
		; entryIt++ )
		{
		(*this)[pasteStartPos+lineIt->first].notes()[left+entryIt->first]=entryIt->second;
		}
	}
}

void SinglePattern::mixBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats) {
	float pasteStartPos =  top / (float) beatZoom() ;
	for( SinglePattern::const_iterator lineIt = pattern.begin()
			; lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
	{
		const PatternLine & line = lineIt->second;
		for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
		; entryIt != line.notes().end() && entryIt->first <= tracks
		; entryIt++ )
		{
		(*this)[pasteStartPos+lineIt->first].notes()[left+entryIt->first]=entryIt->second;
		}
	}
}

void SinglePattern::deleteBlock( int left, int right, int top, int bottom )
{
	// deletes a given block into a new Pattern
	// the range is:
	// startTrack >= tracks < endTracks ; startLine >= lines < endLines

	SinglePattern newPattern;

	for( SinglePattern::iterator lineIt = find_lower_nearest( top )
			; lineIt != end() ; )
	{
		PatternLine & line = lineIt->second;
		int y = (int) ( lineIt->first * beatZoom() + 0.5 );
		if ( y >= bottom ) break;

		for( std::map<int, PatternEvent>::iterator entryIt = line.notes().lower_bound( left )
		; entryIt != line.notes().end() && entryIt->first < right
		; )
		{
					line.notes().erase(entryIt++);
						}   
		if (line.notes().size() == 0) 
			erase(lineIt++);
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
		for ( it = tweakInfoMap.begin(); it != tweakInfoMap.end(); it++) {
			maxTrack = std::max( maxTrack, it->second + 1 );
		}
		tweakInfoMap[info] = maxTrack;
		return maxTrack;
	}
}

}}
