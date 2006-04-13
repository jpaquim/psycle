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
#ifndef NRUNTIME_H
#define NRUNTIME_H

#include "nobject.h"
#include <vector>
#include <iostream>
#include <sstream>

class NVisitor;

/**
@author Stefan
*/
class NObject;

class NRuntime : public NObject
{
public:
    NRuntime();

   virtual ~NRuntime();

   std::vector<NRuntime*> components;

   void add(NRuntime* component);
   void insert(NRuntime* component, int index);
   virtual void removeChild(NRuntime* child);
   virtual void removeChilds();
   virtual void erase(NRuntime* child);

   void setParent(NRuntime* parent);
   NRuntime* parent();
   NRuntime* parent() const;

   virtual bool visit(NVisitor* v);

   static std::string stringify(double x);
   static std::string stringify(int x);
   static std::string trim(std::string str);

   int componentSize();

   int d2i(double d);

private:

   NRuntime* parent_;

};

#endif
