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
#include "Pattern.h"
#include "PatternPool.h"
#include "helpers/xml.h"
#include <sstream>

namespace psy { namespace core {
/************************************************************************/
// TrackInfo
TrackInfo::TrackInfo()
	:macIdx_(0xFFFF)
	,commands_(0)
	,ismuted_(false)
{}

TrackInfo::TrackInfo( std::uint16_t mac, bool muted)
	:macIdx_(mac)
	,commands_(0)
	,ismuted_(muted)
{}

void TrackInfo::setMachineIdx(std::uint16_t macindex)
{
	macIdx_=macindex;
	std::list<TweakTrackInfo*>::iterator endit = tweakinfos.end();
	for (std::list<TweakTrackInfo*>::iterator it = tweakinfos.begin(); it != endit; it++)
	{
		*it.setMachineIdx(macindex);
	}
}

void TrackInfo::addtweakinfo(TweakTrackInfo* tinfo)
{
	tinfo->setMachineIdx(macIdx_);
	tweakinfos.push_back(tinfo);
}
void TrackInfo::changetweakinfo(std::uint8_t index,TweakTrackInfo* tinfo)
{
	std::list<TweakTrackInfo*>::iterator endit = tweakinfos.end();
	int count=0;
	for (std::list<TweakTrackInfo*>::iterator it = tweakinfos.begin(); it != endit; it++)
	{
		if (count == index) { *it = tinfo; break; }
		count++;
	}
}
void TrackInfo::removetweakinfo(TweakTrackInfo* tinfo)
{
	tweakinfos.remove(tinfo);
}

std::string TrackInfo::toXml( int track ) const
{
	std::ostringstream xml;
	xml << "<trackInfo track='" << track
		<< "' machineIndex='" << macIdx_
		<< "' commands='" << commands_
		<< "' muted='" << ismuted_ << "'>" << std::endl;

	for ( std::list<TweakTrackInfo*>::const_iterator it = tweakinfos.begin() ; it != tweakinfos.end() ; it++ ) {
		const TweakTrackInfo* tinfo = *it;
		xml << tinfo->toXml();
	}
	xml << "</trackInfo>" << std::endl;
}
/**************************************************************************/
// TweakTrackInfo

char *TweakTrackInfo::tweakTypeText[]={ "twk", "tws", "mdi", "mdis", "wire", "wires", "aut"  };

TweakTrackInfo::TweakTrackInfo()
	:paramIdx_(0)
	,type_( twk )
{
}

TweakTrackInfo::TweakTrackInfo(TweakType type, std::uint16_t param )
	:type_( type )
	,paramIdx_( param)
{
}

bool TweakTrackInfo::operator<(const TweakTrackInfo & key) const {
	return parameterIdx() < key.parameterIdx();
}

std::string TweakTrackInfo::toXml() const
{
	std::ostringstream xml;
	xml << "<tweak type='" << tweakTypeText[type()]
		<< "' parameter='" << (int) paramIdx_ << "' >" << std::endl;
}

/**************************************************************************/
// Pattern

//int Pattern::idCounter = 0;
TimeSignature Pattern::defaultSignature;

Pattern::Pattern()
{
	TimeSignature timeSig;
	timeSig.setCount(4);
	timeSignatures_.push_back( timeSig  );
	beatZoom_ = 4;
	category_ = 0;
//	id_ = idCounter;
//	idCounter++;
}

// Explicit copy constructor needed because boost::signal is noncopyable
Pattern::Pattern(Pattern const& other)
	: beatZoom_(other.beatZoom_)
	, id_(other.id_)
	, name_(other.name_)
	, category_(other.category_)
	, timeSignatures_(other.timeSignatures_)
{
//	std::map<int,TrackInfo> trackInfoMap;
//	std::map<TweakTrackInfo,int> tweakInfoMap;
//	std::map<double, PatternLine> lineMap;
}

Pattern::~Pattern()
{
	wasDeleted(this);
}


void Pattern::setID( int id )
{
	id_ = id;
//	idCounter = std::max(id_,idCounter)+1;
}

void Pattern::addBar( const TimeSignature & signature )
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

void Pattern::removeBar( float pos )
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

			iterator startIt = lower_bound(pos);
			iterator endIt   = upper_bound(pos);

