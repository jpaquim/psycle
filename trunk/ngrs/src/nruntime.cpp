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
#include "nruntime.h"

using namespace std;

NRuntime::NRuntime()
 : NObject()
{
  parent_ = 0;
}


NRuntime::~NRuntime()
{
  if (parent()!=NULL) {
    vector<NRuntime*>::iterator it = find(parent()->components.begin(),parent()->components.end(),this);
    if (it!=parent()->components.end()) parent()->components.erase(it);
  }
  for (vector<NRuntime*>::iterator it = components.begin(); it < components.end(); it++) {
    NRuntime* child = *it;
    child->setParent(NULL);
    delete child;
  }
}

// here you will find all public methods

void NRuntime::add( NRuntime * component )
{
  components.push_back(component);
  component->setParent(this);
}

NRuntime * NRuntime::parent( )
{
  return parent_;
}

void NRuntime::setParent( NRuntime * parent )
{
  parent_ = parent;
}

bool NRuntime::visit( NVisitor * v )
{
  return false;
}

std::string NRuntime::stringify( double x )
{
    std::ostringstream o;
    if (!(o << x))
      return "error"; else
    return o.str();
}

std::string NRuntime::stringify( int x )
{
     std::ostringstream o;
    if (!(o << x))
      return "error"; else
    return o.str();
}

string NRuntime::trim(string str)
{
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
  return str;
}

int NRuntime::componentSize( )
{
  return components.size();
}

void NRuntime::insert( NRuntime * component, int index )
{
  components.insert(components.begin()+index,component);
  component->setParent(this);
}

void NRuntime::removeChilds( )
{
  for (vector<NRuntime*>::iterator it = components.begin(); it < components.end(); it++) {
   NRuntime* child = *it;
   child->setParent(NULL);
   delete child;
  }
  components.clear();
}

void NRuntime::removeChild( NRuntime * child )
{
  child->setParent(NULL);
  std::vector<NRuntime*>::iterator itr = find(components.begin(),components.end(),child);
  components.erase(itr);
  delete child;
}

NRuntime * NRuntime::parent( ) const
{
  return parent_;
}

int NRuntime::d2i(double d)
 {
   return (int) ( d<0?d-.5:d+.5);
 }


