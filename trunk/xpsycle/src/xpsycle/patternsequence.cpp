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
#include "patternsequence.h"
#include <iostream>

namespace psycle
{
	namespace host
	{


		BpmChangeEvent::BpmChangeEvent( )
		{
			bpm_ = 125.0f;
		}

		BpmChangeEvent::BpmChangeEvent( float bpm )
		{
			bpm_ = bpm;
		}

		BpmChangeEvent::~ BpmChangeEvent( )
		{
		}

		void BpmChangeEvent::setBpm( float bpm )
		{
			bpm_ = bpm;
		}

		float BpmChangeEvent::bpm( ) const
		{
			return bpm_;
		}


		// pattern Entry contains one ptr to a SinglePattern and the tickPosition for the absolute Sequencer pos

		SequenceEntry::SequenceEntry( )
		{
			line_ = 0;
			pattern_ = 0;
		}

		SequenceEntry::SequenceEntry( SequenceLine * line )
		{
			pattern_ = 0;
			line_ = line;
		}

		SequenceEntry::~ SequenceEntry( )
		{
			beforeDelete.emit(this);
		}

		void SequenceEntry::setPattern( SinglePattern * pattern )
		{
			pattern_ = pattern;
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

		// end of PatternEntry

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
		}

		SequenceEntry* SequenceLine::createEntry( SinglePattern * pattern, double position )
		{
			SequenceEntry* entry = new SequenceEntry(this);
			entry->setPattern(pattern);
			insert(value_type(position, entry));
			entry->pattern()->beforeDelete.connect(this,&SequenceLine::onDeletePattern);

			return entry;
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

		void SequenceLine::onDeletePattern( SinglePattern * pattern )
		{
			// todo rework deleting at all!
			for (iterator it = begin(); it != end(); it++) {
				if ( it->second->pattern() == pattern) {
					erase(it);
					break;
				}
			}
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

		const std::string & SequenceLine::name() const
		{
			return name_;
		}

		void SequenceLine::setName(const std::string & newname)
		{
			name_=newname;
		}
		//end of sequenceLine;



		// PatternSequence
		PatternSequence::PatternSequence()
		{
			std::map<double, BpmChangeEvent*>::iterator it = bpmChangeEvents.begin();
			for ( it; it != bpmChangeEvents.end(); it++) delete it->second;
		}


		PatternSequence::~PatternSequence()
		{
			for(iterator it = begin(); it != end(); ++it)
				delete *it;
		}


		SequenceLine * PatternSequence::createNewLine( )
		{
			SequenceLine* line = new SequenceLine(this);
			push_back(line);
			return line;
		}

//todo move pattern data to here
//		PatternData* PatternSequence::patternData() {
//			return &patternData_;
//		}

		void PatternSequence::GetLinesInRange( double start, double length, std::multimap<double, PatternLine>& events ) 
		{
			for( iterator seqIt = this->begin(); seqIt != this->end(); ++seqIt )
			{
				SequenceLine *pTrack = *seqIt;

				typedef std::map<double, SequenceEntry*>::reverse_iterator RTrackIt;

				RTrackIt trackIt = (RTrackIt)( pTrack->lower_bound(start+length) );
				for(; trackIt != pTrack->rend() && trackIt->first + trackIt->second->patternBeats() >= start; ++trackIt )
				{
					SinglePattern* pPat = trackIt->second->pattern();
					std::map<double, PatternLine>::iterator patIt   = pPat->lower_bound( start-trackIt->first ),
					patEnd = pPat->lower_bound( start+length-trackIt->first );
					for( ; patIt != patEnd; ++patIt)
						events.insert( std::pair<const double, PatternLine>( patIt->first + trackIt->first, patIt->second ) );
				}
			}
		}

		double PatternSequence::GetNextGlobalEvents(double start, double length, std::vector<GlobalEvent*>& globals, bool bInclusive)
		{
			std::map<double, BpmChangeEvent*>::iterator iter
				= ( bInclusive ?
				  bpmChangeEvents.lower_bound(start) :
				  bpmChangeEvents.upper_bound(start) );

			if(iter != bpmChangeEvents.end() && iter->first < start+length)
			{
				GlobalEvent* bpmchange = new GlobalEvent(iter->second->bpm());
				bpmchange->type=GlobalEvent::BPM_CHANGE;
				globals.push_back(bpmchange);
				return iter->first;
			}
			return start+length;
		}

		BpmChangeEvent * PatternSequence::createBpmChangeEntry( double position , float bpm)
		{
			BpmChangeEvent* bpmEvent = new BpmChangeEvent(bpm);
			bpmChangeEvents[position] = bpmEvent;

			return bpmEvent;
		}

		void PatternSequence::MoveBpmChangeEntry( BpmChangeEvent * entry, double newpos )
		{
			std::map<double, BpmChangeEvent*>::iterator iter = bpmChangeEvents.begin();
			for(; iter!= bpmChangeEvents.end(); ++iter)
			{
				if(iter->second==entry)
				break;
			}
			if(iter!=bpmChangeEvents.end())
			{
				double oldpos = iter->first;
				bpmChangeEvents.erase(iter);
				bpmChangeEvents[newpos] = entry;
			}

		}

		const std::map< double, BpmChangeEvent * > & PatternSequence::bpmChanges( )
		{
			return bpmChangeEvents;
		}

	} // end of host namespace
} // end of psycle namespace