			if (startIt != end() && endIt != end() ) {
				lineMap.erase(startIt, endIt);
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

bool Pattern::barStart( double pos , TimeSignature & signature ) const
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

void Pattern::clearBars( )
{
	timeSignatures_.clear();
}

const TimeSignature & Pattern::timeSignatureAt(double pos) const
{
	double bts = 0;
	std::vector<TimeSignature>::const_iterator it = timeSignatures_.begin();
	for (; it < timeSignatures_.end(); it++)
	{
		const TimeSignature & timeSignature = *it;
		bts += timeSignature.beats();
		if (pos < bts) return timeSignature;
	}
	return defaultSignature;
}

float Pattern::beats( ) const
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


Pattern::iterator Pattern::find_nearest( int line )
{
	iterator result;

	double low = ( (line - 0.5) / (float) beatZoom() );
	double up  = (line + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

Pattern::const_iterator Pattern::find_nearest( int line ) const
{
	const_iterator result;

	double low = ( (line - 0.499999) / (float) beatZoom() );
	double up  = (line + 0.49999999) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

Pattern::iterator Pattern::find_lower_nearest( int linenr ) {
	iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
};

Pattern::const_iterator Pattern::find_lower_nearest( int linenr ) const
{
	const_iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lower_bound( low );

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
}

void Pattern::setEvent( int line, int track, const NoteEvent & event ) {
	iterator it = find_nearest( line );
	if ( it != end())
	{
		it->second.noteEvent(track) = event;
	} else {
		float position = line / (float) beatZoom();
		lineMap[position].noteEvent(track) = event;
	}
}

NoteEvent Pattern::event( int line, int track ) {
	iterator it = find_nearest( line );
	if ( it != end())
		return it->second.noteEvent(track);
	else
		return NoteEvent();
}

void Pattern::setTweakEvent( int line, int track, const TweakEvent & event ) {
	iterator it = find_nearest( line );
	if ( it != end())
	{
		it->second.tweak(track) = event;
	} else {
		float position = line / (float) beatZoom();
		lineMap[position].tweak(track) = event;
	}
}

TweakEvent Pattern::tweakEvent( int line, int track ) {
	iterator it = find_nearest( line );
	if ( it != end())
		return it->second.tweak(track);
	else
		return TweakEvent();
}


void Pattern::clearTrack( int linenr , int tracknr ) {
	///\todo: verify this code
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.remNoteEvent(tracknr);
	if ( line.isnotesempty() ) lineMap.erase(it);
}

void Pattern::clearTweakTrack( int linenr , int tracknr ) {
	///\todo: verify this code
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.remTweak(tracknr);
	if ( line.empty() ) lineMap.erase(it);
}

bool Pattern::lineIsEmpty( int linenr ) const {
	return ( find_nearest(linenr) == end() );
}
/*
void Pattern::clearEmptyLines()
{
	for( iterator it = begin(); it != end(); )
	{
		if(it->second.empty())
			lineMap.erase(it++);
		else
			++it;
	}
}

void Pattern::scaleBlock(int left, int right, double top, double bottom, float factor)
{
	double length = bottom - top;

	if(factor>1) //expanding-- iterate backwards
	{
		std::map<double, PatternLine>::reverse_iterator rLineIt = (reverse_iterator)(lower_bound(bottom));

		// use > instead of >= -- lines at exactly top don't need to be moved
		for(; rLineIt != rend() && rLineIt->first >top; ++rLineIt )
		{
			PatternLine & line = rLineIt->second;
			double newpos = top + (rLineIt->first-top) * factor;

			for( std::map<int, NoteEvent>::iterator entryIt = line.notes().lower_bound(left)
				; entryIt != line.notes().end() && entryIt->first < right
				; )
			{
				if( newpos < beats() )
				{
					(*this)[newpos].noteEvent(entryIt->first) = entryIt->second;
				} 
				line.noteEvent(entryIt++);
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
					(*this)[newpos].noteEvent(entryIt->first) = entryIt->second;
				}
				line.noteEvent(entryIt++);
			}
		}
	}
}

void Pattern::transposeBlock(int left, int right, double top, double bottom, int trp)
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

void Pattern::deleteBlock(int left, int right, double top, double bottom)
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




std::auto_ptr<Pattern> Pattern::block( int left, int right, int top, int bottom )
{
	// copies a given block into a new Pattern
	// the range is:
	// startTrack >= tracks < endTracks ; startLine >= lines < endLines

	float topBeat = top / (float) beatZoom();

	std::auto_ptr<Pattern> newPattern(new Pattern());

	for( iterator lineIt = find_lower_nearest( top )
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
		newPattern->insert( Pattern::value_type( lineIt->first-topBeat, newLine ) );
	}
	return newPattern;
}

void Pattern::copyBlock(int left, int top, const Pattern & pattern, int tracks, float maxBeats) {
	float pasteStartPos = top / (float) beatZoom();
	deleteBlock(left,left+tracks, pasteStartPos, pasteStartPos+ maxBeats);
	for( const_iterator lineIt = pattern.begin()
		; lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
	{
		const PatternLine & line = lineIt->second;
		for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
			; entryIt != line.notes().end() && entryIt->first <= tracks
			; entryIt++ )
		{
			(*this)[pasteStartPos+lineIt->first].noteEvent(left+entryIt->first)=entryIt->second;
		}
	}
}

void Pattern::mixBlock(int left, int top, const Pattern & pattern, int tracks, float maxBeats) {
	float pasteStartPos =  top / (float) beatZoom() ;
	for( const_iterator lineIt = pattern.begin()
		; lineIt != pattern.end() && lineIt->first < maxBeats; ++lineIt )
	{
		const PatternLine & line = lineIt->second;
		for( std::map<int, PatternEvent>::const_iterator entryIt = line.notes().begin()
			; entryIt != line.notes().end() && entryIt->first <= tracks
			; entryIt++ )
		{
			(*this)[pasteStartPos+lineIt->first].noteEvent(left+entryIt->first)=entryIt->second;
		}
	}
}

void Pattern::deleteBlock( int left, int right, int top, int bottom )
{
	// deletes a given block into a new Pattern
	// the range is:
	// startTrack >= tracks < endTracks ; startLine >= lines < endLines

	Pattern newPattern;

	for( iterator lineIt = find_lower_nearest( top )
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
std::string Pattern::toXml( ) const
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
TweakTrackInfo Pattern::tweakInfo( int track ) const
{
	std::map<TweakTrackInfo, int>::const_iterator it = tweakInfoMap.begin();
	for ( ; it != tweakInfoMap.end(); it++ ) {
		if ( it->second == track ) return it->first;
	}
	return TweakTrackInfo();
}

int Pattern::tweakTrack( const TweakTrackInfo & info )
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
*/
}}
