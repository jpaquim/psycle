// -*- mode:c++; indent-tabs-mode:t -*-
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
#include "patternsequence.h"
#include <sstream>

namespace psy { namespace core {

/**************************************************************************/
// GlobalEvent

GlobalEvent::GlobalEvent( ) : parameter_(0), target_(-1), target2_(-1), type_(NONE)
{
}

GlobalEvent::GlobalEvent( float parameter ): parameter_(parameter), target_(-1), target2_(-1),type_(NONE)
{
}

GlobalEvent::~ GlobalEvent( )
{
}

void GlobalEvent::setParameter( float parameter )
{
	parameter_ = parameter;
}

float GlobalEvent::parameter( ) const
{
	return parameter_;
}

void GlobalEvent::setTarget( int target )
{
	target_ = target;
}

int GlobalEvent::target( ) const
{
	return target_;
}

void GlobalEvent::setTarget2( int target )
{
	target2_ = target;
}

int GlobalEvent::target2( ) const
{
	return target2_;
}

void GlobalEvent::setType( GlobalType type )
{
	type_ = type;
}

GlobalEvent::GlobalType GlobalEvent::type( ) const
{
	return type_;
}


/**************************************************************************/
// SequenceEntry
// pattern Entry contains one ptr to a SinglePattern and the tickPosition for the absolute Sequencer pos

SequenceEntry::SequenceEntry( )
{
	line_ = 0;
	init();
}

SequenceEntry::SequenceEntry( SequenceLine * line )
{
	line_ = line;
	init();
}

void SequenceEntry::init( )
{
	pattern_ = 0;
	startPos_ = 0;
	endPos_ = PatternEnd;
	transpose_ = 0;
}

SequenceEntry::~ SequenceEntry( )
{
wasDeleted(this);
}

void SequenceEntry::setPattern( SinglePattern * pattern )
{
	pattern_ = pattern;
	startPos_ = 0;
	endPos_   = pattern->beats();
}

SinglePattern * SequenceEntry::pattern( )
{
	return pattern_;
}

SinglePattern * SequenceEntry::pattern( ) const
{
	return pattern_;
}

float SequenceEntry::patternBeats() const
{
	return pattern_->beats();
}

double SequenceEntry::tickPosition( ) const
{
	SequenceLine::iterator iter = line_->begin();
	for(; iter!= line_->end(); ++iter)
	{
		if(iter->second==this)
		break;
	}
	if(iter!=line_->end())
	{
		return iter->first;
	}
	return 0;
}

double SequenceEntry::tickEndPosition( ) const
{
	return tickPosition() + patternBeats();
}

void SequenceEntry::setTranspose( int offset )
{
	transpose_ = offset;
}

int SequenceEntry::transpose( ) const
{
	return transpose_;
}

void SequenceEntry::setStartPos( float pos )
{
	startPos_ = pos;
}

float SequenceEntry::startPos( ) const
{
	return startPos_;
}

void SequenceEntry::setEndPos( float pos )
{
	endPos_ = pos;
}

float SequenceEntry::endPos( ) const
{
	return endPos_;
}

std::string SequenceEntry::toXml( double pos ) const
{
	std::ostringstream xml;
	xml << "<seqentry pos='" << pos << "' patid='" << pattern()->id() << std::hex << "' "  << "start='" << startPos() << "' end='" << endPos() << "' " << "transpose='" << transpose() << std::hex << "' />" << std::endl;
	return xml.str();
}

void SequenceEntry::setSequenceLine( SequenceLine *newLine )
{
	printf("line before %p\n", line_);
	line_->moveEntryToNewLine( this, newLine );
	line_ = newLine;
	printf("line after %p\n", line_);
}

/**************************************************************************/
// SequenceLine
// represents one track/line in the sequencer

SequenceLine::SequenceLine( )
{
	patternSequence_ = 0;
}

SequenceLine::SequenceLine( PatternSequence * patSeq )
{
	patternSequence_ = patSeq;
}


SequenceLine::~ SequenceLine( )
{
	iterator it = begin();
	for ( it; it != end(); it++) delete it->second;
wasDeleted(this);
}

SequenceEntry* SequenceLine::createEntry( SinglePattern * pattern, double position )
{
	SequenceEntry* entry = new SequenceEntry(this);
	entry->setPattern(pattern);
	insert(value_type(position, entry));
	return entry;
}

void SequenceLine::insertEntry( SequenceEntry *entry )
{
	insert(value_type(entry->startPos(), entry));
}

void SequenceLine::moveEntryToNewLine( SequenceEntry *entry, SequenceLine *newLine )
{
	newLine->insertEntry( entry );
	iterator it = begin();
	for( ; it!= end(); ++it )
	{
		if ( it->second==entry )
		break;
	}
	erase( it ); // Removes entry from this SequenceLine, but doesn't delete it.
}

void SequenceLine::removeSinglePatternEntries( SinglePattern* pattern )
{
	iterator it = begin();
	while ( it != end() ) {
		SequenceEntry* entry = it->second;
		if ( entry->pattern() == pattern) {
			delete entry;
			erase(it++);
		} else it++;
	}
}

double SequenceLine::tickLength( ) const
{
	if (size() > 0 ) {
		return rbegin()->first + rbegin()->second->patternBeats();
	} else
	return 0;
}

PatternSequence * SequenceLine::patternSequence( )
{
	return patternSequence_;
}

void SequenceLine::MoveEntry(SequenceEntry* entry, double newpos)
{
	iterator iter = begin();
	for(; iter!= end(); ++iter)
	{
		if(iter->second==entry)
		break;
	}
	if(iter!=end())
	{
		double oldpos = iter->first;
		erase(iter);
		insert(value_type(newpos, entry));
	}
}

void SequenceLine::removeEntry( SequenceEntry * entry )
{
	iterator iter = begin();
	for(; iter!= end(); ++iter)
	{
		if(iter->second==entry)
		break;
	}
	if(iter!=end())
	{
		SequenceEntry* entry = iter->second;
		erase(iter);
		delete entry;
	}
}

const std::string & SequenceLine::name() const
{
	return name_;
}

void SequenceLine::setName(const std::string & newname)
{
	name_=newname;
}

std::string SequenceLine::toXml( ) const
{
	std::ostringstream xml;
	xml << "<seqline>" << std::endl;
	for(const_iterator it = begin(); it != end(); ++it) {
		SequenceEntry* entry = it->second;
		xml << entry->toXml( it-> first);
	}
	xml << "</seqline>" << std::endl;
	return xml.str();
}

/**************************************************************************/
// PatternSequence

PatternSequence::PatternSequence()
{
	setNumTracks(16);
}

PatternSequence::~PatternSequence()
{
	for(iterator it = begin(); it != end(); ++it)
		delete *it;
	for (GlobalIter it = globalEvents_.begin(); it != globalEvents_.end(); it++) delete it->second;
}

SequenceLine * PatternSequence::createNewLine( )
{
	SequenceLine* line = new SequenceLine(this);
	push_back(line);
	newLineCreated(line);
	return line;
}

SequenceLine* PatternSequence::insertNewLine( SequenceLine* selectedLine ) {
	SequenceLine* line = 0;
	iterator it = find(begin(), end(), selectedLine);
	if ( it != end() ) {
		line = new SequenceLine(this);
		insert( it,  line);
		newLineInserted(line,selectedLine);
	}
	return line;
}

void PatternSequence::removeLine( SequenceLine * line )
{
	iterator it = find(begin(), end(), line);
	if ( it != end() ) {
		erase(it);
		lineRemoved(line);
		delete line;
	}
}

PatternData* PatternSequence::patternData() {
	return &patternData_;
}

const PatternData & PatternSequence::patternData( ) const
{
	return patternData_;
}

/// returns the PatternLines that are active in the range [start, start+length).
///\param start start time in beats since playback begin.
///\param length length of the range. start+length is the last position (non inclusive)
///\return events : A multimap of lines (multimap of beatposition and PatternLine )
void PatternSequence::GetLinesInRange( double start, double length, std::multimap<double, PatternLine>& events ) 
{
	int seqlineidx = 1; // index zero reserved for live events (midi in, or pc keyb)
	// Iterate over each timeline of the sequence,
	for( iterator seqIt = begin(); seqIt != end(); ++seqIt )
	{
		SequenceLine *pSLine = *seqIt;
		// locate the "sequenceEntry"s which starts nearer to "start+length"
		SequenceLine::reverse_iterator sLineIt( pSLine->lower_bound(start+length) );
		// and iterate backwards to include any other that is inside the range [start,start+length)
		// (The UI won't allow more than one pattern for the same range in the same timeline, but 
		// this was left open in the player code)
		for(; sLineIt != pSLine->rend() && sLineIt->first + sLineIt->second->patternBeats() >= start; ++sLineIt )
		{
			// take the pattern,
			SinglePattern* pPat = sLineIt->second->pattern();
			double entryStart = sLineIt->first;
			float entryStartOffset  = sLineIt->second->startPos();
			float entryEndOffset  = sLineIt->second->endPos();
			float entryLength = entryEndOffset - entryStartOffset;
			double relativeStart = start - entryStart + entryStartOffset;
			
			SinglePattern::iterator patIt = pPat->lower_bound( std::min(relativeStart , (double)entryEndOffset)),
			patEnd = pPat->lower_bound( std::min(relativeStart+length,(double) entryEndOffset) );

			// and iterate through the lines that are inside the range
			for( ; patIt != patEnd; ++patIt)
				{
				PatternLine *thisline= &(patIt->second);
				PatternLine tmpline;

				std::map<int, PatternEvent>::iterator lineIt = thisline->notes().begin();
				// Since the player needs to differentiate between tracks of different SequenceEntrys, 
				// we generate a temporary PatternLine with a special column value.
				for( ;lineIt != thisline->notes().end() ;lineIt++)
				{
					tmpline.notes()[lineIt->first]=lineIt->second;
					tmpline.notes()[lineIt->first].setNote(tmpline.notes()[lineIt->first].note()+sLineIt->second->transpose() );
				}
				
				// finally add the PatternLine to the event map. The beat position is in absolute values from the playback start.
				tmpline.setSequenceTrack(seqlineidx);
				tmpline.tweaks()=thisline->tweaks();
				events.insert( SinglePattern::value_type( entryStart + patIt->first - entryStartOffset, tmpline ) );
				}
		}
		seqlineidx++;
	}
}

bool PatternSequence::getPlayInfo( SinglePattern* pattern, double start, double length, double & entryStart  ) const {
	entryStart = 0;
	PatternLine* searchLine = 0;

	// Iterate over each timeline of the sequence,
	for( const_iterator seqIt = begin(); seqIt != end(); ++seqIt )
	{
		SequenceLine *pSLine = *seqIt;
		// locate the "sequenceEntry"s which starts at "start+length"
		SequenceLine::reverse_iterator sLineIt( pSLine->upper_bound(start + length) );
		// and iterate backwards to include any other that is inside the range [start,start+length)
		// (The UI won't allow more than one pattern for the same range in the same timeline, but 
		// this was left open in the player code)
		for(; sLineIt != pSLine->rend() && sLineIt->first + sLineIt->second->patternBeats() >= start; ++sLineIt )
		{
			// take the pattern,

			SinglePattern* pPat = sLineIt->second->pattern();

			if ( pPat == pattern ) {
				entryStart = sLineIt->first;
				return true;
			}
		
		}
	}
	return false;
}

double PatternSequence::GetNextGlobalEvents(double start, double length, std::vector<GlobalEvent*>& globals, bool bInclusive)
{
	GlobalIter firstIt = ( bInclusive ?
				globalEvents_.lower_bound(start) :
				globalEvents_.upper_bound(start) );
	if(firstIt != globalEvents_.end() && firstIt->first < start+length)
	{
		double pos = firstIt->first;
		GlobalIter lastIt = globalEvents_.upper_bound(pos);
		for(; firstIt != lastIt; ++firstIt)
			globals.push_back(firstIt->second);
		return pos;
	}
	return start+length;
}

GlobalEvent * PatternSequence::createBpmChangeEntry( double position , float bpm)
{
	GlobalEvent* bpmEvent = new GlobalEvent(bpm);
	bpmEvent->setType(GlobalEvent::BPM_CHANGE);
	globalEvents_.insert(GlobalMap::value_type(position, bpmEvent));

	return bpmEvent;
}

void PatternSequence::moveGlobalEvent( GlobalEvent * entry, double newpos )
{
	GlobalIter iter = globalEvents_.begin();
	for(; iter!= globalEvents_.end(); ++iter)
	{
		if(iter->second==entry)
		break;
	}
	if(iter!=globalEvents_.end())
	{
		double oldpos = iter->first;
		globalEvents_.erase(iter);
		globalEvents_.insert(GlobalMap::value_type(newpos, entry) );
	}
}

double PatternSequence::globalTickPosition( GlobalEvent * event ) const
{
	// O(n) runtime here (multiplies from gui resize to O(n^2)!!!!
	// this is worse and could be avoided if Global Events have a tickPosition and we avoid instead that map
	GlobalMap::const_iterator iter = globalEvents_.begin();
	for(; iter!= globalEvents_.end(); ++iter)
	{
		if(iter->second==event)
		break;
	}
	if(iter!=globalEvents_.end())
	{
		return iter->first;
	}
	return 0;
}

const PatternSequence::GlobalMap & PatternSequence::globalEvents( )
{
	return globalEvents_;
}

void PatternSequence::removeSinglePattern( SinglePattern * pattern )
{
	for(iterator it = begin(); it != end(); ++it) {
		(*it)->removeSinglePatternEntries(pattern);
	}
	patternData_.removeSinglePattern(pattern);
}

void PatternSequence::removeAll( )
{
	for(iterator it = begin(); it != end(); ++it) {
		lineRemoved(*it);
		delete *it;
	}
	clear();
	patternData_.removeAll();
}

double PatternSequence::tickLength( ) const
{
	double max = 0;
	for(const_iterator it = begin(); it != end(); ++it) {
		SequenceLine* line = *it;
		max = std::max(line->tickLength(), max);
	}
	return max;
}

void PatternSequence::moveUpLine(SequenceLine* line) {
	iterator it = find( begin(), end(), line);
	if ( it != begin() ) {
		iterator prev = it;
		--prev;
		std::swap(*prev,*it);
		linesSwapped(*it,*prev);
	}
}

void PatternSequence::moveDownLine(SequenceLine* line) {
	iterator it = find( begin(), end(), line);
	if ( it != end() ) {
		iterator next=it;
		++next;
		if ( next != end() ) {
			std::swap(*it,*next);
			linesSwapped(*next,*it);
		}
	}
}

std::string PatternSequence::toXml( ) const
{
	std::ostringstream xml;
	xml << "<sequence>" << std::endl;
	for(const_iterator it = begin(); it != end(); ++it) {
		SequenceLine* line = *it;
		xml << line->toXml();
	}
	xml << "</sequence>" << std::endl;
	return xml.str();
}

}}
