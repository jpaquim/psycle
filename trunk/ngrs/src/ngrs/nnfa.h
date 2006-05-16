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
#ifndef NNFA_H
#define NNFA_H

#include "ngrs/nstate.h"
#include <vector>


class NState;

/**
@author Stefan
*/
class NNfa{
public:
    NNfa();

    ~NNfa();

    void setS0(NState* startState);
    void setA (std::vector<NState*> acceptStates);
    void addA (NState* acceptState);

    NState* state(int i);
    NState* startState();
    std::vector<NState*> & acceptStates();


    bool accept(const std::string & input);

    void concat( NNfa * nfa );
    void star();

private:

    std::vector<NState*> closure;
    std::vector<NState*> followStates;

    std::vector< NState * > & getEpsilonStates( std::vector< NState * > & actualStates );
    void epsilonClosure( NState * state, std::vector< NState * > & eClosure );

    std::vector<NState*> epsilonFollows(const std::vector<NState*> states);
    std::vector<NState*> & getFollowStates( std::vector< NState * >& actualStates, char input);
    std::vector<NState*> const & addToActual( NState * state, std::vector< NState * > & actualStates);
    bool testAccept( std::vector< NState * > & actualStates );


    NState* s0;               // startState
    std::vector<NState*> S;   // a finite set of states (S)
    std::vector<NState*> runningStates;
    std::vector<NState*> A;

};

#endif
