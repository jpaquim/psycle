// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "patternsequence.h"

#include "commands.h"
#include <sstream>

namespace psycle { namespace core {

		/**************************************************************************/
		// SequenceEntry
		// pattern Entry contains one ptr to a Pattern and the tickPosition for the absolute Sequencer pos

		SequenceEntry::SequenceEntry() 
		:
			line_(0),
			pattern_(0),
			startPos_(0),
			endPos_(PatternEnd),
			transpose_(0)
		{}

		SequenceEntry::SequenceEntry(SequenceLine * line)
		:
			line_(line),
			pattern_(0),
			startPos_(0),
			endPos_(PatternEnd),
			transpose_(0)
		{}

		SequenceEntry::~SequenceEntry() {
			// wasDeleted(this);
		}

		void SequenceEntry::setPattern(Pattern * pattern) {
			pattern_ = pattern;
			startPos_ = 0;
			endPos_   = pattern->beats();
		}

		float SequenceEntry::patternBeats() const {
			return pattern_->beats();
		}

		double SequenceEntry::tickPosition() const {
			SequenceLine::iterator iter = line_->begin();
			for(; iter!= line_->end(); ++iter) {
				if(iter->second==this)
				break;
			}
			if(iter!=line_->end()) {
				return iter->first;
			}
			return 0;
		}

		double SequenceEntry::tickEndPosition() const {
			return tickPosition() + patternBeats();
		}

		void SequenceEntry::setTranspose(int offset) {
			transpose_ = offset;
		}
	
		int SequenceEntry::transpose() const {
			return transpose_;
		}

		void SequenceEntry::setStartPos(float pos) {
			startPos_ = pos;
		}

		float SequenceEntry::startPos() const {
			return startPos_;
		}

		void SequenceEntry::setEndPos(float pos) {
			endPos_ = pos;
		}

		float SequenceEntry::endPos() const {
			return endPos_;
		}

		std::string SequenceEntry::toXml(double pos) const {
			std::ostringstream xml;
			xml << "<seqentry pos='" << pos << "' patid='" << pattern()->id() << std::hex << "' "  << "start='" << startPos() << "' end='" << endPos() << "' " << "transpose='" << transpose() << std::hex << "' />" << std::endl;
			return xml.str();
		}

		void SequenceEntry::setSequenceLine(SequenceLine* newLine) {
			line_->moveEntryToNewLine( this, newLine );
			line_ = newLine;
		}

		/**************************************************************************/
		// SequenceLine
		// represents one track/line in the sequencer

		SequenceLine::SequenceLine()
		: sequence_()
		{}

		SequenceLine::SequenceLine(Sequence * sequence)
		: sequence_(sequence)
		{}


		SequenceLine::~SequenceLine() {
			iterator it = begin();
			for (it; it != end(); ++it)
				delete it->second;
			//wasDeleted(this);
		}

		SequenceEntry* SequenceLine::createEntry(Pattern * pattern, double position) {
			SequenceEntry* entry = new SequenceEntry(this);
			entry->setPattern(pattern);
			line_.insert(std::pair<double, SequenceEntry*>(position, entry));
			return entry;
		}

		void SequenceLine::insertEntry(SequenceEntry *entry) {
			line_.insert(std::pair<double, SequenceEntry*>(entry->startPos(), entry));
		}

		void SequenceLine::insertEntryAndMoveRest(SequenceEntry *entry, double pos) {
			std::multimap<double, SequenceEntry*> old_line_ = line_;
			line_.clear();
			std::multimap<double, SequenceEntry*>::iterator it = old_line_.begin();
			bool inserted = false;
			double last_pos = 0;
			for ( ; it != old_line_.end(); ++it ) {
				if ( it->first < pos ) {
					line_.insert(std::pair<double, SequenceEntry*>(it->first, it->second));
				} else  {
					if ( !inserted ) {
						line_.insert(std::pair<double, SequenceEntry*>(pos, entry));
						inserted = true;
					}
					double move = entry->patternBeats();
					line_.insert(std::pair<double, SequenceEntry*>(it->first + move, it->second));
				}
			}
			if ( !inserted ) {
				line_.insert(std::pair<double, SequenceEntry*>(pos, entry));
			}
		}

