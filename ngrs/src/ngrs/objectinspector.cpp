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
#include "objectinspector.h"
#include "property.h"
#include "label.h"
#include "edit.h"
#include "bevelborder.h"
#include <algorithm>

namespace ngrs {

  ObjectInspector::ObjectInspector()
    : Table()
  {
    obj_ = 0;
  }


  ObjectInspector::~ObjectInspector()
  {
  }

  void ObjectInspector::setControlObject( Object * obj )
  {
    obj_ = obj;
    updateView();
  }

  Object * ObjectInspector::controlObject( )
  {
    return obj_;
  }

  void ObjectInspector::updateView( )
  {
    removeChilds();
    edits.clear();

    std::vector<std::string> names = obj_->properties()->publishedNames();

    std::vector<std::string>::iterator it =  names.begin();
    int y = 0;
    for (; it < names.end(); it++) {
      std:: string name = *it;
      if (obj_->properties()->getType(name)==typeid(std::string)) {
        Label* label = new Label(name);
        add(label,0,y);
        Edit* edit = new Edit();
        edit->setSkin(Skin());
        BevelBorder bvl(nNone,nLowered);
        bvl.setSpacing(Size(2,2,2,2));
        edit->setBorder(bvl);
        edit->setName(name);
        edit->setPreferredSize(100,15);
        edit->keyPress.connect(this,&ObjectInspector::onStringEdit);
        add(edit,1,y);
        edits.push_back(edit);
        y++;
      } else
        if (obj_->properties()->getType(name)==typeid(Color)) {

        } else
          if (obj_->properties()->getType(name)==typeid(float)) {
            Label* label = new Label(name);
            add(label,0,y);
            //listBox->add(new Item(name));
            Edit* edit = new Edit();
            edit->setSkin(Skin());
            BevelBorder bvl(nNone,nLowered);
            bvl.setSpacing(Size(2,2,2,2));
            edit->setBorder(bvl);
            edit->setName(name);
            edit->setPreferredSize(100,15);
            float value = obj_->properties()->get<float>(name);
            edit->setText(stringify(value));
            edit->keyPress.connect(this,&ObjectInspector::onFloatEdit);
            add(edit,1,y);
            edits.push_back(edit);
            y++;
          } else
            if (obj_->properties()->getType(name)==typeid(int)) {
              Label* label = new Label(name);
              add(label,0,y);
              Edit* edit = new Edit();
              edit->setSkin(Skin());
              BevelBorder bvl(nNone,nLowered);
              bvl.setSpacing(Size(2,2,2,2));
              edit->setBorder(bvl);
              edit->setName(name);
              edit->setPreferredSize(100,15);
              int value = obj_->properties()->get<int>(name);
              edit->setText(stringify(value));
              edit->keyPress.connect(this,&ObjectInspector::onIntEdit);
              add(edit,1,y);
              edits.push_back(edit);
              y++;
            }
    }
  }

  void ObjectInspector::onStringEdit( const KeyEvent & event )
  {
    std::vector<Edit*>::iterator it = find( edits.begin(), edits.end(), event.sender() );
    if ( it != edits.end() ) {
      obj_->properties()->set(event.sender()->name(), (*it)->text() );
      changed.emit(this);
    }
  }

  void ObjectInspector::onFloatEdit( const KeyEvent & event )
  {
    std::vector<Edit*>::iterator it = find( edits.begin(), edits.end(), event.sender() );
    if ( it != edits.end() ) {
      float value = str<float> ( (*it)->text() );
      obj_->properties()->set(event.sender()->name(), value );
      changed.emit(this);
    }
  }

  void ObjectInspector::onIntEdit( const KeyEvent & event )
  {
    std::vector<Edit*>::iterator it = find( edits.begin(), edits.end(), event.sender() );
    if ( it != edits.end() ) {
      int value = str<int> ( (*it)->text() );
      obj_->properties()->set(event.sender()->name(), value );
      changed.emit(this);
    }
  }

}
