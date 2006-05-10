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
#include "nobject.h"
#include "nevent.h"
#include "napp.h"
#include "nsystem.h"
#include "nproperty.h"

NObject::NObject() : properties_(NApp::system().propertysActive() ? new NPropertyMap() : 0)
{
	if (properties_) properties_->registrate("name", *this, &NObject::name, &NObject::setName);
}

NObject::~NObject()
{
  if (properties_) delete properties_;
}

void NObject::onKeyAcceleratorNotify( NKeyAccelerator acell )
{
}

void NObject::setName( const std::string & name )
{
  name_ = name;
}

const std::string & NObject::name( ) const
{
  return name_;
}

void NObject::onMessage( NEvent * event )
{
}

void NObject::addMessageListener( NObject * obj )
{
  msgListener.push_back(obj);
}

void NObject::sendMessage( NEvent * ev )
{
  for (std::vector<NObject*>::iterator it = msgListener.begin(); it < msgListener.end(); it++) {
     NObject* obj = *it;
     obj->onMessage(ev);
  }
}

NPropertyMap * NObject::properties( ) const
{
  return properties_;
}
