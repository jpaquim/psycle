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
#include "runtime.h"
#include <algorithm>

using namespace std;

namespace ngrs {

  Runtime::Runtime()
    : Object()
  {
    parent_ = 0;
  }


  Runtime::~Runtime()
  {
    if (parent()!=NULL) {
      vector<Runtime*>::iterator it = find(parent()->components.begin(),parent()->components.end(),this);
      if (it!=parent()->components.end()) parent()->components.erase(it);
    }

    for (vector<Runtime*>::iterator it = components.begin(); it < components.end(); it++) {
      Runtime* child = *it;
      child->setParent(NULL);
      delete child;
    }

    std::cout << "delete:" << name() << std::endl;
  }

  // here you will find all public methods

  void Runtime::add( Runtime * component )
  {
    if (component == this) {
      std::cerr << "Runtime Error: not possible to add circular components" << std::endl;
      return;
    }
    components.push_back(component);
    component->setParent(this);
  }

  Runtime * Runtime::parent( )
  {
    return parent_;
  }

  void Runtime::setParent( Runtime * parent )
  {
    parent_ = parent;
  }

  bool Runtime::visit( NVisitor * v )
  {
    return false;
  }

  std::string Runtime::stringify( double x )
  {
    std::ostringstream o;
    if (!(o << x))
      return "error"; else
      return o.str();
  }

  std::string Runtime::stringify( int x )
  {
    std::ostringstream o;
    if (!(o << x))
      return "error"; else
      return o.str();
  }

  string Runtime::trim(string str)
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

  int Runtime::componentSize( )
  {
    return components.size();
  }

  void Runtime::insert( Runtime * component, int index )
  {
    components.insert(components.begin()+index,component);
    component->setParent(this);
  }

  void Runtime::removeChilds( )
  {
    for (vector<Runtime*>::iterator it = components.begin(); it < components.end(); it++) {
      Runtime* child = *it;
      child->setParent(NULL);
      delete child;
    }
    components.clear();
  }

  void Runtime::removeChild( Runtime * child )
  {
    child->setParent(NULL);
    std::vector<Runtime*>::iterator itr = find(components.begin(),components.end(),child);
    components.erase(itr);
    delete child;
  }

  Runtime * Runtime::parent( ) const
  {
    return parent_;
  }

  int Runtime::d2i(double d) const
  {
    return (int) ( d<0?d-.5:d+.5);
  }

  void Runtime::erase( Runtime * child )
  {
    child->setParent(NULL);
    std::vector<Runtime*>::iterator itr = find(components.begin(),components.end(),child);
    components.erase(itr);
  }

  void Runtime::onMessage( Event * event )
  {
    for (vector<Runtime*>::iterator it = components.begin(); it < components.end(); it++) {
      Runtime* child = *it;
      child->onMessage(event);
    }
  }

  bool Runtime::isChildOf( Runtime * comp ) const
  {
    if (this == comp) return true; else
    {
      if (parent() != 0) return parent()->isChildOf(comp); else return false;
    }
  }

  void Runtime::erase( )
  {
    if (parent()) parent()->erase(this);
  }

}
