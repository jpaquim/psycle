// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__SEQUENCE__INCLUDED
#define PSYCLE__CORE__SEQUENCE__INCLUDED
#pragma once

#include "pattern.h"

namespace psycle { namespace core {

		const int PatternEnd = -1;
	
		class SequenceLine;

		class SequenceEntry {
		public:
			SequenceEntry();
			SequenceEntry(SequenceLine* line);
			~SequenceEntry();

			// boost::signal<void (SequenceEntry*)> wasDeleted;
			double tickPosition() const;
			double tickEndPosition( ) const;
			void setPattern(Pattern* pattern);
			Pattern* pattern() { return pattern_; }
			Pattern* pattern() const { return pattern_; }
			float patternBeats() const { return pattern_->beats(); }
			SequenceLine* track() {return line_;}
			void setSequenceLine( SequenceLine *newLine );
			void setStartPos(float pos) { startPos_ = pos; }
			float startPos() const { return startPos_; }
			void setEndPos(float pos) { endPos_ = pos; }
			float endPos() const { return endPos_; }
			void setTranspose(int offset) { transpose_ = offset; }
			int transpose() const { return transpose_; }
			std::string toXml(double pos) const;

		private:
			/// the sequence timeline that the sequenceEntry belongs to
			SequenceLine* line_;
			/// the wrapped pattern
			Pattern* pattern_;
			/// here we can shrink the pattern of the entry
			float startPos_;
			/// endpos shrink (from begin of a pattern starting at 0)
			float endPos_;
			/// a transpose offset for the entry
			int transpose_;
		};

		class Sequence;

	class PSYCLE__CORE__DECL SequenceLine {
		public:
			SequenceLine();
			SequenceLine(Sequence* sequence);
			~SequenceLine();

			boost::signal<void (SequenceLine*)> wasDeleted;

			SequenceEntry* createEntry(Pattern* pattern, double position);
			void insertEntry(SequenceEntry *entry);
			void moveEntryToNewLine(SequenceEntry* entry, SequenceLine *newLine);
			void removePatternEntries(Pattern* pattern);
			void insertEntryAndMoveRest(SequenceEntry* entry, double pos);
			void moveEntries(SequenceEntry* start_entry, double delta);
			void removeSpaces(); // removes spaces between entries
			void clear();
			double tickLength() const;
			Sequence* sequence() { return sequence_; }
			void MoveEntry(SequenceEntry* entry, double newpos);
			void removeEntry(SequenceEntry* entry);
			const std::string& name() const { return name_; }
			void set_name(const std::string& name) {
				name_ = name;
			}
			std::string toXml() const;

			typedef std::multimap<double, SequenceEntry*>::iterator iterator;
			typedef std::multimap<double, SequenceEntry*>::const_iterator const_iterator;
			typedef std::multimap<double, SequenceEntry*>::const_reverse_iterator const_reverse_iterator;
			typedef std::multimap<double, SequenceEntry*>::reverse_iterator reverse_iterator;

			iterator begin() { return line_.begin(); }
			const_iterator begin() const { return line_.begin(); }
			iterator end() { return line_.end(); }
			const_iterator end() const { return line_.end(); }
			reverse_iterator rbegin() { return line_.rbegin(); }
			const_reverse_iterator rbegin() const { return line_.rbegin(); }
			reverse_iterator rend() { return line_.rend(); }
			const_reverse_iterator rend() const { return line_.rend(); }

			void insert(double pos, SequenceEntry* entry) {
				line_.insert(std::pair<double, SequenceEntry*>(pos, entry));
				entry->setSequenceLine(this);
			}

			iterator find(SequenceEntry* entry);
			void erase(iterator it) { line_.erase(it); }
			iterator lower_bound(double pos) { return line_.lower_bound(pos); }
			iterator upper_bound(double pos) { return line_.upper_bound(pos); }

			std::multimap<double, SequenceEntry*>::size_type size() const {
				return line_.size();
			}

			bool empty() const { return line_.empty(); }
			void SetSequence(Sequence* sequence) { sequence_ = sequence; }

		private:
			std::multimap<double, SequenceEntry*> line_;
			std::string name_;
			Sequence* sequence_;
	};

	class PSYCLE__CORE__DECL Sequence {
		friend class SequenceLine;
		public:
			Sequence();
			~Sequence();

			typedef std::vector<SequenceLine*>::iterator iterator;
			typedef std::vector<SequenceLine*>::const_iterator const_iterator;
			typedef std::vector<SequenceLine*>::const_reverse_iterator const_reverse_iterator;
			typedef std::vector<SequenceLine*>::reverse_iterator reverse_iterator;

