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
#include "sequenceritemgui.h"
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

		class SequenceLine;

		class BpmChangeEvent {
			public:
					BpmChangeEvent();
					BpmChangeEvent(int bpm);
					~BpmChangeEvent();

					void setBpm( int bpm );
					int bpm() const;

			private:
					int bpm_;
    };

		class SequenceEntry {
		public:
			SequenceEntry();
			SequenceEntry(SequenceLine* line);
			~SequenceEntry();

			sigslot::signal1<SequenceEntry*> beforeDelete;

			void setPattern(SinglePattern* pattern);
			SinglePattern* pattern();
			SinglePattern* pattern() const;

			float patternBeats() const;

			SequenceLine* track() {return line_;}

		private:
			SequenceLine* line_;
			SinglePattern* pattern_;
		};

		class PatternSequence;

		class SequenceLine : public  std::map<double, SequenceEntry*>, public sigslot::has_slots<>
		{

		public:
			SequenceLine();
			SequenceLine(PatternSequence* patSeq);
			~SequenceLine();

			SequenceEntry* createEntry(SinglePattern* pattern, double position);

			double tickLength() const;

			PatternSequence* patternSequence();

			void MoveEntry(SequenceEntry* entry, double newpos);

			const std::string & name() const;
			void setName(const std::string & newname);

		private:

			std::string name_;
			PatternSequence* patternSequence_;

			void onDeletePattern(SinglePattern* pattern);
		};

		class PatternSequence : public std::vector<SequenceLine*>,public sigslot::has_slots<> {
		public:
			PatternSequence();

			~PatternSequence();

			SequenceLine* createNewLine();
			void GetLinesInRange( double start, double length, std::multimap<double, PatternLine>& events );

			//PatternData* patternData();
			BpmChangeEvent* createBpmChangeEntry(double position, int bpm);
			void MoveBpmChangeEntry(BpmChangeEvent* entry, double newpos);

			const std::map<double, BpmChangeEvent*> & bpmChanges();

		private:

			//PatternData patternData_; todo move patterndata to here
			std::map<double, BpmChangeEvent*> bpmChangeEvents;

		};
	}
}

#endif
