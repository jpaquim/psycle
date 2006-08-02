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
#include "nobjectinspector.h"
#include "nproperty.h"
#include "nlabel.h"

NObjectInspector::NObjectInspector()
 : NTable()
{
  obj_ = 0;
}


NObjectInspector::~NObjectInspector()
{
}

void NObjectInspector::setControlObject( NObject * obj )
{
  obj_ = obj;
  updateView();
}

NObject * NObjectInspector::controlObject( )
{
  return obj_;
}

void NObjectInspector::updateView( )
{
  removeChilds();
  std::vector<std::string> names = obj_->properties()->methodNames();
  std::vector<std::string>::iterator it =  names.begin();
  int y = 0;
  for (; it < names.end(); it++) {
    std:: string name = *it;
    if (obj_->properties()->getType(name)==typeid(std::string)) {
      NLabel* label = new NLabel(name);
      add(label,0,y);
      y++;
    } else
    if (obj_->properties()->getType(name)==typeid(int)) {
      NLabel* label = new NLabel(name);
      add(label,0,y);
       //listBox->add(new NItem(name));
      y++;
    }

  }
}