		void SequenceLine::moveEntries(SequenceEntry* start_entry, double delta) {
			std::multimap<double, SequenceEntry*> old_line_ = line_;
			line_.clear();
			bool inserted = false;
			std::multimap<double, SequenceEntry*>::iterator it = old_line_.begin();
			for ( ; it != old_line_.end(); ++it ) {
				if (it->second != start_entry && !inserted) {
					line_.insert(std::pair<double, SequenceEntry*>(it->first, it->second));
				} else {
					line_.insert(std::pair<double, SequenceEntry*>(it->first + delta, it->second));
					inserted = true;
				}
			}
		}

		SequenceLine::iterator SequenceLine::find(SequenceEntry* entry) {
			std::multimap<double, SequenceEntry*>::iterator it = line_.begin();
			for ( ; it != line_.end(); ++it ) {
				if (it->second == entry)
					return it;
			}
			return line_.end();
		}

		void SequenceLine::removeSpaces()
		{
			std::multimap<double, SequenceEntry*>::iterator it = begin();
			std::multimap<double, SequenceEntry*>::iterator next_it;
			double pos = 0.0;
			while ( it != end() ) {
				double diff = it->first - pos;
				pos = it->second->tickEndPosition() - diff;
				if (diff > 0) {
					next_it = it;
					next_it++;
					MoveEntry(it->second, it->first - diff);
					it = next_it;
				} else {
					++it;
				}
			}
		}

		void SequenceLine::moveEntryToNewLine(SequenceEntry *entry, SequenceLine *newLine) {
			newLine->insertEntry(entry);
			iterator it = begin();
			for(; it!= end(); ++it) {
				if (it->second==entry)
				break;
			}
			line_.erase(it); // Removes entry from this SequenceLine, but doesn't delete it.
		}

		void SequenceLine::removePatternEntries( Pattern* pattern ) {
			iterator it = begin();
			while ( it != end() ) {
				SequenceEntry* entry = it->second;
				if ( entry->pattern() == pattern) {
					delete entry;
					line_.erase(it++);
				} else it++;
			}
		}

		double SequenceLine::tickLength() const {
			if (size() > 0) {
				return rbegin()->first + rbegin()->second->patternBeats();
			} else {
				return 0;
			}
		}

		void SequenceLine::MoveEntry(SequenceEntry* entry, double newpos) {
			iterator iter = begin();
			for(; iter!= end(); ++iter) {
				if(iter->second==entry)
				break;
			}
			if(iter!=end()) {
				erase(iter);
				line_.insert(std::pair<double, SequenceEntry*>(newpos, entry));
			}
		}

		void SequenceLine::removeEntry(SequenceEntry* entry) {
			iterator iter = begin();
			for(; iter!= end(); ++iter) {
				if(iter->second==entry)
					break;
			}
			if(iter!=end()) {
				SequenceEntry* entry = iter->second;
				line_.erase(iter);
				sequence_->last_entry_ = 0;
				delete entry;
			}
		}

		void SequenceLine::clear() {
			if (sequence_)
				sequence_->last_entry_ = 0;
			iterator it = begin();
			for (it; it != end(); ++it)
				delete it->second;
			line_.clear();
		}