			iterator begin() { return lines_.begin(); }
			const_iterator begin() const { return lines_.begin(); }
			iterator end() { return lines_.end(); }
			const_iterator end() const { return lines_.end(); }

			reverse_iterator rbegin() { return lines_.rbegin(); }
			const_reverse_iterator rbegin() const { return lines_.rbegin(); }
			reverse_iterator rend() { return lines_.rend(); }
			const_reverse_iterator rend() const { return lines_.rend(); }

			typedef std::vector<Pattern*>::iterator patterniterator;
			typedef std::vector<Pattern*>::const_iterator const_patterniterator;
			typedef std::vector<Pattern*>::const_reverse_iterator const_reverse_patterniterator;
			typedef std::vector<Pattern*>::reverse_iterator reverse_patterniterator;

			patterniterator patternbegin() { return patterns_.begin(); }
			const_patterniterator patternbegin() const { return patterns_.begin(); }
			patterniterator patternend() { return patterns_.end(); }
			const_patterniterator patternend() const { return patterns_.end(); }

			reverse_patterniterator patternrbegin() { return patterns_.rbegin(); }
			const_reverse_patterniterator patternrbegin() const { return patterns_.rbegin(); }
			reverse_patterniterator patternrend() { return patterns_.rend(); }
			const_reverse_patterniterator patternrend() const { return patterns_.rend(); }
			int numpatterns() { return patterns_.size(); }

			typedef std::multimap<double, PatternEvent*> GlobalMap;
			typedef GlobalMap::iterator GlobalIter;

			SequenceLine* createNewLine();
			boost::signal<void (SequenceLine*)> newLineCreated;
			SequenceLine* insertNewLine( SequenceLine* selectedLine );
			boost::signal<void (SequenceLine*, SequenceLine*)> newLineInserted; // new line, line it is inserted before
			
			void removeLine(SequenceLine* line);
			boost::signal<void (SequenceLine*)> lineRemoved;
			void removeAll();

			// heart of patternsequence
			void GetEventsInRange(double start, double length, std::vector<PatternEvent*>& events);
			void GetOrderedEvents(std::vector<PatternEvent*>& event_list);
			SequenceEntry* GetEntryOnPosition(SequenceLine* line, double tickPosition);
			void CollectEvent(const PatternEvent& command);
			int priority(const PatternEvent& cmd, int count) const;

			// playpos info
			bool getPlayInfo( Pattern* pattern, double start, double length, double & entryStart  ) const;
			void removePattern(Pattern* pattern);

			double tickLength() const;
			void moveDownLine(SequenceLine* line);
			void moveUpLine(SequenceLine* line);
			boost::signal<void (SequenceLine*, SequenceLine*)> linesSwapped;
			const int &numTracks() const { return numTracks_; }
			void setNumTracks(int newtracks)
			{
				///\todo: might be necessary to initialize mutedTrack and armedTrack after this.
				/// Also, trackArmedCount_ might need recalculation.
				numTracks_ = newtracks;
				mutedTrack_.resize(numTracks_);
				armedTrack_.resize(numTracks_);
			}

			int trackMuted(int track) const { assert(track<numTracks_); return mutedTrack_[track]; }
			void setMutedTrack(int track,bool value) { assert(track<numTracks_); mutedTrack_[track]=value; }
			std::vector<bool>& muted_tracks() { return mutedTrack_; }

			int trackArmed(int track) const { assert(track<numTracks_); return armedTrack_[track]; }
			void setArmedTrack(int track,bool value)
			{
				assert(track<numTracks_);
				if ( value != armedTrack_[track])
				{
					armedTrack_[track]=value;
					trackArmedCount_+=(value==false)?-1:1;
				}
			}

			std::string toXml() const;
			void Add(Pattern* pattern);
			void Remove(Pattern* pattern);
			Pattern* FindPattern(int id);
			Pattern* master_pattern() { return master_pattern_; }

			double max_beats() const;

		private:
			// sequencer structure
			std::vector<SequenceLine*> lines_;
			// pattern pool
			std::vector<Pattern*> patterns_;
			int numTracks_;
			std::vector<bool> mutedTrack_;
			/// The number of tracks Armed (enabled for record)
			int trackArmedCount_;
			/// Wether each of the tracks is armed (selected for recording data in)
			std::vector<bool> armedTrack_;

			std::multimap<double, std::multimap< int, PatternEvent > > events_;
			// masterPattern
			Pattern* master_pattern_;
	};
	

}}
#endif
