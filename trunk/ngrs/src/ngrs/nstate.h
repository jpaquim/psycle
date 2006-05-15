/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NSTATE_H
#define NSTATE_H

#include <string>
#include <map>
#include <vector>



/**
@author Stefan
*/
class NState{
public:
    NState();

    ~NState();

    const std::vector<NState*> & T(const char input);
    const std::vector<NState*> & E();
    void setSequence(const char label, const std::vector<NState*> & sequence);
    void addFollowState(const char label, NState* state);

    void setEpsilon(const std::vector<NState*> & sequence);
    void addEpsilon(NState* state);

private:

   std::vector<NState*> ndef;
   std::vector<NState*> epsilonSequence;
   std::map<char,std::vector<NState*> > TMap;

};

#endif
