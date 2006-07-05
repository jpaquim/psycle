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
#include <vector>
#include <list>

/**
@author Stefan Nattkemper
*/

class SequenceEntry {
  public:
     SequenceEntry();
     ~SequenceEntry();

     void setPattern(SinglePattern* pattern);
     SinglePattern* pattern();

     void setTickPosition(double tick);
     double tickPosition() const;

  private:

     SinglePattern* pattern_;
     float tickPosition_;

};

class SequenceLine {

public:
   SequenceLine();
   ~SequenceLine();

   SequenceEntry* createEntry(SinglePattern* pattern, float position);


private:

   std::list<SequenceEntry*> line;

};


class PatternSequence{
public:
    PatternSequence();

    ~PatternSequence();

    SequenceLine* createNewLine();


private:

    std::vector<SequenceLine*> lines;


};

#endif
