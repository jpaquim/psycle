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
#include <iostream>


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
  return S.at(i);
}

bool NNfa::accept( const std::string & input )
{
  runningStates.push_back(s0);

  for (std::string::const_iterator in = input.begin(); in < input.end(); in++)
  {
    char c = *in;
    runningStates = getEpsilonStates(runningStates);
    runningStates = getFollowStates(runningStates,c);

    if (runningStates.size()==0) return false;
  }

  return testAccept(runningStates);
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

void NNfa::concat( NNfa * nfa )
{
  for (vector<NState*>::iterator it = A.begin(); it < A.end(); it++) {
    NState* state = *it;
    state->addEpsilon(nfa->startState());
  }
  A = nfa->acceptStates();
}

NState * NNfa::startState( )
{
  return  s0;
}

std::vector< NState * > & NNfa::acceptStates( )
{
  return A;
}

void NNfa::star( )
{
  for (vector<NState*>::iterator it = A.begin(); it < A.end(); it++) {
   NState* state = *it;
    s0->addEpsilon(state);
    state->addEpsilon(s0);
  }
}


vector< NState * > & NNfa::getEpsilonStates( std::vector< NState * > & actualStates )
{
  closure.clear();
  for (std::vector<NState*>::iterator it = actualStates.begin(); it < actualStates.end(); it++) {
     epsilonClosure(*it,closure);
  }
  return closure;
}

void NNfa::epsilonClosure( NState * state, std::vector< NState * > & eClosure )
{
  // test if its already inserted else break;
  if (find(eClosure.begin(), eClosure.end(), state)==eClosure.end()) eClosure.push_back(state); else return;

  // test if state has an epsilon move
  vector<NState*> states = state->E();
  for (vector<NState*>::iterator it = states.begin(); it < states.end(); it++) {
    NState* state = *it;
    epsilonClosure(state,eClosure);
  }
}

std::vector<NState*> & NNfa::getFollowStates( std::vector< NState*> & actualStates, char input )
{
  followStates.clear();
  for (std::vector<NState*>::iterator it = actualStates.begin(); it < actualStates.end(); it++) {
     NState* state = *it;
     vector<NState*> fStates = state->T(input);
     if (fStates.size()>0) {
       for (std::vector<NState*>::iterator it = fStates.begin(); it < fStates.end(); it++) {
         NState* state = *it;
         followStates = addToActual(state,followStates);
       }
     }
  }
  return followStates;
}


std::vector<NState*> const &NNfa::addToActual( NState * state, std::vector< NState * > & actualStates)
{
  std::vector<NState*>::iterator it = find(actualStates.begin(), actualStates.end(), state);
  if (it == actualStates.end()) actualStates.push_back(state);
  return actualStates;
}


bool NNfa::testAccept( std::vector< NState * > & actualStates )
{
 for (std::vector<NState*>::iterator it = actualStates.begin(); it<actualStates.end(); it++) {
   for (std::vector<NState*>::iterator it1 = A.begin(); it1<A.end(); it1++) {
       if (*it1==*it) return true;
      }
  }
  return false;
}
