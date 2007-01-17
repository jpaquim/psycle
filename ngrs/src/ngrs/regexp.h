/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef NREGEXP_H
#define NREGEXP_H

#include <stack>
#include <vector>
#include <string>

namespace ngrs {

  class Nfa;
  class State;

  const char nRegUnion  = '+';
  const char nRegConcat = '_';
  const char nRegStar   = '*';
  const char nRegOpenParanthesis  = '(';
  const char nRegCloseParanthesis = ')';
  const char nRegNumber = 'N';
  const char nRegAlphabet = 'A';
  const char nRegSigma = 'S';
  const char nRegPlaceHolderBegin = '{';
  const char nRegPlaceHolderEnd = '}';

  /**
  @author  Stefan Nattkemper
  */
  class RegExp{
  public:
    RegExp();

    ~RegExp();

    void setRegExp(const std::string & regExp);
    bool accept( const std::string & input );

  private:

    bool inSigma(char c);
    int priority(char op);

    std::stack<char>  opStack;
    std::stack<Nfa*> nfaStack;

    void addChar(char c, Nfa * nfa);
    void unite(Nfa* nfa1, Nfa* nfa2);

    std::vector<State*> deleteList;
  };

}

#endif
