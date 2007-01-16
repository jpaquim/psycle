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
#ifndef NRUNTIME_H
#define NRUNTIME_H

#include "object.h"
#include <vector>
#include <iostream>
#include <sstream>

/**
@author Stefan
*/

namespace ngrs {

  class NVisitor;
  class Object;

  template<class T> inline T str(const std::string &  value) {
    T result;
    std::stringstream str;
    str << value;
    str >> result;
    return result;
  }

  class Runtime : public Object
  {
  public:
    Runtime();

    virtual ~Runtime();

    std::vector<Runtime*> components;

    void add(Runtime* component);
    void insert(Runtime* component, int index);
    virtual void removeChild(Runtime* child);
    virtual void removeChilds();
    virtual void erase(Runtime* child);
    virtual void erase();

    void setParent(Runtime* parent);
    Runtime* parent();
    Runtime* parent() const;

    virtual bool visit(NVisitor* v);
    virtual void onMessage( Event * event );

    static std::string stringify(double x);
    static std::string stringify(int x);
    static std::string trim(std::string str);

    int componentSize();

    int d2i(double d) const;

    bool isChildOf(Runtime* comp) const;

  private:

    Runtime* parent_;

  };

}

#endif
