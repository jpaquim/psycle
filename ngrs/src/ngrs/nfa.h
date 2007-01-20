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
#ifndef NFA_H
#define NFA_H

#include "state.h"
#include <vector>

namespace ngrs {

  class State;

  /**
  @author Stefan
  */
  class Nfa{
  public:
    Nfa();

    ~Nfa();

    void setS0(State* startState);
    void setA (std::vector<State*> acceptStates);
    void addA (State* acceptState);

    State* state(int i);
    State* startState();
    std::vector<State*> & acceptStates();


    bool accept(const std::string & input);

    void concat( Nfa * nfa );
    void star();

  private:

    std::vector<State*> closure;
    std::vector<State*> followStates;

    std::vector< State * > & getEpsilonStates( std::vector< State * > & actualStates );
    void epsilonClosure( State * state, std::vector< State * > & eClosure );

    std::vector<State*> epsilonFollows(const std::vector<State*> states);
    std::vector<State*> & getFollowStates( std::vector< State * >& actualStates, char input);
    std::vector<State*> const & addToActual( State * state, std::vector< State * > & actualStates);
    bool testAccept( std::vector< State * > & actualStates );


    State* s0;               // startState
    std::vector<State*> S;   // a finite set of states (S)
    std::vector<State*> runningStates;
    std::vector<State*> A;

  };

}

#endif
