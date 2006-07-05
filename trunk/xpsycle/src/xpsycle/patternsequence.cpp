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

// pattern Entry contains one ptr to a SinglePattern and the tickPosition for the absolute Sequencer pos

SequenceEntry::SequenceEntry( )
{
  pattern_ = 0;
}

SequenceEntry::~ SequenceEntry( )
{
}

void SequenceEntry::setPattern( SinglePattern * pattern )
{
  pattern_ = pattern;
}

SinglePattern * SequenceEntry::pattern( )
{
  return pattern_;
}

void SequenceEntry::setTickPosition( double tick )
{
  tickPosition_ = tick;
}

double SequenceEntry::tickPosition( ) const
{
  return tickPosition_;
}
// end of PatternEntry



// represents one track/line in the sequencer and contains a list of patternEntrys, wich holds a pointer and tickposition to a SinglePattern
SequenceLine::SequenceLine( )
{
}

SequenceLine::~ SequenceLine( )
{
  std::list<SequenceEntry*>::iterator it = line.begin();
  for ( it; it != line.end(); it++) delete *it;
}

SequenceEntry* SequenceLine::createEntry( SinglePattern * pattern, float position )
{
  SequenceEntry* entry = new SequenceEntry();
    entry->setPattern(pattern);
    entry->setTickPosition(position);

    line.push_back(entry);

  return entry;
}
//end of sequenceLine;



// PatternSequence
PatternSequence::PatternSequence()
{
}


PatternSequence::~PatternSequence()
{
  std::vector<SequenceLine*>::iterator it = lines.begin();
  for ( it; it != lines.end(); it++) delete *it;
}



SequenceLine * PatternSequence::createNewLine( )
{
  SequenceLine* line = new SequenceLine();
  lines.push_back(line);

  return line;
}





