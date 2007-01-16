/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "nfa.h"
#include "state.h"
#include <iostream>
#include <algorithm>

using namespace std;

namespace ngrs {

  Nfa::Nfa()
  {

  }


  Nfa::~Nfa()
  {
  }

  void Nfa::setS0( State * startState )
  {
    s0 = startState;
  }

  void Nfa::setA( std::vector< State * > acceptStates )
  {
    A = acceptStates;
  }

  void Nfa::addA( State * acceptState )
  {
    A.push_back(acceptState);
  }


  State * Nfa::state( int i )
  {
    return S.at(i);
  }

  bool Nfa::accept( const std::string & input )
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

  std::vector< State* > Nfa::epsilonFollows( const std::vector< State * > states )
  {
    vector<State*> newStates;
    for (vector<State*>::const_iterator it = states.begin(); it < states.end(); it++) {
      State* state = *it;
      vector<State*> e = state->E();
      newStates.insert(newStates.end(),e.begin(), e.end());
    }

    return newStates;
  }

  void Nfa::concat( Nfa * nfa )
  {
    for (vector<State*>::iterator it = A.begin(); it < A.end(); it++) {
      State* state = *it;
      state->addEpsilon(nfa->startState());
    }
    A = nfa->acceptStates();
  }

  State * Nfa::startState( )
  {
    return  s0;
  }

  std::vector< State * > & Nfa::acceptStates( )
  {
    return A;
  }

  void Nfa::star( )
  {
    for (vector<State*>::iterator it = A.begin(); it < A.end(); it++) {
      State* state = *it;
      s0->addEpsilon(state);
      state->addEpsilon(s0);
    }
  }


  vector< State * > & Nfa::getEpsilonStates( std::vector< State * > & actualStates )
  {
    closure.clear();
    for (std::vector<State*>::iterator it = actualStates.begin(); it < actualStates.end(); it++) {
      epsilonClosure(*it,closure);
    }
    return closure;
  }

  void Nfa::epsilonClosure( State * state, std::vector< State * > & eClosure )
  {
    // test if its already inserted else break;
    if (find(eClosure.begin(), eClosure.end(), state)==eClosure.end()) eClosure.push_back(state); else return;

    // test if state has an epsilon move
    vector<State*> states = state->E();
    for (vector<State*>::iterator it = states.begin(); it < states.end(); it++) {
      State* state = *it;
      epsilonClosure(state,eClosure);
    }
  }

  std::vector<State*> & Nfa::getFollowStates( std::vector< State*> & actualStates, char input )
  {
    followStates.clear();
    for (std::vector<State*>::iterator it = actualStates.begin(); it < actualStates.end(); it++) {
      State* state = *it;
      vector<State*> fStates = state->T(input);
      if (fStates.size()>0) {
        for (std::vector<State*>::iterator it = fStates.begin(); it < fStates.end(); it++) {
          State* state = *it;
          followStates = addToActual(state,followStates);
        }
      }
    }
    return followStates;
  }


  std::vector<State*> const &Nfa::addToActual( State * state, std::vector< State * > & actualStates)
  {
    std::vector<State*>::iterator it = find(actualStates.begin(), actualStates.end(), state);
    if (it == actualStates.end()) actualStates.push_back(state);
    return actualStates;
  }


  bool Nfa::testAccept( std::vector< State * > & actualStates )
  {
    for (std::vector<State*>::iterator it = actualStates.begin(); it<actualStates.end(); it++) {
      for (std::vector<State*>::iterator it1 = A.begin(); it1<A.end(); it1++) {
        if (*it1==*it) return true;
      }
    }
    return false;
  }

}
