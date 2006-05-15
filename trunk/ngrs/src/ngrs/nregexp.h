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
#ifndef NREGEXP_H
#define NREGEXP_H

#include <stack>
#include <vector>
#include <string>

class NNfa;
class NState;

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
@author Stefan Nattkemper
*/
class NRegExp{
public:
    NRegExp();

    ~NRegExp();

    void setRegExp(const std::string & regExp);
    bool accept( const std::string & input );

private:

   bool inSigma(char c);
   int priority(char op);

   std::stack<char>  opStack;
   std::stack<NNfa*> nfaStack;

   void addChar(char c, NNfa * nfa);
   void unite(NNfa* nfa1, NNfa* nfa2);

   std::vector<NState*> deleteList;
};

#endif
