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

PatternSequenceRow::PatternSequenceRow( )
{
}

PatternSequenceRow::PatternSequenceRow( int col, SinglePattern * pattern )
{
  colMap[col] = pattern;
}

PatternSequenceRow::~ PatternSequenceRow( )
{
}


PatternSequence::PatternSequence()
{
}


PatternSequence::~PatternSequence()
{
}

void PatternSequence::add( int col, int tick, SinglePattern* pattern )
{
   // tick means here the row number
   std::map<int,PatternSequenceRow>::iterator itr;
   if ( (itr = rows.find( tick )) == rows.end() ) {
       rows[tick] = PatternSequenceRow(col, pattern);
   } else {
       itr->second.add(col, pattern);
   }
}

void PatternSequenceRow::add( int col, SinglePattern * pattern )
{
  colMap[col] = pattern;
}

SinglePattern * PatternSequenceRow::colAt( int index )
{
  std::map<int, SinglePattern*>::iterator itr;
  if ( (itr = colMap.find( index )) == colMap.end() ) {
    return 0;
  } else {
    return itr->second;
  }
}





