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
#include "nstate.h"

using namespace std;

NState::NState()
{
}


NState::~NState()
{
}

const std::vector< NState* > & NState::T( const char input )
{
  std::map<char,std::vector<NState*> >::iterator itr;
  if ( (itr = TMap.find(input)) != TMap.end() ) {
     return itr->second;
  } else 
  {
    return ndef;
  }
}

void NState::setSequence( const char label, const std::vector< NState* > & sequence )
{
  TMap[label] = sequence;
}

void NState::addFollowState( const char label, NState * state )
{
  std::map<char,std::vector<NState*> >::iterator itr;
  if ( (itr = TMap.find(label)) != TMap.end() ) {
     itr->second.push_back(state);
  } else 
  {
    vector<NState*> states;
    states.push_back(state);
    TMap[label] = states;
  }
}

void NState::setEpsilon( const std::vector< NState * > & sequence )
{
  epsilonSequence = sequence;
}

void NState::addEpsilon( NState * state )
{
  epsilonSequence.push_back(state);
}

const std::vector< NState * > & NState::E( )
{
  return epsilonSequence; 
}


