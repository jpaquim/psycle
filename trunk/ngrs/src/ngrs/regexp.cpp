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
#include "regexp.h"
#include "nfa.h"
#include "state.h"

using namespace std;

namespace ngrs {

  RegExp::RegExp()
  {
  }


  RegExp::~RegExp()
  { 
    for (vector<State*>::iterator it = deleteList.begin(); it < deleteList.end(); it++)
      delete *it;

  }

  void RegExp::setRegExp( const std::string & regExp )
  {
    // replace predefined

    string reg="";

    bool set = false;

    for (string::const_iterator it = regExp.begin(); it < regExp.end(); it++ ) {
      char c = *it; 
      if (c!='!') {
        if (set) {
          if (c=='S') reg.push_back('\1'); else
            if (c=='A') reg.push_back('\2'); else
              if (c=='N') reg.push_back('\3');
          set=false;
        } else
          reg.push_back(c);
      }
      else set=true;
    }

    // build op stack

    string cExpr=""; 
    int z=0; 
    bool flag = false; 
    bool sigma = false;

    for (string::iterator it = reg.begin(); it < reg.end(); it++ ) {
      char c = *it;
      if (!sigma && (c==nRegCloseParanthesis || c==nRegStar)) flag = true;
      if (inSigma(c) || sigma) {
        if (sigma) cExpr.push_back(c); else 
        {
          if (flag) {
            z=0; flag=false;
            cExpr.push_back(nRegConcat);
          }
          z++;
          if (z==2) {
            cExpr.push_back(nRegConcat);
            z=1;
          }
          cExpr.push_back(c);
        }
      } else  {
        z=0;
        if (flag && c==nRegOpenParanthesis) {
          cExpr.push_back(nRegConcat);
        }
        if (flag && c==nRegStar) {
          z=1;
        }
        flag=false;
        cExpr.push_back(c);
      }
      if (c=='\'') sigma = !sigma;
    }

    // insert concat symbols

    string postfix = "";
    sigma = false;
    for (string::iterator it = cExpr.begin(); it < cExpr.end(); it++ ) {
      char c = *it;
      if (inSigma(c) || sigma) postfix.push_back(c); else {
        if (opStack.empty()) { opStack.push(c);
        if (c=='(') postfix.push_back('{');
        } else {
          if (c=='(') {
            opStack.push('(');
            postfix.push_back('{');
          } else
            if (c==')') {
              while (opStack.top()!='(') {
                postfix.push_back(opStack.top());
                opStack.pop();
              }
              opStack.pop();
              postfix.push_back(nRegPlaceHolderEnd);
            } else
              if (priority(c)>=priority(opStack.top())) {
                postfix.push_back(opStack.top());
                opStack.pop();
                opStack.push(c);
              } else {
                opStack.push(c);
              }
        }
      }
      if (c=='\'') sigma = !sigma;
    }
    while (!opStack.empty()) {
      char c = opStack.top();
      postfix.push_back(c);
      opStack.pop();
    }

    reg = postfix;


    // build nfa

    sigma = false;
    bool writeInPlaceHolder =false;
    for (string::iterator it = reg.begin(); it < reg.end(); it++ ) {
      char c = *it;
      if (c==nRegPlaceHolderBegin) writeInPlaceHolder=true; else  if (c==nRegPlaceHolderEnd) writeInPlaceHolder=false;
      if (c=='\'') sigma = !sigma; else {
        if (inSigma(c) || sigma) {
          Nfa* na = new Nfa();
          addChar(c,na);
          nfaStack.push(na);
        } else {
          if (c==nRegConcat) {
            Nfa* nfa1 = nfaStack.top(); nfaStack.pop();
            Nfa* nfa2 = nfaStack.top(); nfaStack.pop();
            nfa2->concat(nfa1);
            delete nfa1;
            nfaStack.push(nfa2);
          } else
            if (c==nRegUnion) {
              Nfa* nfa1 = nfaStack.top(); nfaStack.pop();
              Nfa* nfa2 = nfaStack.top(); nfaStack.pop();
              unite(nfa2,nfa1);
              delete nfa1;
              nfaStack.push(nfa2);
            } else
              if (c==nRegStar) {
                Nfa* nfa1 = nfaStack.top(); nfaStack.pop();
                nfa1->star();
                nfaStack.push(nfa1);
              }
        }
      }
    }
  }

  bool RegExp::inSigma( char c )
  {
    if (c == nRegPlaceHolderBegin || c==nRegPlaceHolderEnd || c==nRegConcat || c==nRegUnion || c==nRegStar || c==nRegOpenParanthesis || c==nRegCloseParanthesis) return false; else   
      return true;
  }

  int RegExp::priority( char op )
  {
    if (op==nRegConcat) return 1;
    if (op=='+')  return 2;
    if (op=='(') return 10;
    if (op==')') return 10;
    return 0;
  }

  void RegExp::addChar( char c, Nfa * nfa )
  {
    State* q0 = new State();
    State* q1 = new State();

    nfa->setS0(q0);
    q0->addFollowState(c,q1);
    nfa->addA(q1);

    deleteList.push_back(q0);
    deleteList.push_back(q1);
  }

  void RegExp::unite( Nfa * nfa1, Nfa * nfa2 )
  {
    State* e1 = new State();
    State* e2 = new State();

    e1->addEpsilon(nfa1->startState());
    e1->addEpsilon(nfa2->startState());

    for (vector<State*>::iterator it = nfa1->acceptStates().begin(); it < nfa1->acceptStates().end(); it++) {
      State* state = *it;
      state->addEpsilon(e2);
    }

    nfa1->acceptStates().clear();
    nfa1->addA(e2);

    for (vector<State*>::iterator it = nfa2->acceptStates().begin(); it < nfa2->acceptStates().end(); it++) {
      State* state = *it;
      state->addEpsilon(e2);
    }

    nfa1->setS0(e1);

    deleteList.push_back(e1);
    deleteList.push_back(e2);
  }

  bool RegExp::accept( const std::string & input )
  {
    Nfa* nfa = nfaStack.top();
    return nfa->accept(input);
  }

}
