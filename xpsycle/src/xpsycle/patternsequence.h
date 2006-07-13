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
#include <vector>
#include <list>


/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{

		class SequenceLine;

		class SequenceEntry {
		public:
			SequenceEntry();
			SequenceEntry(SequenceLine* line);
			~SequenceEntry();

			sigslot::signal1<SequenceEntry*> beforeDelete;

			void setTickPosition(double tick);
			double tickPosition() const;
			virtual double beatLength() const;

			virtual void setPlayIteratorToBegin();
			virtual bool prepare(double masterBeatBeginposition, double masterBeatEndPosition, std::list<std::pair<double,PatternLine* > > & tempPlayLines );

		private:

			SequenceLine* line_;
			double tickPosition_;

		};



		class PatternSequenceEntry : public SequenceEntry {
		public:
			PatternSequenceEntry();
			PatternSequenceEntry(SequenceLine* line);

			void setPattern(SinglePattern* pattern);
			SinglePattern* pattern();
			SinglePattern* pattern() const;

			virtual double beatLength() const;

			virtual void setPlayIteratorToBegin();
			std::list<PatternLine>::iterator & playIterator();
			std::list<PatternLine>::iterator begin();
			std::list<PatternLine>::iterator end();

			virtual bool prepare(double masterBeatBeginposition, double masterBeatEndPosition, std::list<std::pair<double,PatternLine* > > & tempPlayLines );


		private:
			SinglePattern* pattern_;
			std::list<PatternLine>::iterator playIterator_;
		};

		class PatternSequence;

		class SequenceLine : public  std::list<SequenceEntry*>, public sigslot::has_slots<>
		{

		public:
			SequenceLine();
			SequenceLine(PatternSequence* patSeq);
			~SequenceLine();

			SequenceEntry* createEntry(SinglePattern* pattern, double position);

			double tickLength() const;

			PatternSequence* patternSequence();

		private:

			PatternSequence* patternSequence_;

			void onDeletePattern(SinglePattern* pattern);

		};

		class PatternSequence : public std::list<SequenceEntry*>,public sigslot::has_slots<> {
		public:
			PatternSequence();

			~PatternSequence();

			SequenceLine* createNewLine();

			void onDeletePattern(SinglePattern* pattern);

			const std::vector<SequenceLine*> & lines() const;

		private:

			std::vector<SequenceLine*> lines_;

		};

	}
}

#endif