		std::string SequenceLine::toXml() const {
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
		// Sequence

		Sequence::Sequence() 
			: last_entry_(0)
		{
			setNumTracks(16);
			// create global master line with an entry that keeps the master pattern
			SequenceLine* master_line_ = createNewLine();
			master_pattern_ = new Pattern();
			// make pattern length endless
			master_pattern_->timeSignatures().clear();
			master_pattern_->timeSignatures().push_back(TimeSignature(std::numeric_limits<double>::max()));
			master_pattern_->setName("master");
			Add(master_pattern_);
			master_line_->createEntry(master_pattern_, 0);
		}

		Sequence::~Sequence() {
			for (iterator it = begin(); it != end(); ++it) {
				delete *it;
			}
			std::vector<Pattern*>::iterator pat_it = patterns_.begin();
			for(; pat_it != patterns_.end(); ++pat_it) {
				delete *pat_it;
			}
		}

		SequenceLine* Sequence::createNewLine() {
			SequenceLine* line = new SequenceLine(this);
			lines_.push_back(line);
			newLineCreated(line);
			return line;
		}

		SequenceLine* Sequence::insertNewLine(SequenceLine* selectedLine) {
			SequenceLine* line = 0;
			iterator it = find(begin(), end(), selectedLine);
			if ( it != end() ) {
				line = new SequenceLine(this);
				lines_.insert( it,  line);
				newLineInserted(line,selectedLine);
			}
			return line;
		}
		
		Pattern* Sequence::FindPattern(int id) {
			patterniterator it = patterns_.begin();
			for ( ; it != patterns_.end(); ++it) {
				if ((*it)->id() == id)
					return *it;
			}
			return 0;
		}

		void Sequence::removeLine(SequenceLine * line) {
			iterator it = find(begin(), end(), line);
			if ( it != end() ) {
				lines_.erase(it);
				lineRemoved(line);
				last_entry_ = 0;
				delete line;
			}
		}

		/// returns the PatternEvents that are active in the range [start, start+length).
		///\param start start time in beats since playback begin.
		///\param length length of the range. start+length is the last position (non inclusive)
		///\return events : A vector of sorted events
		void Sequence::GetEventsInRange(double start, double length, std::vector<PatternEvent*>& events)  {
			events_.clear();
			int seqlineidx = 1; // index zero reserved for live events (midi in, or pc keyb)
			// Iterate over each timeline of the sequence
			iterator seqIt = begin();
			for( ; seqIt != end(); ++seqIt ) {
				SequenceLine *pSLine = *seqIt;
				// locate the "sequenceEntry"s which starts nearer to "start+length"
				SequenceLine::reverse_iterator sLineIt( pSLine->lower_bound(start+length) );
				// and iterate backwards to include any other that is inside the range [start,start+length)
				// (The UI won't allow more than one pattern for the same range in the same timeline, but 
				// this was left open in the player code)
				bool worked = false;
				for(; sLineIt != pSLine->rend() && sLineIt->first + sLineIt->second->patternBeats() >= start; ++sLineIt ) {
					// take the pattern,
					Pattern* pPat = sLineIt->second->pattern();
					worked = true;
					last_entry_ = sLineIt->second;
					double entryStart = sLineIt->first;
					float entryStartOffset  = sLineIt->second->startPos();
					float entryEndOffset  = sLineIt->second->endPos();
					double relativeStart = start - entryStart + entryStartOffset;
			
					Pattern::iterator patIt = pPat->lower_bound( std::min(relativeStart , (double)entryEndOffset)),
					patEnd = pPat->lower_bound( std::min(relativeStart+length,(double) entryEndOffset) );

					// and iterate through the lines that are inside the range
					for( ; patIt != patEnd; ++patIt) {
						PatternEvent& ev= patIt->second;
						ev.set_sequence(seqlineidx);
						ev.set_time_offset(entryStart + patIt->first - entryStartOffset);
						CollectEvent(ev);
						#if 0 ///\todo
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
							events.insert( Pattern::value_type( entryStart + patIt->first - entryStartOffset, tmpline ) );
						#endif
						}
				}
				if (!worked)
					last_entry_ = 0;
				++seqlineidx;
			}
			GetOrderedEvents(events);
			// assert test if sorted correct
			std::vector<PatternEvent*>::iterator it = events.begin();
			double old_pos = 0;
			bool has_note = 0;
			for ( ; it != events.end(); ++it ) {
				PatternEvent* cmd = *it;
				double pos = cmd->time_offset();
				if (old_pos != pos)
					has_note = 0;
				old_pos = pos;
				if (cmd->note() == notetypes::tweak_slide) {
					assert(!has_note);
				} else if (cmd->note() == notetypes::tweak) {
					assert(!has_note);
				} else {
					has_note = 1;
					// note
				}
			}
		}

		SequenceEntry* Sequence::GetEntryOnPosition(SequenceLine* line, double pos) {
			for (SequenceLine::iterator it = line->begin(); it != line->end(); ++it) {
				if (pos >= it->second->tickPosition() && 
					pos < it->second->tickEndPosition()
					) {
					return it->second;
				}
			}
			return 0;
		}

		int Sequence::priority(const PatternEvent& cmd, int count) const {
			int p = 8;
			if (cmd.IsGlobal()) {
				p = 0;
			} else
			if (cmd.note() == notetypes::tweak_slide) {
				p = 1;
			} else if (cmd.note() == notetypes::tweak) {
				p = 2;
			} else {
				p = 3;
			}
			return p;
		}

		void Sequence::CollectEvent(const PatternEvent& command) {
			assert(command.time_offset() >= 0);
			double delta_frames = command.time_offset();
			std::multimap<double, std::multimap<int, PatternEvent > >::iterator it;
			it = events_.find(delta_frames);
			if ( it == events_.end() ) {
				std::multimap<int, PatternEvent> map;
				map.insert(std::pair<int, PatternEvent>(priority(command,0), command))->second.set_time_offset(delta_frames);
				events_.insert(std::pair<double, std::multimap< int, PatternEvent > >(delta_frames, map ) );
			} else {
				std::multimap<int, PatternEvent>& map = it->second;
				map.insert(std::pair<int, PatternEvent>( priority(command, map.size()), command))->second.set_time_offset(delta_frames);
			}
		}

		void Sequence::GetOrderedEvents(std::vector<PatternEvent*>& event_list) {
			std::multimap<double, std::multimap< int, PatternEvent > >::iterator event_it = events_.begin();
			for ( ; event_it != events_.end(); ++event_it ) {
				std::multimap< int, PatternEvent>& map = event_it->second;
				std::multimap< int, PatternEvent>::iterator map_it = map.begin();
				for ( ; map_it != map.end(); ++map_it ) {
					PatternEvent& command = map_it->second;
					event_list.push_back( &command );
				}
			}
		}

		bool Sequence::getPlayInfo(Pattern* pattern, double start, double length, double& entryStart) const {
			#if 0 ///\todo
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
						Pattern* pPat = sLineIt->second->pattern();
						if ( pPat == pattern ) {
							entryStart = sLineIt->first;
							return true;
						}
					}
				}
			#endif
			return false;
		}

