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

NObject::NObject()
{
  if (NApp::system().propertysActive()) propertys_ = new NProperty(); else propertys_ = 0;

  if (propertys_) {
     SetGet<const std::string &, std::string &> setGet;
     setGet.set.connect(this,&NObject::setName);
     setGet.get.connect(this,&NObject::getNameProperty);
     propertys_->registrate("name",setGet);

  }
}


NObject::~NObject()
{
}

void NObject::onKeyAcceleratorNotify(NKeyAccelerator acell )
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

NProperty * NObject::propertys( )
{
  return propertys_;
}

void NObject::getNameProperty( std::string & name )
{
  name = name_;
}




