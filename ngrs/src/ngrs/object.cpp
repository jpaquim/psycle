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
#include "app.h"
#include "object.h"
#include "event.h"
#include "system.h"
#include "property.h"

namespace ngrs {

  Object::Object() : properties_(0) //App::system().propertysActive() ? new PropertyMap() : 0)
  {
    // segfaults under windows                    
    // if (properties_) properties_->bind("name", *this, &Object::name, &Object::setName);
    // segfault!	
  }

  Object::~Object()
  {
    if (properties_) delete properties_;
  }

  void Object::onKeyAcceleratorNotify( KeyAccelerator acell )
  {
  }

  void Object::setName( const std::string & name )
  {
    name_ = name;
  }

  const std::string & Object::name( ) const
  {
    return name_;
  }

  void Object::onMessage( Event * event )
  {
  }

  void Object::addMessageListener( Object * obj )
  {
    msgListener.push_back(obj);
  }

  void Object::sendMessage( Event * ev )
  {
    for (std::vector<Object*>::iterator it = msgListener.begin(); it < msgListener.end(); it++) {
      Object* obj = *it;
      obj->onMessage(ev);
    }
  }

  PropertyMap * Object::properties( ) const
  {
    return properties_;
  }

  std::string Object::toString() const {
    return name_;
  }

}
