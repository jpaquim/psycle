// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "SinglePattern.h"

#include "xml.h"

#include <sstream>

namespace psy { namespace core {


Pattern::Pattern() 
	: id_(-1)
{
	TimeSignature timeSig;
	timeSig.setCount(4);
	timeSignatures_.push_back(timeSig);
	beatZoom_ = 4;
}

// Explicit copy constructor needed because boost::signal is noncopyable
Pattern::Pattern(Pattern const& other)
:	id_(-1),  
	beatZoom_(other.beatZoom_),
	name_(other.name_),
	category_(other.category_),
	timeSignatures_(other.timeSignatures_),
	zeroTime(other.zeroTime)
{
	lines_ = other.lines_;
}

Pattern::~Pattern() {
	wasDeleted(this);
}

Pattern& Pattern::operator=(const Pattern& rhs)
{
  if (this == &rhs) return *this; 
  lines_ = rhs.lines_;
  beatZoom_ = rhs.beatZoom_;
  name_ = rhs.name_;
  category_ = rhs.category_;
  timeSignatures_ = rhs.timeSignatures_;
  return *this;
}


void Pattern::Clear() {
   lines_.clear();
   TimeSignature timeSig;
   timeSig.setCount(4);
   timeSignatures_.push_back(timeSig);
   beatZoom_ = 4;
}

void Pattern::setBeatZoom( int zoom ) {
	beatZoom_ = zoom;
}

int Pattern::beatZoom( ) const {
	return beatZoom_;
}

void Pattern::addBar( const TimeSignature & signature ) {
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

void Pattern::removeBar( float pos ) {
#if 0 ///\todo
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

				Pattern::iterator startIt = lower_bound(pos);
				Pattern::iterator endIt   = upper_bound(pos);

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
#endif
}

const TimeSignature & Pattern::playPosTimeSignature(double pos) const
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

void Pattern::setName( const std::string & name )
{
	name_ = name;
}

const std::string & Pattern::name( ) const
{
	return name_;
}

void Pattern::setCategory(const std::string& category)
{
	category_ = category;
}

float Pattern::beatsPerLine() const {
	return 1 / (float) beatZoom();
}

void Pattern::clearEmptyLines() {
#if 0 ///\todo
	for( iterator it = begin(); it != end(); ) {
		if(it->second.empty())
			erase(it++);
		else
			++it;
	}
#endif
}

void Pattern::scaleBlock(int left, int right, double top, double bottom, float factor) {
#if 0 ///\todo
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
#endif
}



void Pattern::blockSetInstrument( int left, int right, double top, double bottom, std::uint8_t newInstrument ) {
#if 0 ///\todo
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
			entry.setInstrument( newInstrument );
		}
	}
#endif
}

void Pattern::blockSetMachine( int left, int right, double top, double bottom, std::uint8_t newMachine ) {
#if 0 ///\todo
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
			entry.setMachine( newMachine );
		}
	}
#endif
}

bool Pattern::lineIsEmpty( int linenr ) const {
#if 0 ///\todo
	return ( find_nearest(linenr) == end() );
#else
	return 0;
#endif
}

void Pattern::clearTrack( int linenr , int tracknr ) {
#if 0 ///\todo
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.notes().erase(tracknr);
	if ( line.notes().empty() ) erase(it);
#endif
}

void Pattern::clearTweakTrack( int linenr , int tracknr ) {
#if 0 ///\todo
	iterator it = find_nearest(linenr);
	PatternLine & line = it->second;
	if ( it == end() ) return;
	line.tweaks().erase(tracknr);
	if ( line.empty() ) erase(it);
#endif
}

std::vector<TimeSignature> & Pattern::timeSignatures() {
	return timeSignatures_;
}

const std::vector<TimeSignature> & Pattern::timeSignatures() const {
	return timeSignatures_;
}

std::string Pattern::toXml( ) const {
	std::ostringstream xml;
#if 0 ///\todo
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
#endif
	return xml.str();
}


