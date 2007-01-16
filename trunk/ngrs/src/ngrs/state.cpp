/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "state.h"

using namespace std;

namespace ngrs {

  State::State()
  {
  }

  State::~State()
  {
  }

  const std::vector< State* > & State::T( const char input )
  {
    std::map<char, vector<State*> >::iterator itr;

    if ( (itr = TMap.find('\1')) != TMap.end()) return itr->second;  else
      if ( (((((unsigned char)input >64) && ((unsigned char) input <90))) || (((unsigned char) input > 96) && ((unsigned char) input < 123)))  &&
        (itr = TMap.find('\2')) != TMap.end()) return itr->second; else  
        if ( (((unsigned char) input > 47) && (unsigned char) input < 58 ) && (itr = TMap.find('\3')) != TMap.end()) return itr->second;  else                   
          if ( (itr = TMap.find(input)) != TMap.end()) return itr->second; 
          else return ndef;
  }

  void State::setSequence( const char label, const std::vector< State* > & sequence )
  {
    TMap[label] = sequence;
  }

  void State::addFollowState( const char label, State * state )
  {
    std::map<char,std::vector<State*> >::iterator itr;
    if ( (itr = TMap.find(label)) != TMap.end() ) {
      itr->second.push_back(state);
    } else 
    {
      vector<State*> states;
      states.push_back(state);
      TMap[label] = states;
    }
  }

  void State::setEpsilon( const std::vector< State * > & sequence )
  {
    epsilonSequence = sequence;
  }

  void State::addEpsilon( State * state )
  {
    epsilonSequence.push_back(state);
  }

  const std::vector< State * > & State::E( )
  {
    return epsilonSequence;
  }

}
