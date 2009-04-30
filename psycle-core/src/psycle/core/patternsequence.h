// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PATTERN_SEQUENCE__INCLUDED
#define PSYCLE__CORE__PATTERN_SEQUENCE__INCLUDED
#pragma once

#include "singlepattern.h"

namespace psy { namespace core {

		const int PatternEnd = -1;

		class GlobalEvent {
			public:
					enum GlobalType {
					BPM_CHANGE,
					SET_VOLUME,
					SET_PANNING,
					JUMP_TO,
					SET_BYPASS,
					UNSET_BYPASS,
					SET_MUTE,
					UNSET_MUTE,
					LOOP_TO,
					NONE,
				};

				GlobalEvent();
				GlobalEvent(float parameter_);
				virtual ~GlobalEvent();
	
				void setType(GlobalType type);
				GlobalType type() const;

				void setParameter(float parameter);
				float parameter() const;
				void setTarget(int target);
				int target() const;
				void setTarget2(int target);
				int target2() const;

				private:
					float parameter_;
					int target_, target2_;
					GlobalType type_;
		};


		class SequenceLine;

		class SequenceEntry {
		public:

			SequenceEntry();
			SequenceEntry(SequenceLine* line);
			~SequenceEntry();

			boost::signal<void (SequenceEntry*)> wasDeleted;

			double tickPosition() const;
			double tickEndPosition( ) const;

			void setPattern(SinglePattern* pattern);
			SinglePattern* pattern() { return pattern_; }
			SinglePattern* pattern() const { return pattern_; }

			float patternBeats() const;

			SequenceLine* track() {return line_;}
			void setSequenceLine( SequenceLine *newLine );

			void setStartPos( float pos );
			float startPos() const;

			void setEndPos( float pos );
			float endPos() const;

			void setTranspose( int offset );
			int transpose() const;

			std::string toXml(double pos) const;


		private:

			/// the sequence timeline that the sequenceEntry belongs to
			SequenceLine* line_;
			/// the wrapped pattern
			SinglePattern* pattern_;
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

			SequenceEntry* createEntry(SinglePattern* pattern, double position);
			void insertEntry(SequenceEntry *entry);
			void moveEntryToNewLine(SequenceEntry *entry, SequenceLine *newLine);
			void removeSinglePatternEntries(SinglePattern* pattern);			
			void insertEntryAndMoveRest(SequenceEntry *entry, double pos);
			void removeSpaces(); // removes spaces between entries

			double tickLength() const;

			Sequence* patternSequence() { return sequence_; }

			void MoveEntry(SequenceEntry* entry, double newpos);			
			void removeEntry(SequenceEntry* entry);

			const std::string& name() const;
			void setName(const std::string & newname);

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

			void erase(iterator it) {
				line_.erase(it);
			}

			iterator lower_bound(double pos) {
				return line_.lower_bound(pos);
			}

			iterator upper_bound(double pos) {
				return line_.upper_bound(pos);
			}

			std::multimap<double, SequenceEntry*>::size_type size() const {
				return line_.size();
			}

			bool empty() const { return line_.empty(); }

		private:

			std::multimap<double, SequenceEntry*> line_;
			std::string name_;
			Sequence* sequence_;

	};

	class PSYCLE__CORE__DECL Sequence {
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

			typedef std::vector<SinglePattern*>::iterator patterniterator;
			typedef std::vector<SinglePattern*>::const_iterator const_patterniterator;
			typedef std::vector<SinglePattern*>::const_reverse_iterator const_reverse_patterniterator;
			typedef std::vector<SinglePattern*>::reverse_iterator reverse_patterniterator;

			patterniterator patternbegin() { return patterns_.begin(); }
			const_patterniterator patternbegin() const { return patterns_.begin(); }
			patterniterator patternend() { return patterns_.end(); }
			const_patterniterator patternend() const { return patterns_.end(); }

			reverse_patterniterator patternrbegin() { return patterns_.rbegin(); }
			const_reverse_patterniterator patternrbegin() const { return patterns_.rbegin(); }
			reverse_patterniterator patternrend() { return patterns_.rend(); }
			const_reverse_patterniterator patternrend() const { return patterns_.rend(); }

			int numpatterns() { return patterns_.size(); }

			typedef std::multimap<double, GlobalEvent*> GlobalMap;
			typedef GlobalMap::iterator GlobalIter;

			SequenceLine* createNewLine();
			boost::signal<void (SequenceLine*)> newLineCreated;
			SequenceLine* insertNewLine( SequenceLine* selectedLine );
			boost::signal<void (SequenceLine*, SequenceLine*)> newLineInserted; // new line, line it is inserted before
			
			void removeLine(SequenceLine* line);
			boost::signal<void (SequenceLine*)> lineRemoved;
			void removeAll();

			// heart of patternsequence
			void GetEventsInRange( double start, double length, std::vector<PatternEvent*>& events );
			void GetOrderedEvents(std::vector<PatternEvent*>& event_list);
			SequenceEntry* GetEntryOnPosition(SequenceLine* line, double tickPosition);
			void CollectEvent(const PatternEvent& command);
			int priority(const PatternEvent& cmd, int count) const;

			// playpos info

			bool getPlayInfo( SinglePattern* pattern, double start, double length, double & entryStart  ) const;

			void removeSinglePattern(SinglePattern* pattern);

			///populates globals with a list of the first row of global events between beatpositions start and start+length.
			///\param bInclusive whether to include events with positions of exactly start.
			///\return the beat position of the global events, or if there are none, start+length.
			double GetNextGlobalEvents(double start, double length, std::vector<GlobalEvent*>& globals, bool bInclusive);

			GlobalEvent* createBpmChangeEntry(double position, float bpm);
			void moveGlobalEvent(GlobalEvent* entry, double newpos);
			double globalTickPosition( GlobalEvent * event) const;
			const GlobalMap & globalEvents();

			double tickLength() const;

			void moveDownLine(SequenceLine* line);
			void moveUpLine(SequenceLine* line);
			boost::signal<void (SequenceLine*, SequenceLine*)> linesSwapped;

			const int &numTracks() const { return numTracks_; }
			void setNumTracks(int newtracks)
			{
				///\todo: might be necessary to initialize mutedTrack and armedTrack after this.
				/// Also, trackArmedCount_ might need recalculation.
				numTracks_ = newtracks; mutedTrack_.resize(numTracks_); armedTrack_.resize(numTracks_);
			}

			int trackMuted(int track) const { assert(track<numTracks_); return mutedTrack_[track]; }
			void setMutedTrack(int track,bool value) { assert(track<numTracks_); mutedTrack_[track]=value; }

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

			void Add(SinglePattern* pattern);
			void Remove(SinglePattern* pattern);
			SinglePattern* FindPattern(int id);

		private:
			// sequencer structure
			std::vector<SequenceLine*> lines_;
			// pattern pool
			std::vector<SinglePattern*> patterns_;

			int numTracks_;
			std::vector<bool> mutedTrack_;
			/// The number of tracks Armed (enabled for record)
			int trackArmedCount_;
			/// Wether each of the tracks is armed (selected for recording data in)
			std::vector<bool> armedTrack_;

			GlobalMap globalEvents_;

			std::multimap<double, std::multimap< int, PatternEvent > > events_;
	};
	typedef Sequence PatternSequence;

}}
#endif