		void Sequence::removePattern(Pattern* pattern) {
			for(iterator it = begin(); it != end(); ++it) {
				(*it)->removePatternEntries(pattern);
			}
			Remove(pattern);
		}

		void Sequence::removeAll( ) {
			for(iterator it = begin(); it != end(); ++it) {
				lineRemoved(*it);
				delete *it;
			}
			lines_.clear();
			std::vector<Pattern*>::iterator pat_it = patterns_.begin();
			for(; pat_it != patterns_.end(); ++pat_it) {
				delete *pat_it;
			}
			patterns_.clear();
			last_entry_ = 0;

			// create global master line with an entry that keeps the master pattern
			SequenceLine* master_line_ = createNewLine();
			master_pattern_ = new Pattern();
			// make pattern length endless
			master_pattern_->timeSignatures().clear();
			master_pattern_->timeSignatures().push_back(TimeSignature(std::numeric_limits<double>::max()));
			master_pattern_->setName("master");
			Add(master_pattern_);
			master_line_->createEntry(master_pattern_, 0);
		}

		double Sequence::tickLength() const {
			double max = 0;
			for(const_iterator it = begin(); it != end(); ++it) {
				SequenceLine* line = *it;
				max = std::max(line->tickLength(), max);
			}
			return max;
		}

		void Sequence::moveUpLine(SequenceLine* line) {
			iterator it = find( begin(), end(), line);
			if ( it != begin() ) {
				iterator prev = it;
				--prev;
				std::swap(*prev,*it);
				linesSwapped(*it,*prev);
			}
		}

		void Sequence::moveDownLine(SequenceLine* line) {
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

		std::string Sequence::toXml() const {
			std::ostringstream xml;
			xml << "<sequence>" << std::endl;
			for(const_iterator it = begin(); it != end(); ++it) {
				SequenceLine* line = *it;
				xml << line->toXml();
			}
			xml << "</sequence>" << std::endl;
			return xml.str();
		}

		void Sequence::Add(Pattern* pattern) {
			assert(pattern);
			patterns_.push_back(pattern);
		}
	
		void Sequence::Remove(Pattern* pattern) {
			///\todo
		}

}}
