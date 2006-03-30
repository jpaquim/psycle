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
#include "nnfa.h"
#include "nstate.h"



using namespace std;

NNfa::NNfa()
{

}


NNfa::~NNfa()
{
}

void NNfa::setS0( NState * startState )
{
  s0 = startState;
}

void NNfa::setA( std::vector< NState * > acceptStates )
{
  A = acceptStates;
}

void NNfa::addA( NState * acceptState )
{
  A.push_back(acceptState);
}


NState * NNfa::state( int i )
{
  S.at(i);
}

bool NNfa::accept( const std::string & input )
{
  runningStates.push_back(s0);

  // test for acceptStates

  for (vector<NState*>::iterator it = runningStates.begin(); it < runningStates.end(); it++) {
       NState* state = *it;
       vector<NState*>::iterator fIt = find(A.begin(),A.end(),state);
       if (fIt < A.end()) return true;
    }

  // iterate over transition table

  vector<NState*> eFollows = epsilonFollows(runningStates);

  for (std::string::const_iterator in = input.begin(); in < input.end(); in++)
  {
    char c = *in;
    vector<NState*> newRunningStates;

    vector<NState*> eFollows = epsilonFollows(runningStates);
    runningStates.insert( runningStates.end(), eFollows.begin(), eFollows.end() );

    // test follow State except epsilon
    for (vector<NState*>::iterator it = runningStates.begin(); it < runningStates.end(); it++) {
      NState* state = *it;
      vector<NState*> sequenceT = state->T(c);
      newRunningStates.insert( newRunningStates.end(), sequenceT.begin(), sequenceT.end() );
    }

    // test for acceptStates

    for (vector<NState*>::iterator it = newRunningStates.begin(); it < newRunningStates.end(); it++) {
       NState* state = *it;
       vector<NState*>::iterator fIt = find(A.begin(),A.end(),state);
       if (fIt < A.end()) return true;
    }

    runningStates = newRunningStates;
  }
  return false;
}

std::vector< NState* > NNfa::epsilonFollows( const std::vector< NState * > states )
{
  vector<NState*> newStates;
  for (vector<NState*>::const_iterator it = states.begin(); it < states.end(); it++) {
    NState* state = *it;
    vector<NState*> e = state->E();
    newStates.insert(newStates.end(),e.begin(), e.end());
  }

 return newStates;
}
