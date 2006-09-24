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
#ifndef PATTERNSEQUENCE_H
#define PATTERNSEQUENCE_H

#include "singlepattern.h"
#include "patterndata.h"
#include <vector>
#include <list>
#include <string>


/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{

		const int PatternEnd = -1;

		class GlobalEvent
		{
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

			double tickPosition() const;

			void setPattern(SinglePattern* pattern);
			SinglePattern* pattern();
			SinglePattern* pattern() const;

			float patternBeats() const;

			SequenceLine* track() {return line_;}

			void setStartPos( float pos );
			float startPos() const;

			void setEndPos( float pos );
			float endPos() const;

			void setTranspose( int offset );
			int transpose() const;

			std::string toXml(double pos) const;

		private:

			SequenceLine* line_; 			// the sequence track , the sequence belongs to
			SinglePattern* pattern_; 	// the wrapped pattern
			float startPos_;					// here we can shrink the pattern of the entry
			float endPos_;						// endpos shrink (from begin of a pattern starting at 0)
			int transpose_;						// a transpose offset for the entry

			void init();

		};

		class PatternSequence;

		class SequenceLine : public  std::multimap<double, SequenceEntry*>
		{

		public:
			SequenceLine();
			SequenceLine(PatternSequence* patSeq);
			~SequenceLine();

			SequenceEntry* createEntry(SinglePattern* pattern, double position);
			void removeSinglePatternEntries(SinglePattern* pattern);

			double tickLength() const;

			PatternSequence* patternSequence();

			void MoveEntry(SequenceEntry* entry, double newpos);
			void removeEntry(SequenceEntry* entry);

			const std::string & name() const;
			void setName(const std::string & newname);

			std::string toXml() const;

		private:

			std::string name_;
			PatternSequence* patternSequence_;

		};

		class PatternSequence : public std::vector<SequenceLine*> {
		public:
			PatternSequence();
			~PatternSequence();

			typedef std::multimap<double, GlobalEvent*> GlobalMap;
			typedef GlobalMap::iterator GlobalIter;

			SequenceLine* createNewLine();
			SequenceLine* insertNewLine( SequenceLine* selectedLine );
			
			void removeLine(SequenceLine* line);
			void removeAll();

			// heart of patternsequence
			void GetLinesInRange( double start, double length, std::multimap<double, PatternLine>& events );

			// playpos info

			bool getPlayInfo( SinglePattern* pattern, double start, double length, double & entryStart  ) const;

			PatternData* patternData();
			const PatternData & patternData() const;

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

			std::string toXml() const;

		private:

			PatternData patternData_;

			GlobalMap globalEvents_;

		};
	}
}

#endif