Pattern::iterator Pattern::find_nearest(int line, int beat_zoom) {
	Pattern::iterator result;
	
	double low = (line - 0.5) / beat_zoom;
	double up  = (line + 0.5) / beat_zoom;

	result = lines_.lower_bound(low);

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

Pattern::const_iterator Pattern::find_nearest(int line, int beat_zoom) const {
	Pattern::const_iterator result;

	double low = (line - 0.499999) / (double) beat_zoom;
	double up  = (line + 0.49999999) / (double) beat_zoom;

	result = lines_.lower_bound(low);

	if ( result != end() && result->first >=low && result->first < up ) {
		return result;
	}
	return end();
}

Pattern::iterator Pattern::find_lower_nearest( int linenr ) {
	Pattern::iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lines_.lower_bound(low);

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
};

Pattern::const_iterator Pattern::find_lower_nearest( int linenr ) const
{
	Pattern::const_iterator result;

	double low = (linenr - 0.5) / (float) beatZoom();
	double up  = (linenr + 0.5) / (float) beatZoom();

	result = lines_.lower_bound( low );

	if ( result != end() && result->first >=low ) {
		return result;
	}
	return end();
}


void Pattern::setEvent(int line, int track, int beat_zoom, const PatternEvent& event) {
	iterator it = find_nearest(line, beat_zoom);
	if ( it != end())
	{
		it->second = event;
		it->second.set_track(track);
	} else {
		double position = line / (double) beat_zoom;
		iterator it = lines_.insert(std::pair<double, PatternEvent>(position, event));
		it->second.set_track(track);
	}
}

PatternEvent Pattern::event(int line, int track, int beat_zoom) {
	iterator it = find_nearest(line, beat_zoom);
	if ( it != end()) {
		double pos = it->first;
		double low = (line - 0.499999) / (double)beat_zoom;
		double up  = (line + 0.49999999) / (double)beat_zoom;
		while ( it != lines_.end() && pos <= up) {
			if ( it->second.track() == track )
				return it->second;
			++it;
		}
	}
	return PatternEvent();	
}

void Pattern::setTweakEvent( int line, int track, const PatternEvent & event ) {
#if 0 ///\todo
	iterator it = find_nearest( line );
	if ( it != end())
	{
		it->second.tweaks()[track] = event;
	} else {
		float position = line / (float) beatZoom();
		(*this)[position].tweaks()[track] = event;
	}
#endif
}

PatternEvent Pattern::tweakEvent( int line, int track ) {
#if 0 ///\todo
	iterator it = find_nearest( line );
	if ( it != end())
		return it->second.tweaks()[track];
	else
#endif
		return PatternEvent();
}

Pattern Pattern::Clone(double from, double to, int start_track, int end_track) {
	Pattern clone_pattern;
	Pattern::iterator it(lower_bound(from));
    while (it != end()) {
		PatternEvent& pattern_event = it->second;
        if (it->first > to)
          break;  
		if (pattern_event.track() >= start_track && 
			pattern_event.track() <= end_track) {
				Pattern::iterator cloned_it = clone_pattern.insert(it->first - from, pattern_event);
				cloned_it->second.set_track(pattern_event.track() - start_track);
		}
		++it;
    }
	return clone_pattern;
}

void Pattern::insert(const Pattern& src_pattern, double to, int to_track) {
  Pattern::const_iterator it(src_pattern.begin());
  while (it != src_pattern.end()) {
    const PatternEvent& line = it->second;
	Pattern::iterator new_it = insert(it->first + to, line);
	new_it->second.set_track(to_track + line.track());
    ++it;
  }
}

void Pattern::erase(double from, double to, int start_track, int end_track) {
   Pattern::iterator it(lower_bound(from));
   while (it != end()) {
     PatternEvent& patternEvent = it->second;
     if ( it->first >= to )
		break;
	 if (patternEvent.track() <= end_track &&
		 patternEvent.track() >= start_track
	  ) {
		it = erase(it);
	} else it++;
  }
}

void Pattern::Transpose(int delta, double from, double to, int start_track, int end_track)
{
  Pattern::iterator it(lower_bound(from));
  while (it != end()) {
     PatternEvent& pattern_ev = it->second;
     if ( it->first >= to )
		break;
	 if (pattern_ev.track() <= end_track &&
		 pattern_ev.track() >= start_track
	  ) {
		int note = pattern_ev.note();
		if (note < 120) {
		   pattern_ev.setNote(std::min(std::max(0, note + delta), 119));
		}
	 }
	 ++it;
  }
}


}}
